/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
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
* @brief  Implementation of CPACS fuselage profile handling routines.
*/

#include "CCPACSFuselageProfile.h"
#include "CTiglError.h"
#include "CTiglTransformation.h"
#include "TopoDS.hxx"
#include "TopoDS_Wire.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "GC_MakeSegment.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "Geom_TrimmedCurve.hxx"
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
#include "CTiglInterpolateBsplineWire.h"
#include "tiglcommonfunctions.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"

#include "math.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>


namespace tigl
{

// Constructor
CCPACSFuselageProfile::CCPACSFuselageProfile()
    : invalidated(true)
{
    profileWireAlgo = WireAlgoPointer(new CTiglInterpolateBsplineWire);
    Cleanup();
}

// Destructor
CCPACSFuselageProfile::~CCPACSFuselageProfile(void)
{
    delete profileWireAlgo;
    Cleanup();
}

// Cleanup routine
void CCPACSFuselageProfile::Cleanup(void)
{
    m_name       = "";
    m_uID        = "";
    m_description= "";
    wireLength = 0.0;
    mirrorSymmetry = false;

    Invalidate();
}

// Read fuselage profile file
void CCPACSFuselageProfile::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    Cleanup();
    generated::CPACSProfileGeometry::ReadCPACS(tixiHandle, xpath);

    // TODO: m_symmetry can neber be half
    //if (symString == "half") {
    //	mirrorSymmetry = true;
    //}
    mirrorSymmetry = false;

    // convert point list to coordinates
    if (m_pointList_choice1.isValid()) {
        const auto& xs = m_pointList_choice1->GetX().GetVector();
        const auto& ys = m_pointList_choice1->GetY().GetVector();
        const auto& zs = m_pointList_choice1->GetZ().GetVector();

        // points with maximal/minimal z-component
        double maxY = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        int maxYIndex = -1;
        int minYIndex = -1;
        // Loop over all points in the vector
        for (int i = 0; i < xs.size(); i++) {
            CTiglPoint point(xs[i], ys[i], zs[i]);
            coordinates.push_back(point);
            if (ys[i] > maxY) {
                maxY = ys[i];
                maxYIndex = i;
            }
            if (ys[i] < minY) {
                minY = ys[i];
                minYIndex = i;
            }
        }

        if (!mirrorSymmetry) {
            // check if points with maximal/minimal y-component were calculated correctly
            if (maxYIndex == -1 || minYIndex == -1 || maxYIndex == minYIndex) {
                throw CTiglError("Error: CCPACSWingProfilePointList::ReadCPACS: Unable to separate upper and lower wing profile from point list", TIGL_XML_ERROR);
            }
            // force order of points to run through y>0 part first
            if (minYIndex < maxYIndex) {
                LOG(WARNING) << "The point list order in fuselage profile " << m_uID << " is reversed in order to run through y>0 part first" << endl;
                std::reverse(coordinates.begin(), coordinates.end());
            }
        }
    }

    Update();
}

// Write fuselage profile file
void CCPACSFuselageProfile::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
{
    // update point lists from coordinates
    if (m_pointList_choice1.isValid()) {
        auto& xs = const_cast<std::vector<double>&>(m_pointList_choice1->GetX().GetVector());
        auto& ys = const_cast<std::vector<double>&>(m_pointList_choice1->GetY().GetVector());
        auto& zs = const_cast<std::vector<double>&>(m_pointList_choice1->GetZ().GetVector());

        xs.resize(coordinates.size());
        ys.resize(coordinates.size());
        zs.resize(coordinates.size());

        for (unsigned int j = 0; j < coordinates.size(); j++) {
            xs[j] = coordinates[j].x;
            ys[j] = coordinates[j].y;
            zs[j] = coordinates[j].z;
        }
    }

    generated::CPACSProfileGeometry::WriteCPACS(tixiHandle, xpath);
}

const int CCPACSFuselageProfile::GetNumPoints(void) const 
{
    return static_cast<int>(coordinates.size());
}

// Returns the flag for the mirror symmetry with respect to the x-z-plane in the fuselage profile
bool CCPACSFuselageProfile::GetMirrorSymmetry(void) const
{
    return mirrorSymmetry;
}

// Invalidates internal fuselage profile state
void CCPACSFuselageProfile::Invalidate(void)
{
    invalidated = true;
}

// Update the internal state, i.g. recalculates wire
void CCPACSFuselageProfile::Update(void)
{
    if (!invalidated) {
        return;
    }

    BuildWires();
    invalidated = false;
}

// Returns the fuselage profile wire
TopoDS_Wire CCPACSFuselageProfile::GetWire(bool forceClosed)
{
    Update();
    return (forceClosed ? wireClosed : wireOriginal);
}

// check if the distance between two points are below a fixed value, so that
// these point could be imaged as "equal".
bool CCPACSFuselageProfile::checkSamePoints(gp_Pnt pointA, gp_Pnt pointB)
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
void CCPACSFuselageProfile::BuildWires(void)
{
    ITiglWireAlgorithm::CPointContainer points;

    if (coordinates.size() < 2) {
        throw CTiglError("Error: Number of points is less than 2 in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);
    }

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
    gp_Pnt pStart =  coordinates[0].Get_gp_Pnt();
    gp_Pnt pEnd   =  coordinates[coordinates.size()-1].Get_gp_Pnt();
    if (checkSamePoints(pStart,pEnd)) {
        points.push_back(pStart);
    }
    else {
        points.push_back(pEnd);
    }

    // Build wire from fuselage profile points
    const ITiglWireAlgorithm& wireBuilder = *profileWireAlgo;
    const CTiglInterpolateBsplineWire* pSplineBuilder = dynamic_cast<const CTiglInterpolateBsplineWire*>(&wireBuilder);
    if (pSplineBuilder) {
        const_cast<CTiglInterpolateBsplineWire*>(pSplineBuilder)->setEndpointContinuity(C1);
    }

    TopoDS_Wire tempWireClosed   = wireBuilder.BuildWire(points, true);
    TopoDS_Wire tempWireOriginal = wireBuilder.BuildWire(points, false);
    if (pSplineBuilder) {
        const_cast<CTiglInterpolateBsplineWire*>(pSplineBuilder)->setEndpointContinuity(C0);
    }

    if (tempWireClosed.IsNull() == Standard_True || tempWireOriginal.IsNull() == Standard_True) {
        throw CTiglError("Error: TopoDS_Wire is null in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);
    }

    // Apply fuselage profile transformation to wire
    CTiglTransformation transformation;
    TopoDS_Shape tempShapeClosed   = transformation.Transform(tempWireClosed);
    TopoDS_Shape tempShapeOriginal = transformation.Transform(tempWireOriginal);

    // Cast shapes to wires, see OpenCascade documentation
    if (tempShapeClosed.ShapeType() != TopAbs_WIRE || tempShapeOriginal.ShapeType() != TopAbs_WIRE) {
        throw CTiglError("Error: Wrong shape type in CCPACSFuselageProfile::BuildWire", TIGL_ERROR);
    }

    wireClosed   = TopoDS::Wire(tempShapeClosed);
    wireOriginal = TopoDS::Wire(tempShapeOriginal);

    wireLength = GetWireLength(wireOriginal);
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
gp_Pnt CCPACSFuselageProfile::GetPoint(double zeta)
{
    Update();

    if (zeta < 0.0 || zeta > 1.0) {
        throw CTiglError("Error: Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CCPACSFuselageProfile::GetPoint", TIGL_ERROR);
    }

    double length = wireLength * zeta;

    // Get the first edge of the wire
    BRepTools_WireExplorer wireExplorer(wireOriginal);
    if (!wireExplorer.More()) {
        throw CTiglError("Error: Not enough edges found in CCPACSFuselageProfile::GetPoint", TIGL_ERROR);
    }
    Standard_Real firstParam;
    Standard_Real lastParam;
    TopoDS_Edge edge = wireExplorer.Current();
    wireExplorer.Next();
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, firstParam, lastParam);

    // Length of current edge
    GeomAdaptor_Curve adaptorCurve;
    adaptorCurve.Load(curve);
    double currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

    // Length of complete wire up to now
    double sumLength = currLength;

    while (length > sumLength) {
        if (!wireExplorer.More()) {
            break;
        }
        edge = wireExplorer.Current();
        wireExplorer.Next();

        curve = BRep_Tool::Curve(edge, firstParam, lastParam);
        adaptorCurve.Load(curve);

        // Length of current edge
        currLength = GCPnts_AbscissaPoint::Length(adaptorCurve);

        // Length of complete wire up to now
        sumLength += currLength;
    }

    // Distance of searched point from end point of current edge
    double currEndDelta = std::max((sumLength - length), 0.0);

    // Distance of searched point from start point of current edge
    double currDist = std::max((currLength - currEndDelta), 0.0);

    GCPnts_AbscissaPoint abscissaPoint(adaptorCurve, currDist, adaptorCurve.FirstParameter());
    gp_Pnt point = adaptorCurve.Value(abscissaPoint.Parameter());

    return point;
}


void CCPACSFuselageProfile::BuildDiameterPoints(void)
{
    Update();
    if (mirrorSymmetry) {
        startDiameterPoint = coordinates[0].Get_gp_Pnt();
        endDiameterPoint = coordinates[coordinates.size() - 1].Get_gp_Pnt();
    }
    else {
        // compute starting diameter point
        gp_Pnt firstPnt = coordinates[0].Get_gp_Pnt();
        gp_Pnt lastPnt  = coordinates[coordinates.size() - 1].Get_gp_Pnt();
        double x = (firstPnt.X() + lastPnt.X())/2.;
        double y = (firstPnt.Y() + lastPnt.Y())/2.;
        double z = (firstPnt.Z() + lastPnt.Z())/2.;
        startDiameterPoint = gp_Pnt(x,y,z);

        // find the point with the max dist to starting point
        endDiameterPoint = startDiameterPoint;
        for (auto it = coordinates.begin(); it != coordinates.end(); ++it) {
            gp_Pnt point = it->Get_gp_Pnt();
            if (startDiameterPoint.Distance(point) > startDiameterPoint.Distance(endDiameterPoint)) {
                endDiameterPoint = point;
            }
        }
        // project into x-z plane
        endDiameterPoint.SetY(0.);
        startDiameterPoint.SetY(0.);
    }
}

// Returns the profile points as read from TIXI.
std::vector<CTiglPoint*> CCPACSFuselageProfile::GetCoordinateContainer()
{
    std::vector<CTiglPoint*> newPointVector;
    for (std::size_t i = 0; i < coordinates.size(); i++) {
        gp_Pnt pnt = coordinates[i].Get_gp_Pnt();
        pnt = TransformPoint(pnt);
        newPointVector.push_back(new CTiglPoint(pnt.X(), pnt.Y(), pnt.Z()));
    }
    return newPointVector;
}

TopoDS_Wire CCPACSFuselageProfile::GetDiameterWire()
{
    BuildDiameterPoints();
    Handle(Geom_TrimmedCurve) diameterCurve = GC_MakeSegment(startDiameterPoint, endDiameterPoint);
    TopoDS_Edge diameterEdge = BRepBuilderAPI_MakeEdge(diameterCurve);
    TopoDS_Wire diameterWire = BRepBuilderAPI_MakeWire(diameterEdge);
    return diameterWire;
}

} // end namespace tigl

