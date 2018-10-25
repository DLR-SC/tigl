/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-12 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
*
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
* @brief  Implementation of CPACS wing profile as a point list.
*/

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <limits>

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglInterpolateBsplineWire.h"
#include "CTiglInterpolateLinearWire.h"
#include "ITiglWingProfileAlgo.h"
#include "CTiglWingProfilePointList.h"
#include "CTiglTransformation.h"
#include "math.h"
#include "CCPACSWingProfile.h"
#include "tiglcommonfunctions.h"

#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Vec.hxx"
#include "gp_Dir2d.hxx"
#include "gp_Pln.hxx"
#include "Bnd_Box.hxx"
#include "Geom2d_Line.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Geom_Plane.hxx"
#include "TopoDS.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "BRep_Tool.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "GeomAPI.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBndLib.hxx"
#include "ShapeFix_Wire.hxx"


inline gp_Pnt operator+(const gp_Pnt& a, const gp_Pnt& b)
{
    return gp_Pnt(a.X()+b.X(), a.Y()+b.Y(), a.Z()+b.Z());
}
inline gp_Pnt operator/(const gp_Pnt& a, double b)
{
    return gp_Pnt(a.X() / b, a.Y() / b, a.Z() / b);
}

namespace tigl
{

const double CTiglWingProfilePointList::c_trailingEdgeRelGap = 1E-2;
const double CTiglWingProfilePointList::c_blendingDistance = 0.1;

// Constructor
CTiglWingProfilePointList::CTiglWingProfilePointList(const CCPACSWingProfile& profile, const CCPACSPointListXYZ& cpacsPointList)
    : profileRef(profile)
    , coordinates(cpacsPointList.AsVector())
    , profileWireAlgo(new CTiglInterpolateBsplineWire)
    , wireCache(*this, &CTiglWingProfilePointList::BuildWires)
{
}

void CTiglWingProfilePointList::Invalidate()
{
    wireCache.clear();
}

// Builds the wing profile wire. The returned wire is already transformed by the
// wing profile element transformation.
void CTiglWingProfilePointList::BuildWires(WireCache& cache) const
{
    ITiglWireAlgorithm::CPointContainer points;
    ITiglWireAlgorithm::CPointContainer openPoints, closedPoints;

    for (std::vector<CTiglPoint>::const_iterator it = coordinates.begin(); it != coordinates.end(); ++it) {
        points.push_back(it->Get_gp_Pnt());
    }
    // special handling for supporting opened and closed profiles
    if (points.size() < 2) {
        LOG(ERROR) << "Not enough points defined for Wing Profile" << endl;
        throw CTiglError("Not enough points defined for Wing Profile");
    }
    // close profile if not already closed
    gp_Pnt startPnt = points[0];
    gp_Pnt endPnt = points[points.size()-1];
    // compute list of open and closed profile points
    openPoints = points;
    closedPoints = points;
    if (startPnt.Distance(endPnt) >= Precision::Confusion()) {
        closeProfilePoints(closedPoints);
        // save information that profile is opened by default
        cache.profileIsClosed = false;
    }
    else {
        openProfilePoints(openPoints);
        // save information that profile is closed by default
        cache.profileIsClosed = true;
    }

    // Build wires from wing profile points.
    const ITiglWireAlgorithm& wireBuilder = *profileWireAlgo;

    // CCPACSWingSegment::makeSurfaces cannot handle currently
    // wire with multiple edges. Thus we get problems if we have
    // a linear interpolated wire consting of many edges.
    if (dynamic_cast<const CTiglInterpolateLinearWire*>(&wireBuilder)) {
        LOG(ERROR) << "Linear Wing Profiles are currently not supported" << endl;
        throw CTiglError("Linear Wing Profiles are currently not supported",TIGL_ERROR);
    }

    TopoDS_Wire tempWireOpened = wireBuilder.BuildWire(openPoints, false);
    TopoDS_Wire tempWireClosed = wireBuilder.BuildWire(closedPoints, true);
    if (tempWireOpened.IsNull() || tempWireClosed.IsNull()) {
        throw CTiglError("TopoDS_Wire is null in CTiglWingProfilePointList::BuildWire", TIGL_ERROR);
    }

    //@todo: do we really want to remove all y information? this has to be a bug
    // Apply wing profile transformation to wires
    CTiglTransformation transformation;
    transformation.AddProjectionOnXZPlane();

    TopoDS_Wire tempShapeOpened = TopoDS::Wire(transformation.Transform(tempWireOpened));
    TopoDS_Wire tempShapeClosed = TopoDS::Wire(transformation.Transform(tempWireClosed));
    // the open wire should consist of only 1 edge - lets check
    if (GetNumberOfEdges(tempShapeOpened) != 1 || GetNumberOfEdges(tempShapeClosed) != 1) {
        throw CTiglError("Number of Wing Profile Edges is not 1. Please contact the developers");
    }
    TopExp_Explorer wireExOpened(tempShapeOpened, TopAbs_EDGE);
    TopoDS_Edge profileEdgeTmpOpened = TopoDS::Edge(wireExOpened.Current());
    TopExp_Explorer wireExClosed(tempShapeClosed, TopAbs_EDGE);
    TopoDS_Edge profileEdgeTmpClosed = TopoDS::Edge(wireExClosed.Current());

    BuildLETEPoints(cache);

    // Get the curve of the wire
    Standard_Real u1,u2;
    Handle_Geom_Curve curveOpened = BRep_Tool::Curve(profileEdgeTmpOpened, u1, u2);
    curveOpened = new Geom_TrimmedCurve(curveOpened, u1, u2);
    Handle_Geom_Curve curveClosed = BRep_Tool::Curve(profileEdgeTmpClosed, u1, u2);
    curveClosed = new Geom_TrimmedCurve(curveClosed, u1, u2);

    // Get Leading edge parameter on curve
    double lep_par_opened = GeomAPI_ProjectPointOnCurve(cache.lePoint, curveOpened).LowerDistanceParameter();
    double lep_par_closed = GeomAPI_ProjectPointOnCurve(cache.lePoint, curveClosed).LowerDistanceParameter();

    // upper and lower curve
    Handle(Geom_TrimmedCurve) lowerCurveOpened = new Geom_TrimmedCurve(curveOpened, curveOpened->FirstParameter(), lep_par_opened);
    Handle(Geom_TrimmedCurve) upperCurveOpened = new Geom_TrimmedCurve(curveOpened, lep_par_opened, curveOpened->LastParameter());
    Handle(Geom_TrimmedCurve) lowerCurveClosed = new Geom_TrimmedCurve(curveClosed, curveClosed->FirstParameter(), lep_par_closed);
    Handle(Geom_TrimmedCurve) upperCurveClosed = new Geom_TrimmedCurve(curveClosed, lep_par_closed, curveClosed->LastParameter());

    trimUpperLowerCurve(cache, lowerCurveOpened, upperCurveOpened, curveOpened);
    trimUpperLowerCurve(cache, lowerCurveClosed, upperCurveClosed, curveClosed);

    // upper and lower edges
    cache.lowerWireOpened = BRepBuilderAPI_MakeEdge(lowerCurveOpened);
    cache.upperWireOpened = BRepBuilderAPI_MakeEdge(upperCurveOpened);
    cache.upperLowerEdgeOpened = BRepBuilderAPI_MakeEdge(curveOpened);
    cache.lowerWireClosed = BRepBuilderAPI_MakeEdge(lowerCurveClosed);
    cache.upperWireClosed = BRepBuilderAPI_MakeEdge(upperCurveClosed);
    cache.upperLowerEdgeClosed = BRepBuilderAPI_MakeEdge(curveClosed);

    // Trailing edge points
    gp_Pnt te_up, te_down;
    te_up = upperCurveOpened->EndPoint();
    te_down = lowerCurveOpened->StartPoint();

    //check if we have to close upper and lower wing shells
    // TODO: maybe change the implementation to ensure that this is true, since the 
    //       open profile should always have a trailing edge
    if (te_up.Distance(te_down) > Precision::Confusion()) {
        cache.trailingEdgeOpened = BRepBuilderAPI_MakeEdge(te_up,te_down);
    }
    else {
        cache.trailingEdgeOpened.Nullify();
    }

    // closed profile nevere has a trailing edge
    cache.trailingEdgeClosed.Nullify();
}

// Builds leading and trailing edge points of the wing profile wire.
// The trailing edge point is defined at the center between first and
// last defined Point. The leading edge point is defined as the point
// which is located farmost from the trailing edge point.
// Finally, we correct the trailing edge to make sure, that the GetPoint
// functions work correctly.
void CTiglWingProfilePointList::BuildLETEPoints(WireCache& cache) const
{
    // compute TE point
    gp_Pnt firstPnt = coordinates[0].Get_gp_Pnt();
    gp_Pnt lastPnt = coordinates[coordinates.size() - 1].Get_gp_Pnt();
    double x = (firstPnt.X() + lastPnt.X()) / 2.;
    double y = (firstPnt.Y() + lastPnt.Y())/2.;
    double z = (firstPnt.Z() + lastPnt.Z())/2.;
    cache.tePoint = gp_Pnt(x,y,z);

    // find the point with the max dist to TE point
    cache.lePoint = cache.tePoint;
    for (std::vector<CTiglPoint>::const_iterator it = coordinates.begin(); it != coordinates.end(); ++it) {
        gp_Pnt point = it->Get_gp_Pnt();
        if (cache.tePoint.Distance(point) > cache.tePoint.Distance(cache.lePoint)) {
            cache.lePoint = point;
        }
    }
    // project into x-z plane
    cache.lePoint.SetY(0.);
    cache.tePoint.SetY(0.);

    // shorten chord at te, that upper and lower
    // profile are reachable through cord
    gp_Vec vchord(cache.lePoint, cache.tePoint);
    gp_Vec vfirst(cache.lePoint, firstPnt);
    gp_Vec vlast (cache.lePoint, lastPnt);
    double alphaFirst = vfirst * vchord / vchord.SquareMagnitude();
    double alphaLast  = vlast  * vchord / vchord.SquareMagnitude();
    double alphamin = std::min(alphaFirst, alphaLast);
    cache.tePoint = cache.lePoint.XYZ() + alphamin*(vchord.XYZ());
}

const std::vector<CTiglPoint>& CTiglWingProfilePointList::GetSamplePoints() const {
    return coordinates;
}

// get upper wing profile wire
const TopoDS_Edge& CTiglWingProfilePointList::GetUpperWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        if (wireCache->profileIsClosed) {
            return wireCache->upperWireClosed;
        }
        else {
            return wireCache->upperWireOpened;
        }
        break;
    case SHARP_TRAILINGEDGE:
        return wireCache->upperWireClosed;
        break;
    case BLUNT_TRAILINGEDGE:
        return wireCache->upperWireOpened;
        break;
    default:
        throw CTiglError("Unknown TiglShapeModifier passed to CTiglWingProfilePointList::GetUpperWire");
    }
}

// get lower wing profile wire
const TopoDS_Edge& CTiglWingProfilePointList::GetLowerWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        if (wireCache->profileIsClosed) {
            return wireCache->lowerWireClosed;
        }
        else {
            return wireCache->lowerWireOpened;
        }
        break;
    case SHARP_TRAILINGEDGE:
        return wireCache->lowerWireClosed;
        break;
    case BLUNT_TRAILINGEDGE:
        return wireCache->lowerWireOpened;
        break;
    default:
        throw CTiglError("Unknown TiglShapeModifier passed to CTiglWingProfilePointList::GetLowerWire");
    }
}

// get the upper and lower wing profile combined into one edge
const TopoDS_Edge& CTiglWingProfilePointList::GetUpperLowerWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        if (wireCache->profileIsClosed) {
            return wireCache->upperLowerEdgeClosed;
        }
        else {
            return wireCache->upperLowerEdgeOpened;
        }
        break;
    case SHARP_TRAILINGEDGE:
        return wireCache->upperLowerEdgeClosed;
        break;
    case BLUNT_TRAILINGEDGE:
        return wireCache->upperLowerEdgeOpened;
        break;
    default:
        throw CTiglError("Unknown TiglShapeModifier passed to CTiglWingProfilePointList::GetUpperLowerWire");
    }
}

// get trailing edge
const TopoDS_Edge& CTiglWingProfilePointList::GetTrailingEdge(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        if (wireCache->profileIsClosed) {
            return wireCache->trailingEdgeClosed;
        }
        else {
            return wireCache->trailingEdgeOpened;
        }
        break;
    case SHARP_TRAILINGEDGE:
        return wireCache->trailingEdgeClosed;
        break;
    case BLUNT_TRAILINGEDGE:
        return wireCache->trailingEdgeOpened;
        break;
    default:
        throw CTiglError("Unknown TiglShapeModifier passed to CTiglWingProfilePointList::GetTrailingEdge");
    }
}

// get leading edge point();
const gp_Pnt& CTiglWingProfilePointList::GetLEPoint() const
{
    return wireCache->lePoint;
}

// get trailing edge point();
const gp_Pnt& CTiglWingProfilePointList::GetTEPoint() const
{
    return wireCache->tePoint;
}

// Helper method for closing profile points at trailing edge
void CTiglWingProfilePointList::closeProfilePoints(ITiglWireAlgorithm::CPointContainer& points) const
{
    gp_Pnt startPnt = points.front();
    gp_Pnt endPnt = points.back();
    // points are always sorted beginning at trailing edge point in
    // direction of lower side
    gp_Vec gap(startPnt, endPnt);

    // always keep last x position for determination of upper or lower side
    double lastX = startPnt.X();
    ITiglWireAlgorithm::CPointContainer::iterator it;
    for (it = points.begin(); it != points.end(); ++it) {
        gp_Pnt& pnt = (*it);
        if (pnt.X() >= 1.0 - c_blendingDistance) { // inside the blending range:
            // points are always sorted beginning at trailing edge point in
            // direction of lower side
            bool upperSide = lastX < pnt.X();
            double factor = (pnt.X() - (1.0 - c_blendingDistance)) / c_blendingDistance;
            if (upperSide) { // upper side
                pnt.Translate(-1 * factor * 0.5 * gap);
            }
            else {//lower side
                pnt.Translate(factor * 0.5 * gap);
            }
        }
        lastX = pnt.X();
    }

    // finally set start point identical to the end point, as reference use the one with
    // the x coordinate nearest to 1
    points.back() = points.front();
}

// Helper method for opening profile points at trailing edge
void CTiglWingProfilePointList::openProfilePoints(ITiglWireAlgorithm::CPointContainer& points) const
{
    // Pass 1: determine deltay
    double minZ = 0;
    double maxZ = 0;

    ITiglWireAlgorithm::CPointContainer::iterator it;
    for (it = points.begin(); it != points.end(); ++it) {
        gp_Pnt& pnt = (*it);
        if (pnt.Z() < minZ) {
            minZ = pnt.Z();
        }
        if (pnt.Z() > maxZ) {
            maxZ = pnt.Z();
        }
    }
    double deltay = (maxZ - minZ) * c_trailingEdgeRelGap * 0.5; // applied to upper and lower side
    double lastX = points.begin()->X();
    // Pass 2: apply deltay
    for (it = points.begin(); it != points.end(); ++it) {
        gp_Pnt& pnt = (*it);
        if (pnt.X() >= 1.0 - c_blendingDistance) { // inside the blending range:
            // points are always sorted beginning at trailing edge point in
            // direction of lower side
            bool lowerSide = lastX >= pnt.X();
            if (lowerSide) {
                pnt.SetZ(pnt.Z() - ((pnt.X() - (1.0 - c_blendingDistance)) / c_blendingDistance * deltay));
            }
            else {
                pnt.SetZ(pnt.Z() + ((pnt.X() - (1.0 - c_blendingDistance)) / c_blendingDistance * deltay));
            }
        }
        lastX = pnt.X();
    }
}

void CTiglWingProfilePointList::trimUpperLowerCurve(WireCache& cache, Handle(Geom_TrimmedCurve) lowerCurve, Handle(Geom_TrimmedCurve) upperCurve, Handle_Geom_Curve curve) const
{
    gp_Pnt firstPnt = lowerCurve->StartPoint();
    gp_Pnt lastPnt = upperCurve->EndPoint();

    // Trim upper and lower curve to make sure, that the trailing edge
    // is perpendicular to the chord line
    double tolerance = 1e-4;
    gp_Pln plane(cache.tePoint, gp_Vec(cache.lePoint, cache.tePoint));
    GeomAPI_IntCS int1(lowerCurve, new Geom_Plane(plane));
    if (int1.IsDone() && int1.NbPoints() > 0) {
        Standard_Real u, v, w;
        int1.Parameters(1, u, v, w);
        if (w > lowerCurve->FirstParameter() + Precision::Confusion() && w < lowerCurve->LastParameter()) {
            double relDist = lowerCurve->Value(w).Distance(firstPnt) / cache.tePoint.Distance(cache.lePoint);
            if (relDist > tolerance) {
                LOG(WARNING) << "The wing profile " << profileRef.GetUID() << " will be trimmed"
                    << " to avoid a skewed trailing edge."
                    << " The lower part is trimmed about " << relDist*100. << " % w.r.t. the chord length."
                    << " Please correct the wing profile!";
            }
            lowerCurve = new Geom_TrimmedCurve(lowerCurve, w, lowerCurve->LastParameter());
            curve = new Geom_TrimmedCurve(curve, w, curve->LastParameter());
        }
    }
    GeomAPI_IntCS int2(upperCurve, new Geom_Plane(plane));
    if (int2.IsDone() && int2.NbPoints() > 0) {
        Standard_Real u, v, w;
        int2.Parameters(1, u, v, w);
        if (w < upperCurve->LastParameter() - Precision::Confusion() && w > upperCurve->FirstParameter()) {
            double relDist = upperCurve->Value(w).Distance(lastPnt) / cache.tePoint.Distance(cache.lePoint);
            if (relDist > tolerance) {
                LOG(WARNING) << "The wing profile " << profileRef.GetUID() << " will be trimmed"
                    << " to avoid a skewed trailing edge."
                    << " The upper part is trimmed about " << relDist*100. << " % w.r.t. the chord length."
                    << " Please correct the wing profile!";
            }
            upperCurve = new Geom_TrimmedCurve(upperCurve, upperCurve->FirstParameter(), w);
            curve = new Geom_TrimmedCurve(curve, curve->FirstParameter(), w);
        }
    }
}

bool CTiglWingProfilePointList::HasBluntTE() const
{
    gp_Pnt firstPnt = coordinates[0].Get_gp_Pnt();
    gp_Pnt lastPnt  = coordinates[coordinates.size() - 1].Get_gp_Pnt();
    return firstPnt.Distance(lastPnt) > Precision::Confusion();
}

} // end namespace tigl


