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
#include "CTiglMakeLoft.h"
#include "CPointsToLinearBSpline.h"
#include "tiglcommonfunctions.h"
#include "CCPACSWingCSStructure.h"
#include "CNamedShape.h"
#include "CTiglWingChordface.h"
#include "CTiglShapeGeomComponentAdaptor.h"
#include "CNamedShape.h"
#include "Debugging.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Wire.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "GeomAPI_ExtremaCurveCurve.hxx"
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
    , CTiglAbstractSegment<CCPACSWingComponentSegment>(parent->GetComponentSegments(), parent->GetParent())
    , wing(parent->GetParent())
    , wingSegments(*this, &CCPACSWingComponentSegment::BuildWingSegments)
    , geomCache(*this, &CCPACSWingComponentSegment::BuildGeometry)
    , linesCache(*this, &CCPACSWingComponentSegment::BuildLines)
    , upperShape(tigl::make_unique<ShapeAdaptor>(this, &CCPACSWingComponentSegment::GetUpperShape, m_uidMgr))
    , lowerShape(tigl::make_unique<ShapeAdaptor>(this, &CCPACSWingComponentSegment::GetLowerShape, m_uidMgr))
    , chordFace(make_unique<CTiglWingChordface>(*this, uidMgr))
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
    CTiglAbstractSegment<CCPACSWingComponentSegment>::Reset();
    wingSegments.clear();
    if (m_structure) {
        m_structure->Invalidate();
    }
    geomCache.clear();
    linesCache.clear();
    chordFace->Reset();
    upperShape->Reset();
    lowerShape->Reset();
}

// Cleanup routine
void CCPACSWingComponentSegment::Cleanup()
{
    m_name = "";
    m_fromElementUID = "";
    m_toElementUID   = "";
    geomCache.clear();
    linesCache.clear();
    CTiglAbstractSegment<CCPACSWingComponentSegment>::Reset();
    wingSegments.clear();
}

// Update internal segment data
void CCPACSWingComponentSegment::Update()
{
    Invalidate();

    chordFace->SetUID(GetDefaultedUID() + "_chordface");
    lowerShape->SetUID(GetDefaultedUID() + "_lower");
    upperShape->SetUID(GetDefaultedUID() + "_upper");
}

// Read CPACS segment elements
void CCPACSWingComponentSegment::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& segmentXPath)
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
PNamedShape CCPACSWingComponentSegment::GetUpperShape() const
{
    return geomCache->upperShape;
}

// Getter for lower Shape
PNamedShape CCPACSWingComponentSegment::GetLowerShape() const
{
    return geomCache->lowerShape;
}

// Getter for inner segment face
TopoDS_Face CCPACSWingComponentSegment::GetInnerFace() const
{
    return geomCache->innerFace;
}

// Getter for outer segment face
TopoDS_Face CCPACSWingComponentSegment::GetOuterFace() const
{
    return geomCache->outerFace;
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
    gp_Pnt globalPnt = wing->GetTransformationMatrix().Transform(point);

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
    gp_Pnt globalPnt = wing->GetTransformationMatrix().Transform(point);

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
TopoDS_Wire CCPACSWingComponentSegment::GetMidplaneLine(const gp_Pnt& localStartPoint, const gp_Pnt& localEndPoint) const
{
    // TODO: this method creates a straight line between the start- and end-point within the single
    // segments, while the real line could be a curve (depending on the twist and the position of
    // the points)

    // determine start and end point
    const gp_Pnt globalStartPnt = wing->GetTransformationMatrix().Transform(localStartPoint);
    const gp_Pnt globalEndPnt   = wing->GetTransformationMatrix().Transform(localEndPoint);

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

    TopoDS_Shape localLoft = GetWing().GetTransformationMatrix().Inverted().Transform(GetLoft()->Shape());

    // get minimum and maximum z-value of bounding box
    Bnd_Box bbox;
    BRepBndLib::Add(localLoft, bbox);
    const double zmin = bbox.CornerMin().Z();
    const double zmax = bbox.CornerMax().Z();

    // build cut face
    gp_Pnt p0 = localStartPoint;
    gp_Pnt p1 = localStartPoint;
    gp_Pnt p2 = localEndPoint;
    gp_Pnt p3 = localEndPoint;
    p0.SetZ(zmin);
    p1.SetZ(zmax);
    p2.SetZ(zmin);
    p3.SetZ(zmax);
    TopoDS_Face cutFace = BuildFace(p0, p1, p2, p3);

    // compute start and end point from intersection with inner/outer section in
    // case the points lie outside of the segments
    gp_Pnt startPnt       = localStartPoint;
    gp_Pnt endPnt         = localEndPoint;
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
            if (!prevPnt.IsEqual(endPnt, Precision::Confusion())) {
                TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(prevPnt, endPnt);
                wireBuilder.Add(edge);
            }
            break;
        }
    }
    return wireBuilder.Wire();
}

// Getter for the leading edge line
const TopoDS_Wire& CCPACSWingComponentSegment::GetLeadingEdgeLine() const
{
    return linesCache->leadingEdgeLine;
}

// Getter for the trailing edge line
const TopoDS_Wire& CCPACSWingComponentSegment::GetTrailingEdgeLine() const
{
    return linesCache->trailingEdgeLine;
}

const std::string& CCPACSWingComponentSegment::GetInnerSegmentUID() const
{
    return GetSegmentList().front()->GetUID();
}

const std::string& CCPACSWingComponentSegment::GetOuterSegmentUID() const
{
    return GetSegmentList().back()->GetUID();
}

const SegmentList& CCPACSWingComponentSegment::GetSegmentList() const
{
    return *wingSegments;
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
    if (eta > 1 + Precision::Confusion() || eta < - Precision::Confusion()) {
        throw CTiglError("Eta not in range [0,1] in CCPACSWingComponentSegment::InterpolateOnLine.", TIGL_MATH_ERROR);
    }

    // compute component segment line
    gp_Pnt p1 = GetPoint(csEta1, csXsi1);
    gp_Pnt p2 = GetPoint(csEta2, csXsi2);

    const Handle(Geom_Curve) etaCurve = chordFace->GetSurface()->VIso(eta);
    const Handle(Geom_Curve) line = new Geom_Line(p1, p2.XYZ() - p1.XYZ());

    GeomAPI_ExtremaCurveCurve algo(etaCurve, line, 0., 1., 0., p1.Distance(p2));
    if (algo.NbExtrema() < 1) {
        throw CTiglError("Cannot compute xsi coordinate in CCPACSWingComponentSegment::InterpolateOnLine", TIGL_MATH_ERROR);
    }

    // xsi coordinate
    double u;
    algo.LowerDistanceParameters(xsi, u);

    // TODO (siggel): check if point on line is outside the wing

    // compute the error distance
    // This is the distance from the line to the nearest point on the chord face
    errorDistance = algo.LowerDistance();
}

const CTiglWingChordface &CCPACSWingComponentSegment::GetChordface() const
{
    return *chordFace;
}

// get short name for loft
std::string CCPACSWingComponentSegment::GetShortShapeName() const
{
    unsigned int windex = 0;
    unsigned int wcsindex = 0;
    for (int i = 1; i <= wing->GetConfiguration().GetWingCount(); ++i) {
        const CCPACSWing& w = wing->GetConfiguration().GetWing(i);
        if (wing->GetUID() == w.GetUID()) {
            windex = i;
            for (int j = 1; j <= w.GetComponentSegmentCount(); j++) {
                const CCPACSWingComponentSegment& wcs = w.GetComponentSegment(j);
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
PNamedShape CCPACSWingComponentSegment::BuildLoft() const
{
    // Set Names
    std::string loftName = m_uID;
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft (new CNamedShape(geomCache->loftShape, loftName.c_str(), loftShortName));
    SetFaceTraits(loft, static_cast<unsigned int>(wingSegments->size()));
    return loft;
}

void CCPACSWingComponentSegment::BuildWingSegments(SegmentList& cache) const
{
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
        cache.push_back(pSeg);
    }
}

void CCPACSWingComponentSegment::BuildGeometry(GeometryCache& cache) const
{
    DEBUG_SCOPE(debug);

    // use sewing for full loft
    BRepBuilderAPI_Sewing sewing;
    BRepBuilderAPI_Sewing upperShellSewing;
    BRepBuilderAPI_Sewing lowerShellSewing;

    const SegmentList& segments = GetSegmentList();
    if (segments.size() == 0) {
        throw CTiglError("Could not find segments in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
    }

    int i = 0;
    for (SegmentList::const_iterator it = segments.begin(); it != segments.end(); ++it) {
        CCPACSWingSegment& segment = **it;
        TopoDS_Shape upperSegmentShape = segment.GetUpperShape(WING_COORDINATE_SYSTEM);
        TopoDS_Shape lowerSegmentShape = segment.GetLowerShape(WING_COORDINATE_SYSTEM);
        debug.addShape(upperSegmentShape, "upperSegmentShape" + std::to_string(i));
        debug.addShape(lowerSegmentShape, "lowerSegmentShape" + std::to_string(i));
        upperShellSewing.Add(upperSegmentShape);
        lowerShellSewing.Add(lowerSegmentShape);
        sewing.Add(lowerSegmentShape);
        sewing.Add(upperSegmentShape);
        i++;
    }

    cache.innerFace = GetSingleFace(segments.front()->GetInnerClosure(WING_COORDINATE_SYSTEM));
    cache.outerFace = GetSingleFace(segments.back()->GetOuterClosure(WING_COORDINATE_SYSTEM));

    sewing.Add(cache.innerFace);
    sewing.Add(cache.outerFace);

    sewing.Perform();

    TopoDS_Shape& loftShape = cache.loftShape;
    loftShape = sewing.SewedShape();
    debug.addShape(loftShape, "sewedShape");
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

    // transform all shapes
    CTiglTransformation trafo = wing->GetTransformation().getTransformationMatrix();

    PNamedShape upperShell(new CNamedShape(trafo.Transform(upperShellSewing.SewedShape()), upperShape->GetDefaultedUID()));
    PNamedShape lowerShell(new CNamedShape(trafo.Transform(lowerShellSewing.SewedShape()), lowerShape->GetDefaultedUID()));
    cache.upperShape = upperShell;
    cache.lowerShape = lowerShell;

    loftShape = trafo.Transform(loftShape);

    cache.myVolume = 0;
    cache.mySurfaceArea = 0;
}

// Method for building wires for eta-, leading edge-, trailing edge-lines
void CCPACSWingComponentSegment::BuildLines(LinesCache& cache) const
{
    // search for ETA coordinate
    std::vector<gp_Pnt> lePointContainer;
    std::vector<gp_Pnt> tePointContainer;

    const SegmentList& segments = GetSegmentList();
    SegmentList::const_iterator it;
    // get the leading and trailing edge points of all sections
    for (it = segments.begin(); it != segments.end(); ++it) {
        const CCPACSWingSegment& segment = *(*it);

            // get leading edge point
        lePointContainer.push_back(segment.GetPoint(0, 0, true, WING_COORDINATE_SYSTEM));
        // get trailing edge point
        tePointContainer.push_back(segment.GetPoint(0, 1, true, WING_COORDINATE_SYSTEM));
    }
    // finally add the points for the outer section
    // get leading edge point
    lePointContainer.push_back(segments.back()->GetPoint(1, 0, true, WING_COORDINATE_SYSTEM));

    // get trailing edge point
    tePointContainer.push_back(segments.back()->GetPoint(1, 1, true, WING_COORDINATE_SYSTEM));



    // build wires: etaLine, extendedEtaLine, leadingEdgeLine
    BRepBuilderAPI_MakeWire wbEta, wbLe, wbTe;
    gp_Pnt innerLePoint, outerLePoint, innerPoint2d, outerPoint2d, innerTePoint, outerTePoint;
    TopoDS_Edge leEdge, etaEdge, teEdge;
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

        wbLe.Add(leEdge);
        wbEta.Add(etaEdge);
        wbTe.Add(teEdge);
    }

    cache.leadingEdgeLine = wbLe.Wire();
    cache.etaLine = wbEta.Wire();
    cache.trailingEdgeLine = wbTe.Wire();
}

// Gets a point in relative wing coordinates for a given eta and xsi
gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi, TiglCoordinateSystem referenceCS) const
{
    // search for ETA coordinate
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }

    gp_Pnt result;

    if (eta < Precision::Confusion()) {
        result = chordFace->GetPoint(0., xsi);
    }
    else if (1. - eta < Precision::Confusion()) {
        result = chordFace->GetPoint(1., xsi);
    }
    else {
        result = chordFace->GetPoint(eta, xsi);
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        result = wing->GetTransformationMatrix().Inverted().Transform(result);
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        // nothing needs to be done
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingComponentSegment::GetPoint");
    }

    return result;
}

void CCPACSWingComponentSegment::GetEtaXsi(const gp_Pnt& globalPoint, double& eta, double& xsi) const
{
    // TODO (siggel): check that point is part of component segment

    chordFace->GetEtaXsi(globalPoint, eta, xsi);
}

void CCPACSWingComponentSegment::GetEtaXsiLocal(const gp_Pnt& localPoint, double& eta, double& xsi) const
{
    return GetEtaXsi(GetWing().GetTransformationMatrix().Transform(localPoint), eta, xsi);
}

// Getter for eta direction of midplane
gp_Vec CCPACSWingComponentSegment::GetMidplaneEtaDir(double eta) const
{
    // get point on eta line
    gp_Pnt etaPnt;
    gp_Vec etaDir;

    BRepAdaptor_CompCurve etaLineCurve(linesCache->etaLine, Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length( etaLineCurve );
    etaLineCurve.D1( len * eta, etaPnt, etaDir );
    return etaDir.Normalized();
}

// Getter for midplane normal vector
gp_Vec CCPACSWingComponentSegment::GetMidplaneNormal(double eta) const
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetMidplaneOrChordlinePoint", TIGL_ERROR);
    }

    gp_Pnt lePnt = GetPoint(eta, 0., WING_COORDINATE_SYSTEM);
    gp_Pnt tePnt = GetPoint(eta, 1., WING_COORDINATE_SYSTEM);
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
    int segmentIndex = 0;
    for (SegmentList::const_iterator it = segments.begin(); it != segments.end(); ++it) {
        if (segmentUID == (*it)->GetUID()) {
            break;
        }
        segmentIndex++;
    }
    if (segmentIndex == segments.size()) {
        throw CTiglError("Segment does not belong to component segment in CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi", TIGL_ERROR);
    }

    const std::vector<double>& etas = chordFace->GetElementEtas();
    eta = (1. - seta) * etas[segmentIndex] + seta * etas[segmentIndex + 1];
    xsi = sxsi;
}

void CCPACSWingComponentSegment::GetSegmentEtaXsi(double csEta, double csXsi, std::string &segmentUID, double &seta, double &sxsi) const
{
    if (csEta < 0.0 || csEta > 1.0) {
        throw CTiglError("Error: Parameter seta not in the range 0.0 <= seta <= 1.0 in CCPACSWingComponentSegment::GetSegmentEtaXsi", TIGL_ERROR);
    }
    if (csXsi < 0.0 || csXsi > 1.0) {
        throw CTiglError("Error: Parameter csxsi not in the range 0.0 <= csxsi <= 1.0 in CCPACSWingComponentSegment::GetSegmentEtaXsi", TIGL_ERROR);
    }

    const SegmentList& segments = GetSegmentList();
    const std::vector<double>& etas = chordFace->GetElementEtas();

    // find index such that etas[index] <= csEta < etas[index+1]
    unsigned int index = 0;
    for (index = 0; index < segments.size() - 1; ++index) {
        if (etas[index] <= csEta && csEta < etas[index+1]) {
            break;
        }
    }

    seta = (csEta - etas[index]) / (etas[index+1] - etas[index]);
    sxsi = csXsi;
    segmentUID = segments[index]->GetUID();
}

// Returns the volume of this segment
double CCPACSWingComponentSegment::GetVolume()
{
    return geomCache->myVolume;
}

// Returns the surface area of this segment
double CCPACSWingComponentSegment::GetSurfaceArea()
{
    return geomCache->mySurfaceArea;
}

// Returns the segment to a given point on the componentSegment. 
// Returns null if the point is not an that wing!
const CCPACSWingSegment* CCPACSWingComponentSegment::findSegment(double x, double y, double z, gp_Pnt& nearestPoint, double& deviation, double maxDeviation) const
{
    CCPACSWingSegment* result = NULL;
    gp_Pnt pnt(x, y, z);


    const SegmentList& segments = GetSegmentList();

    deviation = std::numeric_limits<double>::max();
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
            if (currentDist < deviation) {
                deviation    = currentDist;
                nearestPoint = currentPoint;
                result       = *segit;
            }
        }
        catch (...) {
            // do nothing
        }
    }

    // check if pnt lies on component segment shape with maxDeviation tolerance (default 1cm)
    if (deviation > maxDeviation) {
        return NULL;
    }

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

            if (cell.IsInside(eta, xsi)) {
                list.push_back(&(cell.GetMaterial()));
            }
        }

        // add complete skin, only if no cells are defined
        if (list.empty()){
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
