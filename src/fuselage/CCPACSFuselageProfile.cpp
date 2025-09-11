/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file
* @brief  Implementation of CPACS fuselage profile handling routines.
*/

#include "generated/TixiHelper.h"
#include "CCPACSFuselageProfile.h"
#include "CTiglError.h"
#include "CTiglTransformation.h"
#include "CTiglInterpolateBsplineWire.h"
#include "CTiglBSplineAlgorithms.h"
#include "tiglcommonfunctions.h"
#include "CTiglLogging.h"
#include "Debugging.h"

#include "TopoDS.hxx"
#include "TopoDS_Wire.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "GC_MakeSegment.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GeomConvert.hxx"
#include "Geom_Plane.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "Geom2d_Line.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "BRep_Tool.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI.hxx"
#include "gp_Pln.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "math.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>

namespace
{
// In case of a half profile, we have to compute the symmetric points and parameters
void SymmetrizeFuselageProfile(std::vector<tigl::CTiglPoint>& points, tigl::ParamMap& params,
                               std::vector<unsigned int>& kinks)
{
    size_t n_points = points.size();

    if (n_points == 0) {
        return;
    }

    if (fabs(points[0].y) > 1e-6) {
        throw tigl::CTiglError("Cannot create a symmetric fuselage profile. Y-Coordinate not zero!");
    }

    auto get_param_or = [&params](unsigned int idx, double def_value) -> double {
        const auto& it = params.find(idx);
        return it != params.end() ? it->second : def_value;
    };

    double umin = get_param_or(0, 0.);
    double umax = umin + 2. * (get_param_or(static_cast<unsigned int>(n_points - 1), 0.5) - umin);

    // y is already ~ 0, make it really zero!
    points[0].y = 0.;

    // mirror each point at x-z plane i.e. mirror y coordinate to close the profile
    // and skip first point
    for (size_t i = n_points - 1; i > 0; i--) {
        auto curP = points[i];
        if (i == n_points - 1 && std::abs(curP.y) < 1e-6) {
            // do not add the same points twice
            continue;
        }
        curP.y                  = -curP.y;
        unsigned int currentIdx = static_cast<unsigned int>(points.size());
        if (std::find(std::begin(kinks), std::end(kinks), i) != std::end(kinks)) {
            kinks.push_back(currentIdx);
        }
        auto parm_it = params.find(static_cast<unsigned int>(i));
        if (parm_it != params.end()) {
            double param_new   = umax + umin - parm_it->second;
            params[currentIdx] = param_new;
        }
        points.push_back(curP);
    }

    points.push_back(points[0]);
    params[0]                                            = umin;
    params[static_cast<unsigned int>(points.size() - 1)] = umax;
}
} // namespace

namespace tigl
{
// Constructor
CCPACSFuselageProfile::CCPACSFuselageProfile(CCPACSFuselageProfiles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSProfileGeometry(parent, uidMgr)
    , mirrorSymmetry(false)
    , wireCache(*this, &CCPACSFuselageProfile::BuildWires)
    , diameterPointsCache(*this, &CCPACSFuselageProfile::BuildDiameterPoints)
    , profileWireAlgo(new CTiglInterpolateBsplineWire)
    , sizeCache(*this, &CCPACSFuselageProfile::BuildSize)
{
}

CCPACSFuselageProfile::~CCPACSFuselageProfile()
{
}

// Read fuselage profile file
void CCPACSFuselageProfile::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    Invalidate();
    generated::CPACSProfileGeometry::ReadCPACS(tixiHandle, xpath);

    // symmetry element does not conform to CPACS spec
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/symmetry")) {
        mirrorSymmetry = tixi::TixiGetTextElement(tixiHandle, xpath + "/symmetry") == "half";
    }
}

const int CCPACSFuselageProfile::GetNumPoints() const
{
    if (!m_pointList_choice1)
        return 0;
    return static_cast<int>(m_pointList_choice1->AsVector().size());
}

// Returns the flag for the mirror symmetry with respect to the x-z-plane in the fuselage profile
bool CCPACSFuselageProfile::GetMirrorSymmetry() const
{
    return mirrorSymmetry;
}

// Invalidates internal fuselage profile state
void CCPACSFuselageProfile::InvalidateImpl(const boost::optional<std::string>& source) const
{
    wireCache.clear();
    diameterPointsCache.clear();
    sizeCache.clear();
}

// Returns the fuselage profile wire
TopoDS_Wire CCPACSFuselageProfile::GetWire(bool forceClosed) const
{
    return forceClosed ? wireCache->closed : wireCache->original;
}

// Builds the fuselage profile wire. The returned wire is already transformed by the
// fuselage profile element transformation.
void CCPACSFuselageProfile::BuildWires(WireCache& cache) const
{
    if(m_pointList_choice1){
        BuildWiresPointList(cache);
        return;
    }
    if(m_standardProfile_choice3){
        if(m_standardProfile_choice3->GetRectangle_choice1()){
            BuildWiresRectangle(cache);
            return;
        }
        if(m_standardProfile_choice3->GetSuperEllipse_choice2()){
            BuildWiresSuperEllipse(cache);
            return;
        }
    }
    else{
        throw CTiglError("Fuselage profile type not supported.");
    }
}

// Builds the fuselage profile wire from point list.
void CCPACSFuselageProfile::BuildWiresPointList(WireCache& cache) const
{
        if (GetNumPoints() < 2) {
            throw CTiglError("Number of points is less than 2 in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);
        }

        auto points = m_pointList_choice1->AsVector();
        auto params = m_pointList_choice1->GetParamsAsMap();
        auto kinks  = m_pointList_choice1->GetKinksAsVector();
        if (mirrorSymmetry) {
            SymmetrizeFuselageProfile(points, params, kinks);
        }

        // Build the B-Spline
        auto occPoints = OccArray(points);

        // we always want to include the endpoint, if it's the same as the startpoint
        // we use the middle to enforce closing of the spline
        gp_Pnt pStart = points.front().Get_gp_Pnt();
        gp_Pnt pEnd   = points.back().Get_gp_Pnt();

        // this check allows some tolerance, based on the absolute size of the profile
        if (pStart.Distance(pEnd) < 0.005 * CTiglBSplineAlgorithms::scale(occPoints->Array1())) {
            gp_Pnt pMiddle = 0.5 * (pStart.XYZ() + pEnd.XYZ());
            occPoints->SetValue(occPoints->Lower(), pMiddle);
            occPoints->SetValue(occPoints->Upper(), pMiddle);
        }

        // Here, the B-spline is reparameterized based on the CPACS profile after setting it up at first for accuracy reasons
        // Also, a tolerance is passed used for knot insertion and removal during the reparameterization algorithm
        CTiglInterpolatePointsWithKinks interp(occPoints, kinks, params, 0.5, 3, CTiglInterpolatePointsWithKinks::Algo::InterpolateFirstThenReparametrize, 1e-8);
        auto spline = interp.Curve();

        if (mirrorSymmetry) {
            double umin = spline->FirstParameter();
            double umax = spline->LastParameter();
            spline      = CTiglBSplineAlgorithms::trimCurve(spline, umin, 0.5 * (umin + umax));
            CTiglBSplineAlgorithms::reparametrizeBSpline(*spline, umin, umax);
        }

        // Reparamaterization based on a ParamMap defined in the CPACS file within CTiglInterpolatePointsWithKinks does not get along with reparametrizeBSplineNiceKnots.
        // The geometry is changed in a way that is not acceptable anymore. However out of performance reasons, the feature should not be rejected in the most cases.
        // Due to those conflicts, the function is only called, when there are no parameters defined in the CPACS file:
        if (params.empty()) {
            // we reparametrize the spline to get better performing lofts.
            // there might be a small accuracy loss though.
            spline = CTiglBSplineAlgorithms::reparametrizeBSplineNiceKnots(spline).curve;
         }

        // Create wires
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(spline).Edge();
        BRepBuilderAPI_MakeWire builder1(edge);
        TopoDS_Wire tempWireOriginal = builder1.Wire();

        BRepBuilderAPI_MakeWire builder2(edge);
        if (!spline->IsClosed()) {
            builder2.Add(BRepBuilderAPI_MakeEdge(spline->EndPoint(), spline->StartPoint()));
        }
        TopoDS_Wire tempWireClosed = builder2.Wire();
        if (tempWireClosed.IsNull() == Standard_True || tempWireOriginal.IsNull() == Standard_True) {
            throw CTiglError("TopoDS_Wire is null in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);
        }

        cache.closed   = tempWireClosed;
        cache.original = tempWireOriginal;
}

//Builds the fuselage profile wire from heightToWidthRatio and cornerRadius with a tolerance of 1e-3 for the wire
void CCPACSFuselageProfile::BuildWiresRectangle(WireCache& cache) const
{
    if(!m_standardProfile_choice3->GetRectangle_choice1()){
        throw CTiglError("CCPACSFuselageProfile::BuildWiresRectangle: Missing rectangle definition in standardProfile.", TIGL_UNINITIALIZED);
    }
    //Get Paramenters
    auto& rectangle_profile = *m_standardProfile_choice3->GetRectangle_choice1();
    double heightToWidthRatio = rectangle_profile.GetHeightToWidthRatio().GetValue();
    double radius = (rectangle_profile.GetCornerRadius())? *rectangle_profile.GetCornerRadius() : 0. ;
    //Build wire
    TopoDS_Wire wire = BuildWireRectangle(heightToWidthRatio,radius, 1e-3);
    cache.closed = wire;
    cache.original = wire;
}

//Builds the fuselage profile wires from lowerHeightFraction and exponents m,n for lower and upper semi-ellipse
void CCPACSFuselageProfile::BuildWiresSuperEllipse(WireCache& cache) const
{
    if(!m_standardProfile_choice3->GetSuperEllipse_choice2()){
        throw CTiglError("CCPACSFuselageProfile::BuildWiresSuperEllipse: Missing superEllipse definiton in standardProfile.", TIGL_UNINITIALIZED);
    }
    //Get Paramenters
    auto& superellipse_profile = *m_standardProfile_choice3->GetSuperEllipse_choice2();
    double lowerHeightFraction = superellipse_profile.GetLowerHeightFraction();
    double mLower = superellipse_profile.GetMLower().GetValue();
    double mUpper = superellipse_profile.GetMUpper().GetValue();
    double nLower = superellipse_profile.GetNLower().GetValue();
    double nUpper = superellipse_profile.GetNUpper().GetValue();
    //Build wire
    TopoDS_Wire wire = BuildWireSuperEllipse(lowerHeightFraction, mLower, mUpper, nLower, nUpper);
    cache.closed = wire;
    cache.original = wire;
}

// Transforms a point by the fuselage profile transformation
gp_Pnt CCPACSFuselageProfile::TransformPoint(const gp_Pnt& aPoint) const
{
    CTiglTransformation transformation; // do nothing ?!
    return transformation.Transform(aPoint);
}

// Gets a point on the fuselage profile wire in dependence of a parameter zeta with
// 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the wire start point,
// for zeta = 1.0 the last wire point.
gp_Pnt CCPACSFuselageProfile::GetPoint(double zeta) const
{
    if (zeta < 0.0 || zeta > 1.0) {
        throw CTiglError("Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CCPACSFuselageProfile::GetPoint",
                         TIGL_ERROR);
    }

    // Get the first edge of the wire
    BRepTools_WireExplorer wireExplorer(wireCache->original);
    if (!wireExplorer.More()) {
        throw CTiglError("Not enough edges found in CCPACSFuselageProfile::GetPoint", TIGL_ERROR);
    }

    Standard_Real firstParam = 0.;
    Standard_Real lastParam  = 1.;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(wireExplorer.Current(), firstParam, lastParam);

    gp_Pnt point = curve->Value(firstParam * (1 - zeta) + lastParam * zeta);

    return point;
}

void CCPACSFuselageProfile::BuildDiameterPoints(DiameterPointsCache& cache) const
{
    if (m_pointList_choice1){
        const std::vector<CTiglPoint>& coordinates = m_pointList_choice1->AsVector();
        if (mirrorSymmetry) {
            cache.start = coordinates[0].Get_gp_Pnt();
            cache.end   = coordinates[coordinates.size() - 1].Get_gp_Pnt();
        }
        else {
            // compute starting diameter point
            gp_Pnt firstPnt = coordinates[0].Get_gp_Pnt();
            gp_Pnt lastPnt  = coordinates[coordinates.size() - 1].Get_gp_Pnt();
            double x        = (firstPnt.X() + lastPnt.X()) / 2.;
            double y        = (firstPnt.Y() + lastPnt.Y()) / 2.;
            double z        = (firstPnt.Z() + lastPnt.Z()) / 2.;
            cache.start     = gp_Pnt(x, y, z);

            // find the point with the max dist to starting point
            cache.end = cache.start;
            for (std::vector<CTiglPoint>::const_iterator it = coordinates.begin(); it != coordinates.end(); ++it) {
                gp_Pnt point = it->Get_gp_Pnt();
                if (cache.start.Distance(point) > cache.start.Distance(cache.end)) {
                    cache.end = point;
                }
            }
        }
    } else if (m_standardProfile_choice3){
        if(m_standardProfile_choice3->GetRectangle_choice1()){
           //Get Paramenters
           auto& rectangle_profile = *m_standardProfile_choice3->GetRectangle_choice1();
           double heightToWidthRatio = rectangle_profile.GetHeightToWidthRatio().GetValue();
           cache.start = gp_Pnt(0., 0., 0.5 * heightToWidthRatio);
           cache.end = gp_Pnt(0., 0., -0.5 * heightToWidthRatio);
        } else if(m_standardProfile_choice3->GetSuperEllipse_choice2()) {
            cache.start = gp_Pnt(0., 0., 0.5);
            cache.end = gp_Pnt(0., 0., -0.5);
        } else {
        throw CTiglError("Unknown or unsupported profile type");
        }
    }
}

TopoDS_Wire CCPACSFuselageProfile::GetDiameterWire() const
{
    Handle(Geom_TrimmedCurve) diameterCurve = GC_MakeSegment(diameterPointsCache->start, diameterPointsCache->end);
    TopoDS_Edge diameterEdge                = BRepBuilderAPI_MakeEdge(diameterCurve);
    TopoDS_Wire diameterWire                = BRepBuilderAPI_MakeWire(diameterEdge);
    return diameterWire;
}

void CCPACSFuselageProfile::BuildSize(SizeCache& cache) const
{
    TopoDS_Wire wire = GetWire(true);
    BRepMesh_IncrementalMesh mesh(wire, 0.001); // tessellate the wire to have a more accurate bounding box.
    Bnd_Box boundingBox;
    BRepBndLib::Add(wire, boundingBox);
    CTiglPoint min, max;
    boundingBox.Get(min.x, min.y, min.z, max.x, max.y, max.z);
    cache.width  = max.y - min.y;
    cache.height = max.z - min.z;
}

double CCPACSFuselageProfile::GetWidth() const
{
    return sizeCache->width;
}

double CCPACSFuselageProfile::GetHeight() const
{
    return sizeCache->height;
}

} // end namespace tigl
