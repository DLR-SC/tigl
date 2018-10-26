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
#include "CTiglSymetricSplineBuilder.h"
#include "tiglcommonfunctions.h"
#include "CTiglLogging.h"

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

#include "math.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>

namespace tigl
{
// Constructor
CCPACSFuselageProfile::CCPACSFuselageProfile(CTiglUIDManager* uidMgr)
    : generated::CPACSProfileGeometry(uidMgr)
    , mirrorSymmetry(false)
    , wireCache(*this, &CCPACSFuselageProfile::BuildWires)
    , diameterPointsCache(*this, &CCPACSFuselageProfile::BuildDiameterPoints)
    , profileWireAlgo(new CTiglInterpolateBsplineWire)
{
}

CCPACSFuselageProfile::~CCPACSFuselageProfile() {}

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
void CCPACSFuselageProfile::Invalidate()
{
    wireCache.clear();
    diameterPointsCache.clear();
}

// Returns the fuselage profile wire
TopoDS_Wire CCPACSFuselageProfile::GetWire(bool forceClosed) const
{
    return forceClosed ? wireCache->closed : wireCache->original;
}

// check if the distance between two points are below a fixed value, so that
// these point could be imaged as "equal".
bool CCPACSFuselageProfile::checkSamePoints(gp_Pnt pointA, gp_Pnt pointB) const
{
    Standard_Real distance;
    distance = pointA.Distance(pointB);
    if (distance < 0.01) {
        return true;
    }
    else {
        return false;
    }
}


// Builds the fuselage profile wire. The returned wire is already transformed by the
// fuselage profile element transformation.
void CCPACSFuselageProfile::BuildWires(WireCache& cache) const
{
    if (!m_pointList_choice1)
        throw CTiglError("No pointlist specified");
    if (GetNumPoints() < 2) {
        throw CTiglError("Number of points is less than 2 in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);
    }

    const std::vector<CTiglPoint>& coordinates = m_pointList_choice1->AsVector();

    ITiglWireAlgorithm::CPointContainer points;
    points.push_back(coordinates[0].Get_gp_Pnt());
    for (std::size_t i = 1; i < coordinates.size() -1 ; i++) {
        gp_Pnt p1 = coordinates[i-1].Get_gp_Pnt();
        gp_Pnt p2 = coordinates[i].Get_gp_Pnt();

        // only take points that are not "the same"
        if ( !checkSamePoints(p1, p2) ) {
            points.push_back(coordinates[i].Get_gp_Pnt());
        }
    }


    // we always want to include the endpoint, if it's the same as the startpoint
    // we use the startpoint to enforce closing of the spline
    gp_Pnt pStart =  coordinates.front().Get_gp_Pnt();
    gp_Pnt pEnd   =  coordinates.back().Get_gp_Pnt();
    if (checkSamePoints(pStart,pEnd)) {
        points.push_back(pStart);
    }
    else {
        points.push_back(pEnd);
    }

    TopoDS_Wire tempWireClosed, tempWireOriginal;

    bool mirrorAlgorithmSuccess = true;
    if (mirrorSymmetry) {
        try {
            CTiglSymetricSplineBuilder builder(points);
            Handle(Geom_BSplineCurve) c = builder.GetBSpline();

            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(c);
            gp_Pnt pstart = c->Value(c->FirstParameter());
            gp_Pnt pend   = c->Value(c->LastParameter());

            tempWireOriginal = BRepBuilderAPI_MakeWire(edge);
            tempWireClosed   = BRepBuilderAPI_MakeWire(
                                   edge,
                                   BRepBuilderAPI_MakeEdge(pend, pstart));

            mirrorAlgorithmSuccess = true;
        }
        catch (CTiglError&) {
            LOG(WARNING) << "The points in fuselage profile " << GetUID() << " can not be used to create a symmetric half profile."
                         << "The y value of the first point must be zero!";
            mirrorAlgorithmSuccess = false;
        }
    }


    if (!mirrorSymmetry || !mirrorAlgorithmSuccess) {
        // Build wire from fuselage profile points
        const ITiglWireAlgorithm& wireBuilder = *profileWireAlgo;
        const CTiglInterpolateBsplineWire* pSplineBuilder = dynamic_cast<const CTiglInterpolateBsplineWire*>(&wireBuilder);
        if (pSplineBuilder) {
            const_cast<CTiglInterpolateBsplineWire*>(pSplineBuilder)->setEndpointContinuity(_C1);
        }

        tempWireClosed   = wireBuilder.BuildWire(points, true);
        tempWireOriginal = wireBuilder.BuildWire(points, false);
        if (pSplineBuilder) {
            const_cast<CTiglInterpolateBsplineWire*>(pSplineBuilder)->setEndpointContinuity(_C0);
        }

        if (tempWireClosed.IsNull() == Standard_True || tempWireOriginal.IsNull() == Standard_True) {
            throw CTiglError("TopoDS_Wire is null in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);
        }
    }

    cache.closed   = tempWireClosed;
    cache.original = tempWireOriginal;
}

// Transforms a point by the fuselage profile transformation
gp_Pnt CCPACSFuselageProfile::TransformPoint(const gp_Pnt& aPoint) const
{
    CTiglTransformation transformation;
    return transformation.Transform(aPoint);
}

// Gets a point on the fuselage profile wire in dependence of a parameter zeta with
// 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the wire start point,
// for zeta = 1.0 the last wire point.
gp_Pnt CCPACSFuselageProfile::GetPoint(double zeta) const
{
    if (zeta < 0.0 || zeta > 1.0) {
        throw CTiglError("Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CCPACSFuselageProfile::GetPoint", TIGL_ERROR);
    }

    // Get the first edge of the wire
    BRepTools_WireExplorer wireExplorer(wireCache->original);
    if (!wireExplorer.More()) {
        throw CTiglError("Not enough edges found in CCPACSFuselageProfile::GetPoint", TIGL_ERROR);
    }

    Standard_Real firstParam = 0.;
    Standard_Real lastParam = 1.;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(wireExplorer.Current(), firstParam, lastParam);

    gp_Pnt point = curve->Value(firstParam*(1-zeta) + lastParam*zeta);

    return point;
}


void CCPACSFuselageProfile::BuildDiameterPoints(DiameterPointsCache& cache) const
{
    if (!m_pointList_choice1)
        throw CTiglError("No pointlist specified");
    const std::vector<CTiglPoint>& coordinates = m_pointList_choice1->AsVector();

    if (mirrorSymmetry) {
        cache.start = coordinates[0].Get_gp_Pnt();
        cache.end = coordinates[coordinates.size() - 1].Get_gp_Pnt();
    }
    else {
        // compute starting diameter point
        gp_Pnt firstPnt = coordinates[0].Get_gp_Pnt();
        gp_Pnt lastPnt  = coordinates[coordinates.size() - 1].Get_gp_Pnt();
        double x = (firstPnt.X() + lastPnt.X())/2.;
        double y = (firstPnt.Y() + lastPnt.Y())/2.;
        double z = (firstPnt.Z() + lastPnt.Z())/2.;
        cache.start = gp_Pnt(x,y,z);

        // find the point with the max dist to starting point
        cache.end = cache.start;
        for (std::vector<CTiglPoint>::const_iterator it = coordinates.begin(); it != coordinates.end(); ++it) {
            gp_Pnt point = it->Get_gp_Pnt();
            if (cache.start.Distance(point) > cache.start.Distance(cache.end)) {
                cache.end = point;
            }
        }
        // project into x-z plane
        cache.end.SetY(0.);
        cache.start.SetY(0.);
    }
}

TopoDS_Wire CCPACSFuselageProfile::GetDiameterWire() const
{
    Handle(Geom_TrimmedCurve) diameterCurve = GC_MakeSegment(diameterPointsCache->start, diameterPointsCache->end);
    TopoDS_Edge diameterEdge = BRepBuilderAPI_MakeEdge(diameterCurve);
    TopoDS_Wire diameterWire = BRepBuilderAPI_MakeWire(diameterEdge);
    return diameterWire;
}

} // end namespace tigl

