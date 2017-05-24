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
* @brief  Implementation of CPACS wing ComponentSegment handling routines.
*/

#include <cmath>
#include <iostream>
#include <algorithm>
#include <string>
#include <limits>
#include <cassert>

#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSection.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingProfile.h"
#include "CCPACSWingComponentSegments.h"
#include "CTiglLogging.h"
#include "CCPACSWingCell.h"
#include "CTiglApproximateBsplineWire.h"
#include "CPointsToLinearBSpline.h"
#include "tiglcommonfunctions.h"
#include "CCPACSWingCSStructure.h"
#include "CNamedShape.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Wire.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "Extrema_ExtCC.hxx"
#include "Geom_Plane.hxx"
#include "Geom_Line.hxx"
#include "gp_Pln.hxx"
#include "Precision.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "BRepTools.hxx"
#include "BRepGProp.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "BRepClass3d_SolidClassifier.hxx"
#include "TColgp_Array1OfPnt.hxx"
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <BRepAdaptor_CompCurve.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <GC_MakeSegment.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Pln.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <TopExp.hxx>

#include "CTiglLogging.h"
#include <cassert>

namespace tigl
{

namespace
{
    bool inBetween(const gp_Pnt& p, const gp_Pnt& p1, const gp_Pnt& p2) // TODO: move to utils?
    {
        gp_Vec b(p1, p2);
        gp_Vec v1(p, p1);
        gp_Vec v2(p, p2);

        double res = (b*v1)*(b*v2);
        return res <= 0.;
    }

    double GetNearestValidParameter(double p) // TODO: this is clamp(p, 0, 1), aka. saturate(p), rename?
    {
        if (p < 0.) {
            return 0.;
        }
        else if ( p > 1.) {
            return 1.;
        }
        return p;
    }

    // Set the face traits
    void SetFaceTraits (PNamedShape loft, unsigned int nSegments) 
    { 
        // designated names of the faces
        std::vector<std::string> names(3); // TODO: use std::array
        names[0]="Bottom";
        names[1]="Top";
        names[2]="TrailingEdge";
        std::vector<std::string> endnames(2);
        endnames[0]="Inside";
        endnames[1]="Outside";

        // map of faces
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(loft->Shape(),   TopAbs_FACE, map);

        unsigned int nFaces = map.Extent();
        // check if number of faces without inside and outside surface (nFaces-2) 
        // is a multiple of 2 (without Trailing Edges) or 3 (with Trailing Edges)
        if (!((nFaces-2)/nSegments == 2 || (nFaces-2)/nSegments == 3) || nFaces < 4) {
            LOG(ERROR) << "CCPACSWingComponentSegment: Unable to determine name face names from component segment loft";
            return;
        }
        // remove trailing edge name if there is no trailing edge
        if ((nFaces-2)/nSegments == 2) {
            names.erase(names.begin()+2);
        }
        // assign "Top" and "Bottom" to face traits
        for (unsigned int i = 0; i < nFaces-2; i++) {
            CFaceTraits traits = loft->GetFaceTraits(i);
            traits.SetName(names[i%names.size()].c_str());
            loft->SetFaceTraits(i, traits);
        }
        // assign "Inside" and "Outside" to face traits
        for (unsigned int i = nFaces-2; i < nFaces; i++) {
            CFaceTraits traits = loft->GetFaceTraits(i);
            traits.SetName(endnames[i-nFaces+2].c_str());
            loft->SetFaceTraits(i, traits);
        }
    }
}

CCPACSWingComponentSegment::CCPACSWingComponentSegment(CCPACSWingComponentSegments* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSComponentSegment(parent, uidMgr)
    , _uidMgr(uidMgr)
    , CTiglAbstractSegment(parent->GetComponentSegments(), parent->GetParent()->m_symmetry)
    , wing(parent->GetParent())
    , chordFace(*this, uidMgr)
    , surfacesAreValid(false)
{
    assert(wing != NULL);
    Cleanup();
}

// Destructor
CCPACSWingComponentSegment::~CCPACSWingComponentSegment()
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWingComponentSegment::Invalidate()
{
    // call parent class instead of directly setting invalidated flag
    CTiglAbstractSegment::Reset();
    surfacesAreValid = false;
    projLeadingEdge.Nullify();
    wingSegments.clear();
    if (m_structure) {
        m_structure->Invalidate();
    }
    linesAreValid = false;
    chordFace.Reset();
}

// Cleanup routine
void CCPACSWingComponentSegment::Cleanup()
{
    m_name = "";
    m_fromElementUID = "";
    m_toElementUID   = "";
    myVolume       = 0.;
    mySurfaceArea  = 0.;
    surfacesAreValid = false;
    linesAreValid = false;
    CTiglAbstractSegment::Reset();
    projLeadingEdge.Nullify();
    wingSegments.clear();
}

// Update internal segment data
void CCPACSWingComponentSegment::Update()
{
    Invalidate();

    chordFace.SetUID(GetDefaultedUID() + "_chordface");
}

// Read CPACS segment elements
void CCPACSWingComponentSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
{
    Cleanup();
    generated::CPACSComponentSegment::ReadCPACS(tixiHandle, segmentXPath);
    Update();
}

std::string CCPACSWingComponentSegment::GetDefaultedUID() const {
    return m_uID;
}

// Returns the wing this segment belongs to
CCPACSWing& CCPACSWingComponentSegment::GetWing() const {
    return *wing;
}

// Getter for upper Shape
TopoDS_Shape CCPACSWingComponentSegment::GetUpperShape()
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return upperShape;
}

// Getter for lower Shape
TopoDS_Shape CCPACSWingComponentSegment::GetLowerShape()
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return lowerShape;
}

// Getter for inner segment face
TopoDS_Face CCPACSWingComponentSegment::GetInnerFace()
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return innerFace;
}

// Getter for outer segment face
TopoDS_Face CCPACSWingComponentSegment::GetOuterFace()
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return outerFace;
}

// Getter for midplane points
// Returns the inner or outer chordline points, in case eta==0 or eta==1
// Otherwise returns the point at the defined eta/xsi coordinate
// Points are returned relative to the wing coordinate system
gp_Pnt CCPACSWingComponentSegment::GetMidplaneOrChordlinePoint(double eta, double xsi) const
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetMidplaneOrChordlinePoint", TIGL_ERROR);
    }
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingComponentSegment::GetMidplaneOrChordlinePoint", TIGL_ERROR);
    }

    gp_Pnt p;
    if (eta <= Precision::Confusion()) {
        p = GetSegmentList().front()->GetChordPoint(0, xsi);
    }
    else if (eta >= (1-Precision::Confusion())) {
        p = GetSegmentList().back()->GetChordPoint(1, xsi);
    }
    else {
        p = GetPoint(eta, xsi);
    }
    return wing->GetWingTransformation().Inverted().Transform(p);
}

// Getter for leading edge point
gp_Pnt CCPACSWingComponentSegment::GetLeadingEdgePoint(double referencePos) const
{
    // get point on leading edge line
    gp_Pnt lePnt;
    BRepAdaptor_CompCurve leLineCurve(GetLeadingEdgeLine(), Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length( leLineCurve );
    leLineCurve.D0(len * referencePos, lePnt);

    return lePnt;
}

// Getter for trailing edge point
gp_Pnt CCPACSWingComponentSegment::GetTrailingEdgePoint(double referencePos) const
{
    // get point on trailing edge line
    gp_Pnt tePnt;
    BRepAdaptor_CompCurve teLineCurve(GetTrailingEdgeLine(), Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length( teLineCurve );
    teLineCurve.D0(len * referencePos, tePnt);

    return tePnt;
}

TopoDS_Face CCPACSWingComponentSegment::GetSectionElementFace(const std::string& sectionElementUID) const
{
    // get all segments for this component segment
    const SegmentList& segmentList = GetSegmentList();
    SegmentList::const_iterator it;
    // iterate over all segments
    for (it = segmentList.begin(); it != segmentList.end(); ++it) {
        // check for inner section of first segment
        if (it == segmentList.begin()) {
            if ((*it)->GetInnerSectionElementUID() == sectionElementUID) {
                return GetSingleFace((*it)->GetInnerClosure(WING_COORDINATE_SYSTEM));
            }
        }
        if ((*it)->GetOuterSectionElementUID() == sectionElementUID) {
            return GetSingleFace((*it)->GetOuterClosure(WING_COORDINATE_SYSTEM));
        }
    }
    throw CTiglError("Unable to find section element with UID \"" + sectionElementUID + "\" in component segment \"" + m_uID + "\"!");
}


// Getter for the normalized leading edge direction
gp_Vec CCPACSWingComponentSegment::GetLeadingEdgeDirection(const std::string& segmentUID) const
{
    tigl::CCPACSWingSegment& segment = wing->GetSegment(segmentUID);
    gp_Pnt pl0 = segment.GetPoint(0, 0, false, WING_COORDINATE_SYSTEM);
    gp_Pnt pl1 = segment.GetPoint(1, 0, false, WING_COORDINATE_SYSTEM);

    // get the normalized leading edge vector
    gp_Vec lev(pl0, pl1);
    lev.Normalize();
    return lev;
}

// Getter for the normalized leading edge direction
// Parameter defaultSegmentUID can be used for inner/outer segment when point is in extended volume
gp_Vec CCPACSWingComponentSegment::GetLeadingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    gp_Pnt globalPnt = wing->GetWingTransformation().Transform(point);

    std::string segmentUID = defaultSegmentUID;
    gp_Pnt dummy;
    double deviation = 0;
    const CCPACSWingSegment* segment = findSegment(globalPnt.X(), globalPnt.Y(), globalPnt.Z(), dummy, deviation);
    if (segment != NULL) {
        segmentUID = segment->GetUID();
    }
    return GetLeadingEdgeDirection(segmentUID);
}

// Getter for the normalized trailing edge direction
gp_Vec CCPACSWingComponentSegment::GetTrailingEdgeDirection(const std::string& segmentUID) const
{
    tigl::CCPACSWingSegment& segment = wing->GetSegment(segmentUID);
    gp_Pnt pt0 = segment.GetPoint(0, 1, false, WING_COORDINATE_SYSTEM);
    gp_Pnt pt1 = segment.GetPoint(1, 1, false, WING_COORDINATE_SYSTEM);

    // get the normalized trailing edge vector
    gp_Vec tev(pt0, pt1);
    tev.Normalize();
    return tev;
}

// Getter for the normalized trailing edge direction
// Parameter defaultSegmentUID can be used for inner/outer segment when point is in extended volume
gp_Vec CCPACSWingComponentSegment::GetTrailingEdgeDirection(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    gp_Pnt globalPnt = wing->GetWingTransformation().Transform(point);

    std::string segmentUID = defaultSegmentUID;
    gp_Pnt dummy;
    double deviation = 0.;
    const CCPACSWingSegment* segment = findSegment(globalPnt.X(), globalPnt.Y(), globalPnt.Z(), dummy, deviation);
    if (segment != NULL) {
        segmentUID = segment->GetUID();
    }

    return GetTrailingEdgeDirection(segmentUID);
}

// Getter for the length of the leading edge between two eta values
double CCPACSWingComponentSegment::GetLeadingEdgeLength() const
{
    BRepAdaptor_CompCurve leLineCurve(GetLeadingEdgeLine(), Standard_True);
    double length = GCPnts_AbscissaPoint::Length( leLineCurve );
    return length;
}

// Getter for the length of the trailing edge between two eta values
double CCPACSWingComponentSegment::GetTrailingEdgeLength() const
{
    BRepAdaptor_CompCurve teLineCurve(GetTrailingEdgeLine(), Standard_True);
    double length = GCPnts_AbscissaPoint::Length( teLineCurve );
    return length;
}

// Getter for the midplane line between two eta-xsi points
TopoDS_Wire CCPACSWingComponentSegment::GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint) const
{
    // TODO: this method creates a straight line between the start- and end-point within the single
    // segments, while the real line could be a curve (depending on the twist and the position of
    // the points)

    // determine start and end point
    // copy of variables because we have to modify them in case the points lie within a section
    gp_Pnt startPnt = startPoint;
    gp_Pnt endPnt = endPoint;
    gp_Pnt globalStartPnt = wing->GetWingTransformation().Transform(startPnt);
    gp_Pnt globalEndPnt = wing->GetWingTransformation().Transform(endPnt);

    // determine wing segments containing the start and end points
    std::string startSegmentUID, endSegmentUID;
    gp_Pnt dummy;
    double deviation = 0.;
    const CCPACSWingSegment* startSegment = findSegment(globalStartPnt.X(), globalStartPnt.Y(), globalStartPnt.Z(), dummy, deviation);
    if (startSegment != NULL) {
        startSegmentUID = startSegment->GetUID();
    }
    const CCPACSWingSegment* endSegment = findSegment(globalEndPnt.X(), globalEndPnt.Y(), globalEndPnt.Z(), dummy, deviation);
    if (endSegment != NULL) {
        endSegmentUID = endSegment->GetUID();
    }

    // In case no segment can be found expect the inner segment for the inner point and the outer segment for the
    // outer point.
    // This can occur when the inner segment has a z-rotation, because of the extension of the leading/trailing edge
    // for the determination of the ETA line. See CPACS documentation for details
    bool startPntInSection = false;
    bool endPntInSection = false;
    if (startSegmentUID == "") {
        startSegmentUID = GetInnerSegmentUID();
        startPntInSection = true;
    }
    if (endSegmentUID == "") {
        endSegmentUID = GetOuterSegmentUID();
        endPntInSection = true;
    }

    BRepBuilderAPI_MakeWire wireBuilder;

    // get minimum and maximum z-value of bounding box
    Bnd_Box bbox;
    BRepBndLib::Add(loft->Shape(), bbox);
    double zmin, zmax, temp;
    bbox.Get(temp, temp, zmin, temp, temp, zmax);

    // build cut face
    gp_Pnt p0 = startPnt;
    gp_Pnt p1 = startPnt;
    gp_Pnt p2 = endPnt;
    gp_Pnt p3 = endPnt;
    p0.SetZ(zmin);
    p1.SetZ(zmax);
    p2.SetZ(zmin);
    p3.SetZ(zmax);
    TopoDS_Face cutFace = BuildFace(p0, p1, p2, p3);

    // compute start and end point from intersection with inner/outer section in
    // case the points lie outside of the segments
    if (startPntInSection) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&)wing->GetSegment(startSegmentUID);
        gp_Pnt pl = segment.GetPoint(0, 0, true, WING_COORDINATE_SYSTEM);
        gp_Pnt pt = segment.GetPoint(0, 1, true, WING_COORDINATE_SYSTEM);
        TopoDS_Edge chordLine = BRepBuilderAPI_MakeEdge(pl, pt);
        GetIntersectionPoint(cutFace, chordLine, startPnt);
    }
    if (endPntInSection) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&)wing->GetSegment(endSegmentUID);
        gp_Pnt pl = segment.GetPoint(1, 0, true, WING_COORDINATE_SYSTEM);
        gp_Pnt pt = segment.GetPoint(1, 1, true, WING_COORDINATE_SYSTEM);
        TopoDS_Edge chordLine = BRepBuilderAPI_MakeEdge(pl, pt);
        GetIntersectionPoint(cutFace, chordLine, endPnt);
    }

    // handle case when start and end point are in the same segment
    if (startSegmentUID == endSegmentUID) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(startPnt, endPnt);
        wireBuilder.Add(edge);
        return wireBuilder.Wire();
    }

    const SegmentList& segments = GetSegmentList();
    SegmentList::const_iterator it;
    gp_Pnt prevPnt = startPnt;
    // first find start segment
    for (it = segments.begin(); it != segments.end(); ++it) {
        if ((*it)->GetUID() == startSegmentUID) {
            break;
        }
    }
    // next iterate until endSegmentUID
    for (; it != segments.end(); ++it) {
        const CCPACSWingSegment& segment = *(*it);
        // add intersection with end section only in case end point is skipped
        if (segment.GetUID() != endSegmentUID) {
            // compute outer chord line
            gp_Pnt pl = segment.GetPoint(1, 0, true, WING_COORDINATE_SYSTEM);
            gp_Pnt pt = segment.GetPoint(1, 1, true, WING_COORDINATE_SYSTEM);

            TopoDS_Edge outerChordLine = BRepBuilderAPI_MakeEdge(pl, pt);
            // cut outer chord line with reference face
            gp_Pnt nextPnt;
            if (GetIntersectionPoint(cutFace, outerChordLine, nextPnt)) {
                // only build new edge if start and end point are not equal (can occur
                // when the startPoint lies within a section, because then findSegment
                // returns the inner segment first
                if (!prevPnt.IsEqual(nextPnt, Precision::Confusion())) {
                    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(prevPnt, nextPnt);
                    wireBuilder.Add(edge);
                    prevPnt = nextPnt;
                }
            }
            else {
                throw CTiglError("Unable to build midline for component segment: intersection with section chord line failed!");
            }
        }
        else {
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(prevPnt, endPnt);
            wireBuilder.Add(edge);
            break;
        }
    }
    return wireBuilder.Wire();
}

// Getter for the leading edge line
const TopoDS_Wire& CCPACSWingComponentSegment::GetLeadingEdgeLine() const
{
    if (!linesAreValid) {
        BuildLines();
    }
    return leadingEdgeLine;
}

// Getter for the trailing edge line
const TopoDS_Wire& CCPACSWingComponentSegment::GetTrailingEdgeLine() const
{
    if (!linesAreValid) {
        BuildLines();
    }
    return trailingEdgeLine;
}

const std::string& CCPACSWingComponentSegment::GetInnerSegmentUID() const
{
    return GetSegmentList().front()->GetUID();
}

const std::string& CCPACSWingComponentSegment::GetOuterSegmentUID() const
{
    return GetSegmentList().back()->GetUID();
}

SegmentList& CCPACSWingComponentSegment::GetSegmentList() const
{
    if (wingSegments.size() == 0) {
        std::vector<int> path;
        path = findPath(m_fromElementUID, m_toElementUID, path, true);

        if (path.size() == 0) {
            // could not find path from fromUID to toUID
            // try the other way around
            path = findPath(m_toElementUID, m_fromElementUID, path, true);
        }

        if (path.size() == 0) {
            LOG(WARNING) << "Could not determine segment list to component segment from \""
                            << GetFromElementUID() << "\" to \"" << GetToElementUID() << "\"!";
        }

        std::vector<int>::iterator it;
        for (it = path.begin(); it != path.end(); ++it) {
            CCPACSWingSegment* pSeg = static_cast<CCPACSWingSegment*>(&(GetWing().GetSegment(*it)));
            wingSegments.push_back(pSeg);
        }
    }

    return wingSegments;
}
    
// Determines, which segments belong to the component segment
std::vector<int> CCPACSWingComponentSegment::findPath(const std::string& fromUID, const::std::string& toUID, const std::vector<int>& curPath, bool forward) const
{
    if ( fromUID == toUID ) {
        return curPath;
    }
        
    // find all segments with InnerSectionUID == fromUID
    std::vector<int> segList;
    for (int i = 1; i <= wing->GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(i);
        std::string startUID = forward ? segment.GetInnerSectionElementUID() : segment.GetOuterSectionElementUID();
        if (startUID == fromUID) {
            segList.push_back(i);
        }
    }
        
    std::vector<int>::iterator segIt = segList.begin();
    for (; segIt != segList.end(); ++segIt) {
        int iseg = *segIt;
        CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(iseg);
        std::vector<int> newpath(curPath);
        newpath.push_back(iseg);
        std::string segEndUID = forward ? segment.GetOuterSectionElementUID() : segment.GetInnerSectionElementUID();
        std::vector<int> result = findPath(segEndUID, toUID, newpath, forward);
        if (result.size() != 0) {
            return result;
        }
    }
        
    // return empty list as path could not be found
    std::vector<int> result;
    return result;
}
    
TopoDS_Wire CCPACSWingComponentSegment::GetCSLine(double eta1, double xsi1, double eta2, double xsi2, int NSTEPS)
{
    BRepBuilderAPI_MakeWire wireBuilder;
        
    gp_Pnt  old_point = GetPoint(eta1,xsi1);
    for (int istep = 1; istep < NSTEPS; ++istep) {
        double eta = eta1 + (double) istep/(double) (NSTEPS-1) * (eta2-eta1);
        double xsi = xsi1 + (double) istep/(double) (NSTEPS-1) * (xsi2-xsi1);
        gp_Pnt point = GetPoint(eta,xsi);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(old_point, point);
        wireBuilder.Add(edge);
        old_point = point;
    }
    return wireBuilder.Wire();
}
    
void CCPACSWingComponentSegment::GetSegmentIntersection(const std::string& segmentUID, double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, double &xsi)
{
    CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(segmentUID);

    // compute component segment line
    gp_Pnt p1 = GetPoint(csEta1, csXsi1);
    gp_Pnt p2 = GetPoint(csEta2, csXsi2);
    double csLen = p1.Distance(p2);

    gp_Lin csLine(p1, p2.XYZ() - p1.XYZ());

    // compute iso eta line of segment
    gp_Pnt pLE = segment.GetChordPoint(eta, 0.);
    gp_Pnt pTE = segment.GetChordPoint(eta, 1.);
    double chordDepth = pTE.Distance(pLE);

    gp_Lin etaLine(pLE, pTE.XYZ() - pLE.XYZ());

    // check, if both lines are parallel
    if (etaLine.Direction().IsParallel(csLine.Direction(), M_PI/180.)) {
        throw CTiglError("Component segment line does not intersect iso eta line of segment in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
        }

    Handle(Geom_Curve) csCurve = new Geom_Line(csLine);
    Handle(Geom_Curve) etaCurve = new Geom_Line(etaLine);
    GeomAdaptor_Curve csAdptAcuve(csCurve);
    GeomAdaptor_Curve etaAdptCurve(etaCurve);

    // find point on etaLine, that minimizes distance to csLine
    Extrema_ExtCC minimizer(csAdptAcuve, etaAdptCurve);
    minimizer.Perform();

    if (!minimizer.IsDone()) {
        throw CTiglError("Component segment line does not intersect iso eta line of segment in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
        }

    // there should be exactly on minimum between two lines
    // if they are not parallel
    assert(minimizer.NbExt() == 1);

    Extrema_POnCurv pOnCSLine, pOnEtaLine;
    minimizer.Points(1, pOnCSLine, pOnEtaLine);

    // If parameter on CS line is < 0 or larger than 
    // Length of line, there is not actual intersection,
    // i.e. the CS Line is choosen to small
    // We use a tolerance here, to account for small user errors
    double tol = 1e-5;
    if (pOnCSLine.Parameter() < -tol || pOnCSLine.Parameter() > csLen + tol) {
        throw CTiglError("Component segment line does not intersect iso eta line of segment in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
            }

    // compute xsi value
    xsi = pOnEtaLine.Parameter()/chordDepth;
}

void CCPACSWingComponentSegment::InterpolateOnLine(double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, double &xsi, double& errorDistance)
{
    // compute component segment line
    gp_Pnt p1 = GetPoint(csEta1, csXsi1);
    gp_Pnt p2 = GetPoint(csEta2, csXsi2);

    UpdateProjectedLeadingEdge();
    UpdateExtendedChordFaces();

    // get eta plane and compute intersection with line
    // compute eta point and normal on the projected LE
    gp_Pnt etaPnt, intersectionPoint;
    gp_Vec etaNormal;
    projLeadingEdge->D1(eta, etaPnt, etaNormal);

    if (IntersectLinePlane(p1, p2, gp_Pln(etaPnt, etaNormal.XYZ()), intersectionPoint) == NoIntersection) {
        throw CTiglError("Cannot interpolate for the given eta coordinate", TIGL_MATH_ERROR);
        }

    // get xsi coordinate
    gp_Pnt nearestPoint;
    double deviation = 0;
    tigl::CCPACSWingSegment* segment = (tigl::CCPACSWingSegment*) findSegment(intersectionPoint.X(), intersectionPoint.Y(), intersectionPoint.Z(),
                nearestPoint, deviation);

    if (!segment) {
        throw CTiglError("Cannot interpolate for the given eta coordinate", TIGL_MATH_ERROR);
            }

    if (deviation > 1e-3) {
        // There are two options:
        // Either, the point is completely outside the component segment or on the extended
        // surfaces

        double tol = 1e-4;
        double curEta = 0.;
        double curXsi = 0.;
        CTiglPoint nearestPointTmp;

        // Test outer surface
        TiglReturnCode retValProj = extendedOuterChord.translate(intersectionPoint.XYZ(), &curEta, &curXsi);
        if (retValProj == TIGL_SUCCESS && curEta > -tol && curEta < 1. + tol && curXsi > -tol && curXsi < 1. + tol) {
            // we are on the extended surface
            segment = GetSegmentList().front();
            extendedOuterChord.translate(curEta, curXsi, &nearestPointTmp);
            nearestPoint = nearestPointTmp.Get_gp_Pnt();
            xsi = curXsi;
        }
        else {
            retValProj = extendedInnerChord.translate(intersectionPoint.XYZ(), &curEta, &curXsi);
            if (retValProj == TIGL_SUCCESS && curEta > -tol && curEta < 1. + tol && curXsi > -tol && curXsi < 1. + tol) {
                // we are on the extended surface
                segment = GetSegmentList().back();
                extendedInnerChord.translate(curEta, curXsi, &nearestPointTmp);
                nearestPoint = nearestPointTmp.Get_gp_Pnt();
                xsi = curXsi;
            }
            else {
                throw CTiglError("The requested point lies outside the wing chord surface.", TIGL_MATH_ERROR);
            }
        }
    }
    else {
        double etaRes, xsiRes;
        segment->GetEtaXsi(nearestPoint, etaRes, xsiRes);
        xsi = xsiRes;
    }

    // compute the error distance
    // This is the distance from the line to the nearest point on the chord face
    gp_Lin line(p1, p2.XYZ() - p1.XYZ());
    errorDistance = line.Distance(nearestPoint);
}

CTiglWingChordface &CCPACSWingComponentSegment::GetChordface() const
{
    UpdateChordFace();

    return chordFace;
}

// get short name for loft
std::string CCPACSWingComponentSegment::GetShortShapeName() 
{
    unsigned int windex = 0;
    unsigned int wcsindex = 0;
    for (int i = 1; i <= wing->GetConfiguration().GetWingCount(); ++i) {
        tigl::CCPACSWing& w = wing->GetConfiguration().GetWing(i);
        if (wing->GetUID() == w.GetUID()) {
            windex = i;
            for (int j = 1; j <= w.GetComponentSegmentCount(); j++) {
                CCPACSWingComponentSegment& wcs = w.GetComponentSegment(j);
                if (GetUID() == wcs.GetUID()) {
                    wcsindex = j;
                    std::stringstream shortName;
                    shortName << "W" << windex << "CS" << wcsindex;
                    return shortName.str();
                }
            }
        }
    }
    return "UNKNOWN";
}

// Builds the loft between the two segment sections
PNamedShape CCPACSWingComponentSegment::BuildLoft()
{
    loft.reset();

    // use sewing for full loft
    BRepBuilderAPI_Sewing sewing;
    BRepBuilderAPI_Sewing upperShellSewing;
    BRepBuilderAPI_Sewing lowerShellSewing;

    const SegmentList& segments = GetSegmentList();
    if (segments.size() == 0) {
        throw CTiglError("Could not find segments in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
    }

    TopoDS_Shape innerShape = segments.front()->GetInnerClosure(WING_COORDINATE_SYSTEM);
    innerFace = GetSingleFace(innerShape);
    sewing.Add(innerFace);

    for (SegmentList::const_iterator it = segments.begin(); it != segments.end(); ++it) {
        CCPACSWingSegment& segment = **it;
        TopoDS_Face lowerSegmentFace = GetSingleFace(segment.GetLowerShape(WING_COORDINATE_SYSTEM));
        TopoDS_Face upperSegmentFace = GetSingleFace(segment.GetUpperShape(WING_COORDINATE_SYSTEM));
        upperShellSewing.Add(upperSegmentFace);
        lowerShellSewing.Add(lowerSegmentFace);
        sewing.Add(lowerSegmentFace);
        sewing.Add(upperSegmentFace);
    }

    TopoDS_Shape outerShape = segments.back()->GetOuterClosure(WING_COORDINATE_SYSTEM);
    outerFace = GetSingleFace(outerShape);
    sewing.Add(outerFace);

    sewing.Perform();
    TopoDS_Shape loftShape = sewing.SewedShape();
    // convert loft to solid for correct normals
    if (loftShape.ShapeType() == TopAbs_SHELL) {
        BRepBuilderAPI_MakeSolid ms;
        ms.Add(TopoDS::Shell(loftShape));
        if (!ms.IsDone()) {
            throw CTiglError("Error building WingComponentSegment shape: generation of solid failed!");
        }
        loftShape = ms.Solid();
    }
    else if (loftShape.ShapeType() != TopAbs_SOLID) {
        throw CTiglError("Error building WingComponentSegment shape: result of sewing is no shell or solid!");
    }
    upperShellSewing.Perform();
    lowerShellSewing.Perform();
    upperShape = upperShellSewing.SewedShape();
    lowerShape = lowerShellSewing.SewedShape();

    BRepTools::Clean(loftShape);

    Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
    sfs->Init ( loftShape );
    sfs->Perform();
    loftShape = sfs->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(loftShape, System);
    myVolume = System.Mass();

    // Calculate surface area
    GProp_GProps AreaSystem;
    BRepGProp::SurfaceProperties(loftShape, AreaSystem);
    mySurfaceArea = AreaSystem.Mass();
        
    // Set Names
    std::string loftName = m_uID;
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft (new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft, static_cast<unsigned int>(segments.size()));
    return loft;
}

// Method for building wires for eta-, leading edge-, trailing edge-lines
void CCPACSWingComponentSegment::BuildLines() const
{
    // search for ETA coordinate
    std::vector<gp_Pnt> lePointContainer;
    std::vector<gp_Pnt> tePointContainer;
    gp_Pnt extendedInnerLePoint;
    gp_Pnt extendedOuterLePoint;
    gp_Pnt extendedInnerTePoint;
    gp_Pnt extendedOuterTePoint;

    gp_Pnt pnt;

    const SegmentList& segments = GetSegmentList();
    SegmentList::const_iterator it;
    // get the leading and trailing edge points of all sections
    for (it = segments.begin(); it != segments.end(); ++it) {
        const CCPACSWingSegment& segment = *(*it);

            // get leading edge point
        pnt = segment.GetPoint(0, 0, true, WING_COORDINATE_SYSTEM);
            lePointContainer.push_back(pnt);
            // get trailing edge point
        pnt = segment.GetPoint(0, 1, true, WING_COORDINATE_SYSTEM);
            tePointContainer.push_back(pnt);
    }
    // finally add the points for the outer section
                // get leading edge point
    pnt = segments.back()->GetPoint(1, 0, true, WING_COORDINATE_SYSTEM);
                lePointContainer.push_back(pnt);
                // get trailing edge point
    pnt = segments.back()->GetPoint(1, 1, true, WING_COORDINATE_SYSTEM);
                tePointContainer.push_back(pnt);

    // determine extended leading/trailing edge points
    // scale leading or trailing edge to get both points in the section planes of the
    // inner and outer sections
    // see CPACS documentation for "componentSegment" element
    extendedInnerLePoint = lePointContainer.at(0);
    extendedInnerTePoint = tePointContainer.at(0);
    extendedOuterLePoint = lePointContainer.at(lePointContainer.size() - 1);
    extendedOuterTePoint = tePointContainer.at(tePointContainer.size() - 1);
    std::string innerSegmentUID = GetInnerSegmentUID();
    std::string outerSegmentUID = GetOuterSegmentUID();
    gp_Vec innerLeDirYZ = GetLeadingEdgeDirection(innerSegmentUID);
    innerLeDirYZ.SetX(0);
    innerLeDirYZ.Normalize();
    gp_Vec outerLeDirYZ = GetLeadingEdgeDirection(outerSegmentUID);
    outerLeDirYZ.SetX(0);
    outerLeDirYZ.Normalize();
    gp_Vec innerChordVec(extendedInnerTePoint, extendedInnerLePoint);

    // compute length of chord line vector projected to eta line vector
    double lp = innerChordVec.Dot(innerLeDirYZ);
    // check if projection of chord line vector points in direction or in opposite direction of eta line vector
    if (lp > Precision::Confusion()) {
        // scale leading edge
        gp_Vec innerLeDir = GetLeadingEdgeDirection(innerSegmentUID);
        // compute cosine of angle between leading edge vector and eta line vector
        double cosPhi = innerLeDir.Dot(innerLeDirYZ);
        // compute the length value for extending the leading edge
        double length = lp / cosPhi;
        extendedInnerLePoint.Translate(-1.0 * innerLeDir * length);
    }
    else if (lp < -Precision::Confusion()) {
        // scale trailing edge
        gp_Vec innerTeDir = GetTrailingEdgeDirection(innerSegmentUID);
        // compute cosine of angle between trailing edge vector and eta line vector
        double cosPhi = innerTeDir.Dot(innerLeDirYZ);
        // compute the length value for extending the trailing edge
        double length = -1.0 * (lp / cosPhi);
        extendedInnerTePoint.Translate(-1.0 * innerTeDir * length);
    }
    gp_Vec outerChordVec(extendedOuterTePoint, extendedOuterLePoint);
    // compute length of chord line vector projected to eta line vector
    lp = outerChordVec.Dot(outerLeDirYZ);
    // check if projection of chord line vector points in direction or in opposite direction of eta line vector
    if (lp > Precision::Confusion()) {
        // scale trailing edge
        gp_Vec outerTeDir = GetTrailingEdgeDirection(outerSegmentUID);
        // compute cosine of angle between trailing edge vector and eta line vector
        double cosPhi = outerTeDir.Dot(outerLeDirYZ);
        // compute the length value for extending the trailing edge
        double length = lp / cosPhi;
        extendedOuterTePoint.Translate(outerTeDir * length);
    }
    else if (lp < -Precision::Confusion()) {
        // scale leading edge
        gp_Vec outerLeDir = GetLeadingEdgeDirection(outerSegmentUID);
        // compute cosine of angle between leading edge vector and eta line vector
        double cosPhi = outerLeDir.Dot(outerLeDirYZ);
        // compute the length value for extending the leading edge
        double length = -1.0 * (lp / cosPhi);
        extendedOuterLePoint.Translate(outerLeDir * length);
    }

#ifdef _DEBUG
    innerChordVec = gp_Vec(extendedInnerTePoint, extendedInnerLePoint);
    outerChordVec = gp_Vec(extendedOuterTePoint, extendedOuterLePoint);
    lp = innerChordVec.Dot(innerLeDirYZ);
    assert(fabs(lp) < Precision::Confusion());
    lp = outerChordVec.Dot(outerLeDirYZ);
    assert(fabs(lp) < Precision::Confusion());
#endif

    // build wires: etaLine, extendedEtaLine, leadingEdgeLine, extendedLeadingEdgeLine,
    //              trailingEdgeLine, extendedTrailingEdgeLine
    BRepBuilderAPI_MakeWire wbEta, wbExtEta, wbLe, wbExtLe, wbTe, wbExtTe;
    gp_Pnt innerLePoint, outerLePoint, innerPoint2d, outerPoint2d, innerTePoint, outerTePoint;
    TopoDS_Edge leEdge, etaEdge, teEdge, extLeEdge, extEtaEdge, extTeEdge;
    int numberOfSections = static_cast<int>(segments.size()) + 1;
    for (int i = 1; i < numberOfSections; i++) {
        innerLePoint = lePointContainer[i-1];
        outerLePoint = lePointContainer[i];
        innerPoint2d = gp_Pnt(0, innerLePoint.Y(), innerLePoint.Z());
        outerPoint2d = gp_Pnt(0, outerLePoint.Y(), outerLePoint.Z());
        innerTePoint = tePointContainer[i-1];
        outerTePoint = tePointContainer[i];
        leEdge = BRepBuilderAPI_MakeEdge(innerLePoint, outerLePoint);
        etaEdge = BRepBuilderAPI_MakeEdge(innerPoint2d, outerPoint2d);
        teEdge = BRepBuilderAPI_MakeEdge(innerTePoint, outerTePoint);
        if (i == 1) {
            innerLePoint = extendedInnerLePoint;
            innerPoint2d = gp_Pnt(0, innerLePoint.Y(), innerLePoint.Z());
            innerTePoint = extendedInnerTePoint;
        } 
        if (i == numberOfSections - 1) {
            outerLePoint = extendedOuterLePoint;
            outerPoint2d = gp_Pnt(0, outerLePoint.Y(), outerLePoint.Z());
            outerTePoint = extendedOuterTePoint;
        }
        extLeEdge = BRepBuilderAPI_MakeEdge(innerLePoint, outerLePoint);
        extEtaEdge = BRepBuilderAPI_MakeEdge(innerPoint2d, outerPoint2d);
        extTeEdge = BRepBuilderAPI_MakeEdge(innerTePoint, outerTePoint);
        wbLe.Add(leEdge);
        wbEta.Add(etaEdge);
        wbTe.Add(teEdge);
        wbExtLe.Add(extLeEdge);
        wbExtEta.Add(extEtaEdge);
        wbExtTe.Add(extTeEdge);
    }
    leadingEdgeLine = wbLe.Wire();
    etaLine = wbEta.Wire();
    trailingEdgeLine = wbTe.Wire();
    extendedLeadingEdgeLine = wbExtLe.Wire();
    extendedEtaLine = wbExtEta.Wire();
    extendedTrailingEdgeLine = wbExtTe.Wire();

    linesAreValid = true;
}

void CCPACSWingComponentSegment::UpdateProjectedLeadingEdge() const
{
    if ( !projLeadingEdge.IsNull() ) {
        return;
    }

    // add inner sections of each segment
    const SegmentList& segments = GetSegmentList();

    if (segments.size() < 1) {
        std::stringstream str;
        str << "Wing component " << m_uID << " does not contain any segments (CCPACSWingComponentSegment::updateProjectedLeadingEdge)!";
        throw CTiglError(str.str(), TIGL_ERROR);
    }

    // create projection plane
    gp_GTrsf wingTrafo = wing->GetTransformationMatrix().Get_gp_GTrsf();
    gp_XYZ pCenter(0,0,0);
    gp_XYZ pDirX(1,0,0);
    wingTrafo.Transforms(pCenter);
    wingTrafo.Transforms(pDirX);
    Handle(Geom_Plane) projPlane = new Geom_Plane(pCenter, pDirX-pCenter);
    
    std::vector<gp_Pnt> LEPointsProjected;
    SegmentList::const_iterator segmentIt = segments.begin();
    int pointIndex = 1;
    for (; segmentIt != segments.end(); ++segmentIt) {
        tigl::CCPACSWingSegment& segment = **segmentIt;

        // build iso xsi line
        gp_Pnt lep = segment.GetChordPoint(0.,0.);

        // build leading edge projected to the plane
        gp_Pnt lep_proj = GeomAPI_ProjectPointOnSurf(lep, projPlane).NearestPoint();
        LEPointsProjected.push_back(lep_proj);

        if (segmentIt == segments.end()-1) {
            // add outer section of last segment
            gp_Pnt lep = segment.GetChordPoint(1., 0.);
            gp_Pnt lep_proj = GeomAPI_ProjectPointOnSurf(lep, projPlane).NearestPoint();
            LEPointsProjected.push_back(lep_proj);
            // the break should not be necessary here, since the loop is
            break;
        }
        pointIndex++;
    }

    // check if we have to extend the leading edge at wing tip
    std::size_t nPoints = LEPointsProjected.size();
    tigl::CCPACSWingSegment& outerSegment = *segments[segments.size()-1];
    tigl::CCPACSWingSegment& innerSegment = *segments[0];

    // project outer point of trailing edge on leading edge
    gp_Pnt pOuterTrailingEdge = outerSegment.GetChordPoint(1.0, 1.0);
    Standard_Real uout = ProjectPointOnLine(pOuterTrailingEdge, LEPointsProjected[nPoints-2], LEPointsProjected[nPoints-1]);


    // project outer point of trailing edge on leading edge
    gp_Pnt pInnerTrailingEdge = innerSegment.GetChordPoint(0.0, 1.0);
    Standard_Real uin = ProjectPointOnLine(pInnerTrailingEdge, LEPointsProjected[0], LEPointsProjected[1]);

    gp_Pnt outnew =  LEPointsProjected[nPoints-1];
    if (uout > 1.0) {
        // extend outer leading edge
        outnew = LEPointsProjected[nPoints-2].XYZ()*(1. - uout) + LEPointsProjected[nPoints-1].XYZ()*uout;
    }

    gp_Pnt innew  = LEPointsProjected[0];
    if (uin < 0.0) {
        // extend inner leading edge
        innew  = LEPointsProjected[0].XYZ()*(1. - uin) + LEPointsProjected[1].XYZ()*uin;
    }

    // set new leading edge points
    LEPointsProjected[nPoints-1] = outnew;
    LEPointsProjected[0]         = innew;

    // build projected leading edge curve
    projLeadingEdge = CPointsToLinearBSpline(LEPointsProjected).Curve();
}

void CCPACSWingComponentSegment::UpdateExtendedChordFaces()
{
    if (surfacesAreValid) {
        return;
    }

    UpdateProjectedLeadingEdge();

    const SegmentList& segments = GetSegmentList();

    // outer segment
    // compute eta point and normal on the projected LE
    gp_Pnt etaPnt; gp_Vec etaNormal; gp_Pln plane;
    projLeadingEdge->D1(1.0, etaPnt, etaNormal);
    plane = gp_Pln(etaPnt, etaNormal.XYZ());

    CCPACSWingSegment* outerSegment = segments.back();
    gp_Pnt pLEOuter = outerSegment->GetChordPoint(1., 0.);
    gp_Pnt pLEInner = outerSegment->GetChordPoint(0., 0.);
    gp_Pnt pTEOuter = outerSegment->GetChordPoint(1., 1.);
    gp_Pnt pTEInner = outerSegment->GetChordPoint(0., 1.);

    gp_Pnt pLEOuterExt, pTEOuterExt;
    if (IntersectLinePlane(pLEInner, pLEOuter, plane, pLEOuterExt) == NoIntersection) {
        throw CTiglError("Leading edge of last wing segment must no go in x direction!");
    }
    pLEOuter = pLEOuterExt;

    if (IntersectLinePlane(pTEInner, pTEOuter, plane, pTEOuterExt) == NoIntersection) {
        throw CTiglError("Trailing edge of last wing segment must no go in x direction!");
    }
    pTEOuter = pTEOuterExt;

    extendedOuterChord.setQuadriangle(pLEInner.XYZ(), pLEOuter.XYZ(), pTEInner.XYZ(), pTEOuter.XYZ());

    // Inner segment
    projLeadingEdge->D1(0.0, etaPnt, etaNormal);
    plane = gp_Pln(etaPnt, etaNormal.XYZ());

    CCPACSWingSegment* innerSegment = segments.front();
    pLEOuter = innerSegment->GetChordPoint(1., 0.);
    pLEInner = innerSegment->GetChordPoint(0., 0.);
    pTEOuter = innerSegment->GetChordPoint(1., 1.);
    pTEInner = innerSegment->GetChordPoint(0., 1.);

    gp_Pnt pLEInnerExt, pTEInnerExt;
    if (IntersectLinePlane(pLEOuter, pLEInner, plane, pLEInnerExt) == NoIntersection) {
        throw CTiglError("Leading edge of first wing segment must no go in x direction!");
    }
    pLEInner = pLEInnerExt;

    if (IntersectLinePlane(pTEOuter, pTEInner, plane, pTEInnerExt) == NoIntersection) {
        throw CTiglError("Leading edge of first wing segment must no go in x direction!");
    }
    pTEInner = pTEInnerExt;

    extendedInnerChord.setQuadriangle(pLEInner.XYZ(), pLEOuter.XYZ(), pTEInner.XYZ(), pTEOuter.XYZ());


    surfacesAreValid = true;
}

void CCPACSWingComponentSegment::UpdateChordFace() const
{
    // update creation of segment list
    GetSegmentList();
    chordFace.BuildChordSurface();
}


// Gets a point in relative wing coordinates for a given eta and xsi
gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi) const
{
    // search for ETA coordinate
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }

    UpdateProjectedLeadingEdge();

    const SegmentList& segments = GetSegmentList();
    if (segments.size() < 1) {
        throw CTiglError("Wing component " + m_uID + " does not contain any segments (CCPACSWingComponentSegment::GetPoint)!", TIGL_ERROR);
    }

    // build up iso xsi line control points
    TColgp_Array1OfPnt xsiPoints(1,(Standard_Integer) segments.size() + 1);
    SegmentList::const_iterator segmentIt = segments.begin();
    int pointIndex = 1;
    for (; segmentIt != segments.end(); ++segmentIt) {
        tigl::CCPACSWingSegment& segment = **segmentIt;

        // build iso xsi line
        gp_Pnt p = segment.GetChordPoint(0,xsi);
        xsiPoints.SetValue(pointIndex, p);
        if (segmentIt == segments.end()-1) {
            // add outer section of last segment
            gp_Pnt p = segment.GetChordPoint(1., xsi);
            xsiPoints.SetValue(pointIndex+1, p);
            // the break should not be necessary here, since the loop is
            break;
        }
        pointIndex++;
    }

    // compute eta point and normal on the projected LE
    gp_Pnt etaPnt; gp_Vec etaNormal;
    projLeadingEdge->D1(eta, etaPnt, etaNormal);

    // compute intersection with line strips
    std::vector<gp_Pnt> intersPoints;
    for (unsigned int i = 1; i <= segments.size(); ++i) {
        gp_Pnt p1 = xsiPoints.Value(i);
        gp_Pnt p2 = xsiPoints.Value(i+1);
        gp_Pnt intersectionPoint;

        IntStatus status = IntersectLinePlane(p1, p2, gp_Pln(etaPnt, etaNormal.XYZ()), intersectionPoint);

        if (status == NoIntersection) {
            continue;
        }

        if ((i == 1 && status == OutsideBefore) || (status == BetweenPoints) || (i == segments.size() && status == OutsideAfter)) {
            intersPoints.push_back(intersectionPoint);
        }
    }

    if (intersPoints.size() == 1) {
        return intersPoints[0];
    }
    else if (intersPoints.size() > 1) {
        // chose the intersection point that has minimal distance to the etaPnt
        // first, we have to project the point on the plane, to ignore any depth distance

        // compute the projection plane as done in CCPACSWingComponentSegment::UpdateProjectedLeadingEdge()
        gp_GTrsf wingTrafo = wing->GetTransformationMatrix().Get_gp_GTrsf();
        gp_XYZ pCenter(0,0,0);
        gp_XYZ pDirX(1,0,0);
        wingTrafo.Transforms(pCenter);
        wingTrafo.Transforms(pDirX);
        Handle(Geom_Plane) projPlane = new Geom_Plane(pCenter, pDirX-pCenter);

        double minDist = FLT_MAX;
        gp_Pnt intersectionPoint;
        for (std::vector<gp_Pnt>::iterator it = intersPoints.begin(); it != intersPoints.end(); ++it) {
            // project to wing plane
            gp_Pnt pInterProj = GeomAPI_ProjectPointOnSurf(*it, projPlane).NearestPoint();

            double dist = pInterProj.Distance(etaPnt);
            if (dist < minDist) {
                minDist = dist;
                intersectionPoint = *it;
            }
        }

        return intersectionPoint;
    }
    else {
        throw CTiglError("Can not compute point in CCPACSWingComponentSegment::GetPoint.", TIGL_MATH_ERROR);
    }
}

void CCPACSWingComponentSegment::GetMidplaneEtaXsi(const gp_Pnt& p, double& eta, double& xsi) const
{
    // @TODO: replace by using the chordface
    gp_Pnt globalPoint = wing->GetWingTransformation().Transform(p);
    gp_Pnt dummy;
    double deviation = 0.;
    
    const CCPACSWingSegment* segment = static_cast<const CCPACSWingSegment*>(
        findSegment(globalPoint.X(), globalPoint.Y(), globalPoint.Z(), dummy, deviation)
    );

    if (!segment) {
        throw CTiglError("Error in GetMidplaneEtaXsi: passed point is not part of the component segment");
    }

    double segmentEta, segmentXsi;
    segment->GetEtaXsi(globalPoint, segmentEta, segmentXsi);

    const double precision = 1E-5;
    // check if valid eta/xsi values are returned, otherwise the point may lie outside of component segment
    if (segmentEta < (0 - precision) || segmentEta > (1 + precision) || 
        segmentXsi < (0 - precision) || segmentXsi > (1 + precision)) {
        throw CTiglError("Error in GetMidplaneEtaXsi: passed point is not part of the component segment");
    }
    if (segmentEta < 0) {
        segmentEta = 0;
    }
    else if (segmentEta > 1) {
        segmentEta = 1;
    }
    if (segmentXsi < 0) {
        segmentXsi = 0;
    }
    else if (segmentXsi > 1) {
        segmentXsi = 1;
    }
 
    GetEtaXsiFromSegmentEtaXsi(segment->GetUID(), segmentEta, segmentXsi, eta, xsi);
}

// Getter for eta direction of midplane
gp_Vec CCPACSWingComponentSegment::GetMidplaneEtaDir(double eta) const
{
    // get point on eta line
    gp_Pnt etaPnt;
    gp_Vec etaDir;

    if (!linesAreValid) {
        BuildLines();
    }

    BRepAdaptor_CompCurve extendedEtaLineCurve(extendedEtaLine, Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length( extendedEtaLineCurve );
    extendedEtaLineCurve.D1( len * eta, etaPnt, etaDir );
    return etaDir.Normalized();
}

// Getter for midplane normal vector
gp_Vec CCPACSWingComponentSegment::GetMidplaneNormal(double eta) const
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetMidplaneOrChordlinePoint", TIGL_ERROR);
    }

    gp_Pnt lePnt = GetMidplaneOrChordlinePoint(eta, 0);
    gp_Pnt tePnt = GetMidplaneOrChordlinePoint(eta, 1);
    gp_Vec etaDir = GetMidplaneEtaDir(eta);
    gp_Vec chordLine(lePnt, tePnt);
    gp_Vec normal = chordLine.Normalized().Crossed(etaDir);
    return normal;
}

void CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi(const std::string& segmentUID, double seta, double sxsi, double& eta, double& xsi) const
{
    // search for ETA coordinate
        
    if (seta < 0.0 || seta > 1.0) {
        throw CTiglError("Parameter seta not in the range 0.0 <= seta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
    if (sxsi < 0.0 || sxsi > 1.0) {
        throw CTiglError("Parameter sxsi not in the range 0.0 <= sxsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }

    const SegmentList& segments = GetSegmentList();
    // check that segment belongs to component segment
    CCPACSWingSegment* segment = NULL;
    for (SegmentList::const_iterator it = segments.begin(); it != segments.end(); ++it) {
        if (segmentUID == (*it)->GetUID()) {
            segment = *it;
            break;
        }
    }
    if (!segment) {
        throw CTiglError("segment does not belong to component segment in CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi", TIGL_ERROR);
    }

    gp_Pnt point3d = segment->GetChordPoint(seta, sxsi);
    xsi = sxsi;

    UpdateProjectedLeadingEdge();
    GeomAPI_ProjectPointOnCurve proj(point3d, projLeadingEdge);
    if (proj.NbPoints() > 0) {
        eta = proj.LowerDistanceParameter();
    }
    else {
        throw CTiglError("Cannot compute eta value in CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi", TIGL_ERROR);
    }
}


// Returns the volume of this segment
double CCPACSWingComponentSegment::GetVolume()
{
    GetLoft();
    return( myVolume );
}

// Returns the surface area of this segment
double CCPACSWingComponentSegment::GetSurfaceArea()
{
    GetLoft();
    return( mySurfaceArea );
}

//    // Returns an upper or lower point on the segment surface in
//    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
//    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
//    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
//    // edge on the outer wing profile. If fromUpper is true, a point
//    // on the upper surface is returned, otherwise from the lower.
//    gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi, bool fromUpper)
//    {
//        if (eta < 0.0 || eta > 1.0)
//        {
//            throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
//        }
//
//        CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
//        CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
//
//        // Compute points on wing profiles for the given xsi
//        gp_Pnt innerProfilePoint;
//        gp_Pnt outerProfilePoint;
//        if (fromUpper == true)
//        {
//            innerProfilePoint = innerProfile.GetUpperPoint(xsi);
//            outerProfilePoint = outerProfile.GetUpperPoint(xsi);
//        }
//        else
//        {
//            innerProfilePoint = innerProfile.GetLowerPoint(xsi);
//            outerProfilePoint = outerProfile.GetLowerPoint(xsi);
//        }
//
//        // Do section element transformation on points
//        innerProfilePoint = innerConnection.GetSectionElementTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetSectionElementTransformation().Transform(outerProfilePoint);
//
//        // Do section transformations
//        innerProfilePoint = innerConnection.GetSectionTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetSectionTransformation().Transform(outerProfilePoint);
//
//        // Do positioning transformations
//        innerProfilePoint = innerConnection.GetPositioningTransformation().Transform(innerProfilePoint);
//        outerProfilePoint = outerConnection.GetPositioningTransformation().Transform(outerProfilePoint);
//
//        // Get point on wing segment in dependence of eta by linear interpolation
//        Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(innerProfilePoint, outerProfilePoint);
//        Standard_Real firstParam = profileLine->FirstParameter();
//        Standard_Real lastParam  = profileLine->LastParameter();
//        Standard_Real param = (lastParam - firstParam) * eta;
//        gp_Pnt profilePoint;
//        profileLine->D0(param, profilePoint);
//
//        return profilePoint;
//    }
//

// Returns the segment to a given point on the componentSegment. 
// Returns null if the point is not an that wing!
const CCPACSWingSegment* CCPACSWingComponentSegment::findSegment(double x, double y, double z, gp_Pnt& nearestPoint, double& deviation) const
{
    CCPACSWingSegment* result = NULL;
    gp_Pnt pnt(x, y, z);


    const SegmentList& segments = GetSegmentList();

    double minDist = std::numeric_limits<double>::max();
    // now discover to which segment the point belongs
    for (SegmentList::const_iterator segit = segments.begin(); segit != segments.end(); ++segit) {
        try {
            double eta, xsi;
            (*segit)->GetEtaXsi(pnt, eta, xsi);
            gp_Pnt pointProjected = (*segit)->GetChordPoint(eta, xsi);

            // Get nearest point on this segment
            double nextEta = GetNearestValidParameter(eta);
            double nextXsi = GetNearestValidParameter(xsi);
            gp_Pnt currentPoint = (*segit)->GetChordPoint(nextEta, nextXsi);

            double currentDist = currentPoint.Distance(pointProjected);
            if (currentDist < minDist) {
                minDist = currentDist;
                nearestPoint = currentPoint;
                result = *segit;
            }
        }
        catch (...) {
            // do nothing
        }
    }

    deviation = minDist;

    return result;
}

MaterialList CCPACSWingComponentSegment::GetMaterials(double eta, double xsi, TiglStructureType type)
{
    MaterialList list;
        
    if (!m_structure) {
        // return empty list
        return list;
    }
        
    if (type != UPPER_SHELL && type != LOWER_SHELL) {
        LOG(ERROR) << "Cannot compute materials for inner structure in CCPACSWingComponentSegment::GetMaterials (not yet implemented)";
        return list;
    }
    else {
        CCPACSWingShell* shell = (type == UPPER_SHELL? &m_structure->GetUpperShell() : &m_structure->GetLowerShell());
        int ncells = shell->GetCellCount();
        for (int i = 1; i <= ncells; ++i){
            CCPACSWingCell& cell = shell->GetCell(i);
            if (!cell.GetMaterial().IsValid()) {
                continue;
            }

            if (cell.IsInside(eta,xsi)) {
                list.push_back(&(cell.GetMaterial()));
            }
        }

        // add complete skin, only if no cells are defined
        if (list.empty() && shell->GetMaterial().IsValid()){
            list.push_back(&(shell->GetMaterial()));
        }
        
    }
    return list;
}

// Method for checking whether segment is contained in componentSegment
bool CCPACSWingComponentSegment::IsSegmentContained(const CCPACSWingSegment& segment) const
{
    bool isSegmentContained = false;
    std::string nextElementUID = m_fromElementUID;
    int segmentCount = wing->GetSegmentCount();

    for (int i=1; i <= segmentCount; i++) {
        CCPACSWingSegment& testSegment = (CCPACSWingSegment&) wing->GetSegment(i);
        std::string innerSectionElementUID = testSegment.GetInnerSectionElementUID();
        // skip segments with not matching inner element UID
        if (innerSectionElementUID != nextElementUID) {
            continue;
        }
        if (innerSectionElementUID == m_toElementUID) {
            break;
        }
        
        if (innerSectionElementUID == segment.GetInnerSectionElementUID()) {
            isSegmentContained = true;
            break;
        }
        nextElementUID = testSegment.GetOuterSectionElementUID();
    }

    return isSegmentContained;
}

const CCPACSWingShell& CCPACSWingComponentSegment::GetUpperShell() const
{
    if (!m_structure) {
        throw CTiglError("no structure existing in CCPACSWingComponentSegment::GetUpperShell!");
    }
    return m_structure->GetUpperShell();
}

CCPACSWingShell& CCPACSWingComponentSegment::GetUpperShell()
{
    // forward call to const method
    return const_cast<CCPACSWingShell&>(static_cast<const CCPACSWingComponentSegment&>(*this).GetUpperShell());
}

const CCPACSWingShell& CCPACSWingComponentSegment::GetLowerShell() const
{
    if (!m_structure) {
        throw CTiglError("no structure existing in CCPACSWingComponentSegment::GetLowerShell!");
    }
    return m_structure->GetLowerShell();
}

CCPACSWingShell& CCPACSWingComponentSegment::GetLowerShell()
{
    // forward call to const method
    return const_cast<CCPACSWingShell&>(static_cast<const CCPACSWingComponentSegment&>(*this).GetLowerShell());
}

TopoDS_Shape CCPACSWingComponentSegment::GetMidplaneShape() const
{
    BRep_Builder builder;
    TopoDS_Shell midplaneShape;
    builder.MakeShell(midplaneShape);

    gp_Pnt innerLePnt, innerTePnt, outerLePnt, outerTePnt;
    const SegmentList& segments = GetSegmentList();
    innerLePnt = segments.front()->GetChordPoint(0, 0);
    innerTePnt = segments.front()->GetChordPoint(0, 1);
    for (SegmentList::const_iterator it = segments.begin(); it != segments.end(); ++it) {
        const CCPACSWingSegment& segment = *(*it);
        outerLePnt = segment.GetChordPoint(1, 0);
        outerTePnt = segment.GetChordPoint(1, 1);
        TopoDS_Face face = BuildFace(innerLePnt, innerTePnt, outerLePnt, outerTePnt);
        builder.Add(midplaneShape, face);
        innerLePnt = outerLePnt;
        innerTePnt = outerTePnt;
        }
    return midplaneShape;
}



} // end namespace tigl
