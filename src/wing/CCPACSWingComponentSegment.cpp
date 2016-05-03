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

#include <math.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <limits>

#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingProfile.h"
#include "CTiglLogging.h"
#include "CCPACSWingCell.h"
#include "CTiglApproximateBsplineWire.h"
#include "tiglcommonfunctions.h"
#include "TixiSaveExt.h"
#include "CCPACSWingCSStructure.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Wire.hxx"
#include "GeomAPI_IntCS.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "Geom_Plane.hxx"
#include "gp_Pln.hxx"
//#include "Geom_Surface.hxx"
#include "GeomLib.hxx"
#include "Precision.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "BRepTools.hxx"
#include "BRepGProp.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "Geom_BSplineCurve.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "BRepClass3d_SolidClassifier.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "TColgp_Array1OfPnt.hxx"
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <BRepAdaptor_CompCurve.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <GC_MakeSegment.hxx>

#include <Geom_TrimmedCurve.hxx>
#include <gp_Pln.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <TopExp.hxx>

#ifdef _DEBUG
#include <assert.h>
#endif // _DEBUG

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


// Constructor
CCPACSWingComponentSegment::CCPACSWingComponentSegment(CCPACSWing* aWing, int aSegmentIndex)
    : CTiglAbstractSegment(aSegmentIndex)
    , CCPACSWingComponentSegmentLineFunctionality(*this)
    , wing(aWing)
    , surfacesAreValid(false)
    , structure()
{
    Cleanup();
}

// Destructor
CCPACSWingComponentSegment::~CCPACSWingComponentSegment(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWingComponentSegment::Invalidate(void)
{
    // call parent class instead of directly setting invalidated flag
    CTiglAbstractSegment::Invalidate();
    surfacesAreValid = false;
    projLeadingEdge.Nullify();
    wingSegments.clear();
    structure.Invalidate();
    CCPACSWingComponentSegmentLineFunctionality::Invalidate();
}

// Cleanup routine
void CCPACSWingComponentSegment::Cleanup(void)
{
    name = "";
    fromElementUID = "";
    toElementUID   = "";
    myVolume       = 0.;
    mySurfaceArea  = 0.;
    surfacesAreValid = false;
    CTiglAbstractSegment::Cleanup();
    structure.Cleanup();
    projLeadingEdge.Nullify();
    wingSegments.clear();
}

// Update internal segment data
void CCPACSWingComponentSegment::Update(void)
{
    Invalidate();
}

// Read CPACS segment elements
void CCPACSWingComponentSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
{
    Cleanup();

    std::string elementPath;

    // Get subelement "name"
    char* ptrName = NULL;
    elementPath = segmentXPath + "/name";
    if (tixiGetTextElement(tixiHandle, elementPath.c_str(), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get attribute "uid"
    char* ptrUID = NULL;
    elementPath = segmentXPath;
    if (tixiGetTextAttribute(tixiHandle, elementPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get fromElementUID
    char* ptrFromElementUID = NULL;
    elementPath = segmentXPath + "/fromElementUID";
    if (tixiGetTextElement(tixiHandle, elementPath.c_str(), &ptrFromElementUID) == SUCCESS) {
        fromElementUID = ptrFromElementUID;
    }

    // Get toElementUID
    char* ptrToElementUID = NULL;
    elementPath = segmentXPath + "/toElementUID";
    if (tixiGetTextElement(tixiHandle, elementPath.c_str(), &ptrToElementUID) == SUCCESS) {
        toElementUID = ptrToElementUID;
    }
        
    // read structure
    elementPath = segmentXPath + "/structure";
    if (tixiCheckElement(tixiHandle, elementPath.c_str()) == SUCCESS){
        structure.ReadCPACS(tixiHandle, elementPath);
    }

    Update();
}

// Write CPACS segment elements
void CCPACSWingComponentSegment::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
{
    std::string elementPath;

    // Set subelement "name"
    TixiSaveExt::TixiSaveTextElement(tixiHandle, segmentXPath.c_str(), "name", name.c_str());

    // Set attribute "uID"
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, segmentXPath.c_str(), "uID", GetUID().c_str());

    // Set fromElementUID qnd toElementUID
    TixiSaveExt::TixiSaveTextElement(tixiHandle, segmentXPath.c_str(), "fromElementUID", fromElementUID.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, segmentXPath.c_str(), "toElementUID", toElementUID.c_str());
    
    elementPath = segmentXPath + "/structure";
    TixiSaveExt::TixiSaveElement(tixiHandle, segmentXPath.c_str(), "structure");
    structure.WriteCPACS(tixiHandle, elementPath);
}

// Returns the wing this segment belongs to
CCPACSWing& CCPACSWingComponentSegment::GetWing(void) const
{
    return *wing;
}

// Getter of the structure object
CCPACSWingCSStructure& CCPACSWingComponentSegment::GetStructure()
{
    return structure;
}

// Getter for upper Shape
TopoDS_Shape CCPACSWingComponentSegment::GetUpperShape(void)
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return upperShape;
}

// Getter for lower Shape
TopoDS_Shape CCPACSWingComponentSegment::GetLowerShape(void)
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return lowerShape;
}

// Getter for inner segment face
TopoDS_Face CCPACSWingComponentSegment::GetInnerFace(void)
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return innerFace;
}

// Getter for outer segment face
TopoDS_Face CCPACSWingComponentSegment::GetOuterFace(void)
{
    // NOTE: Because it is not clear whether loft.IsNull or invalidated defines
    //       a valid state i call GetLoft here to ensure the geometry was built
    GetLoft();
    return outerFace;
}

TIGL_EXPORT gp_Pnt CCPACSWingComponentSegment::GetSectionElementChordlinePoint(const std::string& sectionElementUID, double xsi) const
{
    gp_Pnt chordlinePoint;
    // find section element
    CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(1);
    if (sectionElementUID == segment.GetInnerSectionElementUID()) {
        // convert into wing coordinate system
        CTiglTransformation wingTrans = wing->GetTransformation();
        chordlinePoint = wingTrans.Inverted().Transform(segment.GetChordPoint(0, xsi));
    }
    else {
        int i;
        for (i = 1; i <= wing->GetSegmentCount(); ++i) {
            CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(i);
            if (sectionElementUID == segment.GetOuterSectionElementUID()) {
                // convert into wing coordinate system
                CTiglTransformation wingTrans = wing->GetTransformation();
                chordlinePoint = wingTrans.Inverted().Transform(segment.GetChordPoint(1, xsi));
                break;
            }
        }
        if (i > wing->GetSegmentCount()) {
            throw CTiglError("Error in CCPACSWingComponentSegment::GetSectionElementChordlinePoint: section element not found!");
        }
    }
    return chordlinePoint;
}

TopoDS_Face CCPACSWingComponentSegment::GetSectionElementFace(const std::string& sectionElementUID)
{
    // get all segments for this component segment
    const SegmentList& segmentList = GetSegmentList();
    SegmentList::const_iterator it;
    // iterate over all segments
    for (it = segmentList.begin(); it != segmentList.end(); ++it) {
        // check for inner section of first segment
        if (it == segmentList.begin()) {
            if ((*it)->GetInnerSectionElementUID() == sectionElementUID) {
                return GetSingleFace((*it)->GetInnerClosure(CCPACSWingSegment::WING_COORDINATE_SYSTEM));
            }
        }
        if ((*it)->GetOuterSectionElementUID() == sectionElementUID) {
            return GetSingleFace((*it)->GetOuterClosure(CCPACSWingSegment::WING_COORDINATE_SYSTEM));
        }
    }
    throw CTiglError("Error getting section element face from component segment!");
}


// Getter for the normalized leading edge direction
gp_Vec CCPACSWingComponentSegment::GetLeadingEdgeDirection(const std::string& segmentUID) const
{
    tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(segmentUID);
    gp_Pnt pl0 = segment.GetPoint(0, 0, false, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
    gp_Pnt pl1 = segment.GetPoint(1, 0, false, CCPACSWingSegment::WING_COORDINATE_SYSTEM);

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
    const CTiglAbstractSegment* segment = findSegmentViaShape(globalPnt.X(), globalPnt.Y(), globalPnt.Z());
    if (segment != NULL) {
        segmentUID = segment->GetUID();
    }
    return GetLeadingEdgeDirection(segmentUID);
}

// Getter for the normalized trailing edge direction
gp_Vec CCPACSWingComponentSegment::GetTrailingEdgeDirection(const std::string& segmentUID) const
{
    tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(segmentUID);
    gp_Pnt pt0 = segment.GetPoint(0, 1, false, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
    gp_Pnt pt1 = segment.GetPoint(1, 1, false, CCPACSWingSegment::WING_COORDINATE_SYSTEM);

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
    const CTiglAbstractSegment* segment = findSegmentViaShape(globalPnt.X(), globalPnt.Y(), globalPnt.Z());
    if (segment != NULL) {
        segmentUID = segment->GetUID();
    }

    return GetTrailingEdgeDirection(segmentUID);
}

// Getter for the normalized leading edge direction in the YZ plane
gp_Vec CCPACSWingComponentSegment::GetLeadingEdgeDirectionYZ(const std::string& segmentUID) const
{
    gp_Vec lev = GetLeadingEdgeDirection(segmentUID);
    lev.SetX(0);
    lev.Normalize();
    return lev;
}

// Getter for the normalized leading edge direction in the YZ plane
// Parameter defaultSegmentUID can be used for inner/outer segment when point is in extended volume
gp_Vec CCPACSWingComponentSegment::GetLeadingEdgeDirectionYZ(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    gp_Vec lev = GetLeadingEdgeDirection(point, defaultSegmentUID);
    lev.SetX(0);
    lev.Normalize();
    return lev;
}

// Getter the normal vector of the leading edge of the segment containing the passed point
gp_Vec CCPACSWingComponentSegment::GetLeadingEdgeNormal(const gp_Pnt& point, const std::string& defaultSegmentUID) const
{
    // get direction of leading edge in YZ plane
    gp_Vec lev = GetLeadingEdgeDirectionYZ(point, defaultSegmentUID);

    // rotate normalized leading edge vector by 90 degrees in order to get up vector in YZ-plane
    gp_Vec upVec(0,0,0);
    upVec.SetY(-lev.Z());
    upVec.SetZ(lev.Y());

    return upVec;
}

// Getter for normal vector of inner section
gp_Vec CCPACSWingComponentSegment::GetInnerSectionNormal() const
{
    int numSections = wing->GetSectionCount();
    for (int i = 1; i <= numSections; i++) {
        CCPACSWingSection& section = wing->GetSection(i);
        // TODO: only sections with single element supported here!!!
        if (section.GetSectionElement(1).GetUID() == fromElementUID) {
            gp_GTrsf elementTrans = section.GetSectionElement(1).GetSectionElementTransformation().Get_gp_GTrsf();
            gp_GTrsf sectionTrans = section.GetSectionTransformation().Get_gp_GTrsf();
            // remove translation part
            elementTrans.SetTranslationPart(gp_XYZ(0, 0, 0));
            sectionTrans.SetTranslationPart(gp_XYZ(0, 0, 0));

            gp_XYZ upDir(0,0,1);
            // first apply element transformation
            elementTrans.Transforms(upDir);
            // next apply section transformation
            sectionTrans.Transforms(upDir);

            gp_Vec upVec(upDir.X(), upDir.Y(), upDir.Z());
            return upVec.Normalized();
        }
    }
    throw CTiglError("INTERNAL ERROR: Unable to find inner section normal in method CCPACSWingComponentSegment::GetInnerSectionNormal!");
}

// Getter for normal vector of outer section
gp_Vec CCPACSWingComponentSegment::GetOuterSectionNormal() const
{
    int numSections = wing->GetSectionCount();
    for (int i = 1; i <= numSections; i++) {
        CCPACSWingSection& section = wing->GetSection(i);
        // TODO: only sections with single element supported here!!!
        if (section.GetSectionElement(1).GetUID() == toElementUID) {
            gp_GTrsf elementTrans = section.GetSectionElement(1).GetSectionElementTransformation().Get_gp_GTrsf();
            gp_GTrsf sectionTrans = section.GetSectionTransformation().Get_gp_GTrsf();
            // remove translation part
            elementTrans.SetTranslationPart(gp_XYZ(0, 0, 0));
            sectionTrans.SetTranslationPart(gp_XYZ(0, 0, 0));

            gp_XYZ upDir(0,0,1);
            // first apply element transformation
            elementTrans.Transforms(upDir);
            // next apply section transformation
            sectionTrans.Transforms(upDir);

            gp_Vec upVec(upDir.X(), upDir.Y(), upDir.Z());
            return upVec.Normalized();
        }
    }
    throw CTiglError("INTERNAL ERROR: Unable to find inner section normal in method CCPACSWingComponentSegment::GetInnerSectionNormal!");
}

// Getter for the midplane line between two eta-xsi points
TopoDS_Wire CCPACSWingComponentSegment::GetMidplaneLine(const gp_Pnt& startPoint, const gp_Pnt& endPoint)
{
    // determine start and end point
    // copy of variables because we have to modify them in case the points lie within a section
    gp_Pnt startPnt = startPoint;
    gp_Pnt endPnt = endPoint;
    gp_Pnt globalStartPnt = wing->GetWingTransformation().Transform(startPnt);
    gp_Pnt globalEndPnt = wing->GetWingTransformation().Transform(endPnt);

    // determine wing segments containing the start and end points
    std::string startSegmentUID, endSegmentUID;
    const CTiglAbstractSegment* startSegment = findSegmentViaShape(globalStartPnt.X(), globalStartPnt.Y(), globalStartPnt.Z());
    if (startSegment != NULL) {
        startSegmentUID = startSegment->GetUID();
    }
    const CTiglAbstractSegment* endSegment = findSegmentViaShape(globalEndPnt.X(), globalEndPnt.Y(), globalEndPnt.Z());
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
        gp_Pnt pl = segment.GetPoint(0, 0, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
        gp_Pnt pt = segment.GetPoint(0, 1, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
        TopoDS_Edge chordLine = BRepBuilderAPI_MakeEdge(pl, pt);
        GetIntersectionPoint(cutFace, chordLine, startPnt);
    }
    if (endPntInSection) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&)wing->GetSegment(endSegmentUID);
        gp_Pnt pl = segment.GetPoint(1, 0, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
        gp_Pnt pt = segment.GetPoint(1, 1, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
        TopoDS_Edge chordLine = BRepBuilderAPI_MakeEdge(pl, pt);
        GetIntersectionPoint(cutFace, chordLine, endPnt);
    }

    // handle case when start and end point are in the same segment
    if (startSegmentUID == endSegmentUID) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(startPnt, endPnt);
        wireBuilder.Add(edge);
        return wireBuilder.Wire();
    }

    bool inSegment = false;
    gp_Pnt prevPnt = startPnt;
    // Iterate over all segments and build midplane line
    for (int i = 1; i <= wing->GetSegmentCount(); i++) {
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(i);
        if (!inSegment && segment.GetUID() == startSegmentUID) {
            inSegment = true;
        }

        if (inSegment) {
            // add intersection with end section only in case end point is skipped
            if (segment.GetUID() != endSegmentUID) {
                // compute outer chord line
                gp_Pnt pl = segment.GetPoint(1, 0, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
                gp_Pnt pt = segment.GetPoint(1, 1, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);

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
    }
    return wireBuilder.Wire();
}

// Getter for inner segment UID
std::string CCPACSWingComponentSegment::GetInnerSegmentUID() const
{
    std::string uid;
    for (int i = 1; i <= wing->GetSegmentCount(); i++) {
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(i);
        // Check for the inner wing segment
        if (segment.GetInnerSectionElementUID() == fromElementUID) {
            uid = segment.GetUID();
            break;
        }
    }
    return uid;
}

// Getter for outer segment UID
std::string CCPACSWingComponentSegment::GetOuterSegmentUID() const
{
    std::string uid;
    for (int i = 1; i <= wing->GetSegmentCount(); i++) {
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing->GetSegment(i);
        // Check for the inner wing segment
        if (segment.GetOuterSectionElementUID() == toElementUID) {
            uid = segment.GetUID();
            break;
        }
    }
    return uid;
}

SegmentList& CCPACSWingComponentSegment::GetSegmentList()
{
    if (wingSegments.size() == 0) {
        std::vector<int> path;
        path = findPath(fromElementUID, toElementUID, path, true);

        if (path.size() == 0) {
            // could not find path from fromUID to toUID
            // try the other way around
            path = findPath(toElementUID, fromElementUID, path, true);
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
    // number of component segment point samples per line
    int NSTEPS = 11;
        
    CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(segmentUID);
    bool hasIntersected = false;
        
    // we do an iterative procedure to find the segment intersection
    // by trying to find out, what the exact intersection of the component
    // segment with the segment is
    int iter = 0;
    int maxiter = 10;
    gp_Pnt result(0,0,0);
    gp_Pnt oldresult(100,0,0);
        
    while (result.Distance(oldresult) > 1e-6 && iter < maxiter) {
        oldresult = result;
            
        double deta = (csEta2-csEta1)/double(NSTEPS-1);
        double dxsi = (csXsi2-csXsi1)/double(NSTEPS-1);
        
        std::vector<gp_Pnt> points;
        for (int istep = 0; istep < NSTEPS; ++istep) {
            double eta = csEta1 + (double) istep * deta;
            double xsi = csXsi1 + (double) istep * dxsi;
            gp_Pnt point = GetPoint(eta,xsi);
            points.push_back(point);
        }
            
        BRepBuilderAPI_MakeWire wireBuilder;
        for (int istep = 0; istep < NSTEPS-1; ++istep) {
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(points[istep], points[istep+1]);
            wireBuilder.Add(edge);
        }
            
        //return wireGen.BuildWire(container,false);
        TopoDS_Wire csLine = wireBuilder.Wire();
        
        // create segments outer chord line
        gp_Pnt leadingPoint  = segment.GetChordPoint(eta, 0.);
        gp_Pnt trailingPoint = segment.GetChordPoint(eta, 1.);
            
        TopoDS_Wire outerChord = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(leadingPoint,trailingPoint));
            
        BRepExtrema_DistShapeShape extrema(csLine, outerChord);
        extrema.Perform();
            
        double dist = 0;
        if (extrema.IsDone() && extrema.NbSolution() > 0) {
            gp_Pnt p1 = extrema.PointOnShape1(1);
            gp_Pnt p2 = extrema.PointOnShape2(1);
            dist = p1.Distance(p2);
            result = p2;
            // check if the lines were really intersecting (1cm accuracy should be enough)
            if (dist < 1e-2) {
                hasIntersected = true;
            }
        }

            
        // now lets check in between which points the intersection lies
        int ifound = 0;
        for (int i = 0; i < NSTEPS-1; ++i) {
            if (inBetween(result, points[i], points[i+1])) {
                ifound = i;
                break;
            }
        }
            
        // calculate new search field
        csEta2 = csEta1 + deta*(ifound + 1);
        csEta1 = csEta1 + deta*(ifound);
        csXsi2 = csXsi1 + dxsi*(ifound + 1);
        csXsi1 = csXsi1 + dxsi*(ifound);
        ++iter;
    }
        
    if (hasIntersected) {
        // now check if we found an intersection
        double etaTmp;
        segment.GetEtaXsi(result, etaTmp, xsi);
        // by design, result is inside the segment
        // However due to numerics, eta and xsi might
        // be a bit larger than 1 or smaller than 0
        if (etaTmp > 1.) {
            etaTmp = 1.;
        }
        else if (etaTmp < 0) {
            etaTmp = 0.;
        }
        if (xsi > 1.) {
            xsi = 1.;
        }
        else if (xsi < 0) {
            xsi = 0.;
        }

        // check that etaTmp coordinate is correct
        if (fabs(etaTmp - eta) > 1e-6) {
            throw CTiglError("Error determining proper eta, xsi coordinates in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
        }
    }
    else {
        throw CTiglError("Component segment line does not intersect outer segment border in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
    }
        
    // test if eta,xsi is valid
    if (segment.GetChordPoint(eta,xsi).Distance(result) > 1e-6) {
        throw CTiglError("Error determining proper eta, xsi coordinates in CCPACSWingComponentSegment::GetSegmentIntersection.", TIGL_MATH_ERROR);
    }
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
                tigl::CTiglAbstractSegment& wcs = w.GetComponentSegment(j);
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
PNamedShape CCPACSWingComponentSegment::BuildLoft(void)
{
    loft.reset();

    // use sewing for full loft
    BRepBuilderAPI_Sewing sewing;
    BRepBuilderAPI_Sewing upperShellSewing;
    BRepBuilderAPI_Sewing lowerShellSewing;

    const SegmentList& segments = GetSegmentList();
    if (segments.size() == 0) {
        throw CTiglError("Error: Could not find segments in CCPACSWingComponentSegment::BuildLoft", TIGL_ERROR);
    }

    TopoDS_Shape innerShape = segments.front()->GetInnerClosure(CCPACSWingSegment::WING_COORDINATE_SYSTEM);
    innerFace = GetSingleFace(innerShape);
    sewing.Add(innerFace);

    for (SegmentList::const_iterator it = segments.begin(); it != segments.end(); ++it) {
        CCPACSWingSegment& segment = **it;
        TopoDS_Face lowerSegmentFace = GetSingleFace(segment.GetLowerShape(CCPACSWingSegment::WING_COORDINATE_SYSTEM));
        TopoDS_Face upperSegmentFace = GetSingleFace(segment.GetUpperShape(CCPACSWingSegment::WING_COORDINATE_SYSTEM));
        upperShellSewing.Add(upperSegmentFace);
        lowerShellSewing.Add(lowerSegmentFace);
        sewing.Add(lowerSegmentFace);
        sewing.Add(upperSegmentFace);
    }

    TopoDS_Shape outerShape = segments.back()->GetOuterClosure(CCPACSWingSegment::WING_COORDINATE_SYSTEM);
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
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft (new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft, segments.size());
    return loft;
}

void CCPACSWingComponentSegment::UpdateProjectedLeadingEdge()
{
    if ( !projLeadingEdge.IsNull() ) {
        return;
    }

    // add inner sections of each segment
    const SegmentList& segments = GetSegmentList();

    if (segments.size() < 1) {
        std::stringstream str;
        str << "Wing component " << GetUID() << " does not contain any segments (CCPACSWingComponentSegment::updateProjectedLeadingEdge)!";
        throw CTiglError(str.str(), TIGL_ERROR);
    }

    // create projection plane
    gp_GTrsf wingTrafo = wing->GetTransformation().Get_gp_GTrsf();
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
    unsigned int nPoints = LEPointsProjected.size();
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

    // build projected leading edge wire
    BRepBuilderAPI_MakeWire wireBuilder;
    for (unsigned int j = 1; j < LEPointsProjected.size(); j++) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(LEPointsProjected[j - 1], LEPointsProjected[j]);
        wireBuilder.Add(edge);
    }
    projLeadingEdge = wireBuilder.Wire();
}


// Gets a point in relative wing coordinates for a given eta and xsi
gp_Pnt CCPACSWingComponentSegment::GetPoint(double eta, double xsi)
{
    // search for ETA coordinate
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Error: Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }

    UpdateProjectedLeadingEdge();

    const SegmentList& segments = GetSegmentList();
    if (segments.size() < 1) {
        std::stringstream str;
        str << "Wing component " << GetUID() << " does not contain any segments (CCPACSWingComponentSegment::GetPoint)!";
        throw CTiglError(str.str(), TIGL_ERROR);
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

    double wingLenApprox = GetWireLength(projLeadingEdge);

    // create iso xsi line as linear interpolation between xsi points
    GeomAPI_PointsToBSpline linearInterpolation(xsiPoints, 1, 1, GeomAbs_C0, Precision::Confusion());
    Handle_Geom_BoundedCurve xsiCurve = linearInterpolation.Curve();

    // extend iso xsi line, so that we can still intersect it
    // even if we have twisted sections
    gp_Pnt p; gp_Vec v;
    xsiCurve->D1(xsiCurve->LastParameter(), p, v);
    GeomLib::ExtendCurveToPoint(xsiCurve, p.XYZ() + v.Normalized().XYZ()*wingLenApprox, 1, true);
    xsiCurve->D1(xsiCurve->FirstParameter(), p, v);
    GeomLib::ExtendCurveToPoint(xsiCurve, p.XYZ() - v.Normalized().XYZ()*wingLenApprox, 1, false);

    // compute eta point and normal on the projected LE
    gp_Pnt etaPnt; gp_Vec etaNormal;
    WireGetPointTangent(projLeadingEdge, eta, etaPnt, etaNormal);

    // plane normal to projected leading edge
    gp_Pln gpPlane(etaPnt, gp_Dir(etaNormal.XYZ()));
    Handle_Geom_Surface plane = new Geom_Plane(gpPlane);

    // compute intersection of plane with iso-xsi line
    GeomAPI_IntCS Intersector(xsiCurve, plane);
    if (Intersector.IsDone() && Intersector.NbPoints() > 0) {
        gp_Pnt p = Intersector.Point(1);
        return p;
    }
    else {
        // Fallback mode, if they are not intersecting
        // we don't need it, if we make leading/trailing edge longer
        TopoDS_Shape xsiWire = BRepBuilderAPI_MakeEdge(xsiCurve);
        TopoDS_Shape etaFace = BRepBuilderAPI_MakeFace(gpPlane);
        BRepExtrema_DistShapeShape extrema(xsiWire, etaFace);
        extrema.Perform();
        return extrema.PointOnShape1(1);
    }
}

void CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi(const std::string& segmentUID, double seta, double sxsi, double& eta, double& xsi)
{
    // search for ETA coordinate
        
    if (seta < 0.0 || seta > 1.0) {
        throw CTiglError("Error: Parameter seta not in the range 0.0 <= seta <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
    }
    if (sxsi < 0.0 || sxsi > 1.0) {
        throw CTiglError("Error: Parameter sxsi not in the range 0.0 <= sxsi <= 1.0 in CCPACSWingComponentSegment::GetPoint", TIGL_ERROR);
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
        throw CTiglError("Error: segment does not belong to component segment in CCPACSWingComponentSegment::GetEtaXsiFromSegmentEtaXsi", TIGL_ERROR);
    }

    gp_Pnt point3d = segment->GetChordPoint(seta, sxsi);
    xsi = sxsi;

    UpdateProjectedLeadingEdge();
    eta = ProjectPointOnWire(projLeadingEdge, point3d);
}


// Returns the volume of this segment
double CCPACSWingComponentSegment::GetVolume(void)
{
    GetLoft();
    return( myVolume );
}

// Returns the surface area of this segment
double CCPACSWingComponentSegment::GetSurfaceArea(void)
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
//            throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
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

// Gets the fromElementUID of this segment
const std::string & CCPACSWingComponentSegment::GetFromElementUID(void) const
{
    return fromElementUID;
}

// Gets the toElementUID of this segment
const std::string & CCPACSWingComponentSegment::GetToElementUID(void) const
{
    return toElementUID;
}

// Returns the segment to a given point on the componentSegment. 
// Returns null if the point is not an that wing!
const CTiglAbstractSegment* CCPACSWingComponentSegment::findSegment(double x, double y, double z, gp_Pnt& nearestPoint)
{
    CTiglAbstractSegment* result = NULL;
    gp_Pnt pnt(x, y, z);


    SegmentList& segments = GetSegmentList();

    double minDist = std::numeric_limits<double>::max();
    // now discover to which segment the point belongs
    for (SegmentList::iterator segit = segments.begin(); segit != segments.end(); ++segit) {
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

    // check if pnt lies on component segment shape with 1cm tolerance
    if (minDist > 1.e-2) {
        return NULL;
    }

    return result;
}

MaterialList CCPACSWingComponentSegment::GetMaterials(double eta, double xsi, TiglStructureType type)
{
    MaterialList list;
        
    if (!structure.IsValid()) {
        // return empty list
        return list;
    }
        
    if (type != UPPER_SHELL && type != LOWER_SHELL) {
        LOG(ERROR) << "Cannot compute materials for inner structure in CCPACSWingComponentSegment::GetMaterials (not yet implemented)";
        return list;
    }
    else {
        CCPACSWingShell* shell = (type == UPPER_SHELL? &structure.GetUpperShell() : &structure.GetLowerShell());
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
    std::string nextElementUID = fromElementUID;
    int segmentCount = wing->GetSegmentCount();

    for (int i=1; i <= segmentCount; i++) {
        CCPACSWingSegment& testSegment = (CCPACSWingSegment&) wing->GetSegment(i);
        std::string innerSectionElementUID = testSegment.GetInnerSectionElementUID();
        // skip segments with not matching inner element UID
        if (innerSectionElementUID != nextElementUID) {
            continue;
        }
        if (innerSectionElementUID == toElementUID) {
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

// Getter for upper and lower shell
CCPACSWingShell& CCPACSWingComponentSegment::GetUpperShell()
{
    return structure.GetUpperShell();
}

// Getter for upper and lower shell
CCPACSWingShell& CCPACSWingComponentSegment::GetLowerShell()
{
    return structure.GetLowerShell();
}

TopoDS_Shape CCPACSWingComponentSegment::GetMidplaneShape()
{
    int startSegmentIndex = GetStartSegmentIndex();
    int numWingSegments = wing->GetSegmentCount();

    BRep_Builder builder;
    TopoDS_Shell midplaneShape;
    builder.MakeShell(midplaneShape);

    gp_Pnt innerLePnt, innerTePnt, outerLePnt, outerTePnt;
    for (int i = startSegmentIndex; i <= numWingSegments; i++) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&)wing->GetSegment(i);

        if (i == startSegmentIndex) {
            innerLePnt = segment.GetChordPoint(0, 0);
            innerTePnt = segment.GetChordPoint(0, 1);
        }
        else {
            innerLePnt = outerLePnt;
            innerTePnt = outerTePnt;
        }
        outerLePnt = segment.GetChordPoint(1, 0);
        outerTePnt = segment.GetChordPoint(1, 1);

        TopoDS_Face face = BuildFace(innerLePnt, innerTePnt, outerLePnt, outerTePnt);
        builder.Add(midplaneShape, face);

        // stop at last segment of componentSegment
        if (segment.GetOuterSectionElementUID() == toElementUID) {
            break;
        }
    }
    return midplaneShape;
}

int CCPACSWingComponentSegment::GetStartSegmentIndex()
{
    int startSegmentIndex = 0;
    int segmentCount = wing->GetSegmentCount();
    for (int i=1; i <= segmentCount; i++) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) wing->GetSegment(i);
        if (segment.GetInnerSectionElementUID() == fromElementUID) {
            startSegmentIndex = i;
            break;
        }
    }
    return startSegmentIndex;
}


CCPACSWingCell& CCPACSWingComponentSegment::GetCellByUID(std::string cellUID) // TODO: this algorithm operates solely on the structure, move to corresponding class?
{
    CCPACSWingShell& upper = structure.GetUpperShell();
    for (int c = 1; c <= upper.GetCellCount(); c++) {
        CCPACSWingCell& cell = upper.GetCell(c);
        if (cell.GetUID() == cellUID) {
            return cell;
        }
    }
    
    CCPACSWingShell& lower = structure.GetLowerShell();
    for (int c = 1; c <= lower.GetCellCount(); c++) {
        CCPACSWingCell& cell = lower.GetCell(c);
        if (cell.GetUID() == cellUID) {
            return cell;
        }
    }
    
    throw CTiglError("Error: no cell with uID " + cellUID + " found in CCPACSWingComponentSegment::getCellByUID!");
    
}

// Getter for the member name
const std::string& CCPACSWingComponentSegment::GetName(void) const
{
    return name;
}

const CTiglAbstractSegment* CCPACSWingComponentSegment::findSegmentViaShape(double x, double y, double z) const
{
    // old code from TIGL 2.0.4
    int i = 0;
    CTiglAbstractSegment* result = NULL;
    int segmentCount = wing->GetSegmentCount();
    gp_Pnt pnt(x, y, z);

    // now discover the right segment
    for (i = 1; i <= segmentCount; i++) {
        // added check for only handling segments which are contained in the component segment
        CCPACSWingSegment& segment = static_cast<CCPACSWingSegment&>(wing->GetSegment(i));
        if (!IsSegmentContained(segment)) {
            continue;
        }
        TopoDS_Shape segmentLoft = wing->GetSegment(i).GetLoft()->Shape();
        BRepClass3d_SolidClassifier classifier;
        classifier.Load(segmentLoft);
        classifier.Perform(pnt, 1.0e-3);
        TopAbs_State aState = classifier.State();
        if ((classifier.State() == TopAbs_IN) || (classifier.State() == TopAbs_ON)) {
            result = &wing->GetSegment(i);
            break;
        }
    }

    return result;
}

} // end namespace tigl
