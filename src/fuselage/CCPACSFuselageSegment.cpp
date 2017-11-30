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
* @brief  Implementation of CPACS fuselage segment handling routines.
*/
#include <iostream>
#include <cmath>

#include "CCPACSFuselageSegment.h"
#include "CCPACSFuselage.h"

#include "CCPACSFuselageProfile.h"
#include "CCPACSConfiguration.h"
#include "CTiglUIDManager.h"
#include "generated/CPACSGuideCurve.h"
#include "CCPACSGuideCurveProfiles.h"
#include "CCPACSGuideCurveAlgo.h"
#include "CCPACSFuselageProfileGetPointAlgo.h"
#include "CTiglLogging.h"
#include "CCPACSConfiguration.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shell.hxx"
#include "TopoDS.hxx"
#include "BRep_Tool.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GC_MakeSegment.hxx"
#include "gp_Lin.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Geom_Surface.hxx"
#include "gce_MakeLin.hxx"
#include "Geom_Line.hxx"
#include "IntCurvesFace_Intersector.hxx"
#include "Precision.hxx"
#include "TopLoc_Location.hxx"
#include "Poly_Triangulation.hxx"
#include "BRep_Builder.hxx"
#include "Poly_Array1OfTriangle.hxx"
#include "gp_Trsf.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepMesh.hxx"
#include "BRepTools.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "BRepAlgoAPI_Section.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "BRepBuilderAPI_Transform.hxx"
#include "ShapeAnalysis_Surface.hxx"
#include "BRepLib_FindSurface.hxx"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace
{
    gp_Pnt transformProfilePoint(const tigl::CTiglTransformation& fuselTransform, const tigl::CTiglFuselageConnection& connection, const gp_Pnt& pointOnProfile)
    {
        // Do section element transformation on points
        tigl::CTiglTransformation trafo = connection.GetSectionElementTransformation();

        // Do section transformations
        trafo.PreMultiply(connection.GetSectionTransformation());

        // Do positioning transformations
        trafo.PreMultiply(connection.GetPositioningTransformation());

        trafo.PreMultiply(fuselTransform);

        gp_Pnt transformedPoint = trafo.Transform(pointOnProfile);

        return transformedPoint;
    }

    TopoDS_Wire transformProfileWire(const tigl::CTiglTransformation& fuselTransform, const tigl::CTiglFuselageConnection& connection, const TopoDS_Wire& wire)
    {
        // Do section element transformation on points
        tigl::CTiglTransformation trafo = connection.GetSectionElementTransformation();

        // Do section transformations
        trafo.PreMultiply(connection.GetSectionTransformation());

        // Do positioning transformations
        trafo.PreMultiply(connection.GetPositioningTransformation());

        trafo.PreMultiply(fuselTransform);

        TopoDS_Shape transformedWire = trafo.Transform(wire);

        // Cast shapes to wires, see OpenCascade documentation
        if (transformedWire.ShapeType() != TopAbs_WIRE) {
            throw tigl::CTiglError("Wrong shape type in CCPACSFuselageSegment::transformProfileWire", TIGL_ERROR);
        }

        return TopoDS::Wire(transformedWire);
    }
}

namespace tigl
{

CCPACSFuselageSegment::CCPACSFuselageSegment(CCPACSFuselageSegments* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSegment(parent, uidMgr)
    , CTiglAbstractSegment(parent->GetSegments(), parent->GetParent()->m_symmetry)
    , fuselage(parent->GetParent())
{
    Cleanup();
}

// Destructor
CCPACSFuselageSegment::~CCPACSFuselageSegment()
{
    // unregister
    GetFuselage().GetConfiguration().GetUIDManager().RemoveGeometricComponent(m_uID);

    Cleanup();
}

// Cleanup routine
void CCPACSFuselageSegment::Cleanup()
{
    m_name = "";
    myVolume      = 0.;
    mySurfaceArea = 0.;
    myWireLength  = 0.;
    _continuity    = C2;
    CTiglAbstractGeometricComponent::Reset();
}

void CCPACSFuselageSegment::Invalidate()
{
    CTiglAbstractSegment::Reset();
}

// Read CPACS segment elements
void CCPACSFuselageSegment::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& segmentXPath)
{
    Cleanup();
    generated::CPACSFuselageSegment::ReadCPACS(tixiHandle, segmentXPath);

    if (m_uidMgr) {
        m_uidMgr->AddGeometricComponent(m_uID, this);
    }

    // trigger creation of connections
    SetFromElementUID(m_fromElementUID);
    SetToElementUID(m_toElementUID);

    // TODO: continuity does not exist in CPACS spec

    //// Continuity
    //char* ptrCont = NULL;
    //if (tixiGetTextElement(tixiHandle, (segmentXPath + "/continuity").c_str(); , &ptrCont) == SUCCESS) {
    //    if (strcmp(ptrCont, "C0") == 0) {
    //        continuity = TiglContinuity(C0);
    //    }
    //    else if (strcmp(ptrCont, "C1") == 0) {
    //        continuity = TiglContinuity(C1);
    //    }
    //    else if (strcmp(ptrCont, "C2") == 0) {
    //        continuity = TiglContinuity(C2);
    //    }
    //    else {
    //        LOG(ERROR) << "Invalid continuity specifier " << ptrCont << " for UID " << GetUID();
    //        continuity = C2;
    //    }
    //} else {
    //    continuity = C2;
    //}

    Invalidate();
}

void CCPACSFuselageSegment::SetUID(const std::string& uid) {
    if (m_uidMgr) {
        m_uidMgr->TryRemoveGeometricComponent(m_uID);
        m_uidMgr->AddGeometricComponent(uid, this);
    }
    generated::CPACSFuselageSegment::SetUID(uid);
}

std::string CCPACSFuselageSegment::GetDefaultedUID() const {
    return generated::CPACSFuselageSegment::GetUID();
}

void CCPACSFuselageSegment::SetFromElementUID(const std::string& value) {
    generated::CPACSFuselageSegment::SetFromElementUID(value);
    startConnection = CTiglFuselageConnection(m_fromElementUID, this);
}

void CCPACSFuselageSegment::SetToElementUID(const std::string& value) {
    generated::CPACSFuselageSegment::SetToElementUID(value);
    endConnection = CTiglFuselageConnection(m_toElementUID, this);
}

// Returns the fuselage this segment belongs to
CCPACSFuselage& CCPACSFuselageSegment::GetFuselage() const
{
    return *fuselage;
}

// helper function to get the wire of the start section
TopoDS_Wire CCPACSFuselageSegment::GetStartWire()
{
    CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
    return transformProfileWire(GetFuselage().GetTransformationMatrix(), startConnection, startProfile.GetWire(true));
}

// helper function to get the wire of the end section
TopoDS_Wire CCPACSFuselageSegment::GetEndWire()
{
    CCPACSFuselageProfile& endProfile = endConnection.GetProfile();
    return transformProfileWire(GetFuselage().GetTransformationMatrix(), endConnection, endProfile.GetWire(true));
}

// get short name for loft
std::string CCPACSFuselageSegment::GetShortShapeName()
{
    unsigned int findex = 0;
    unsigned int fsindex = 0;
    for (int i = 1; i <= fuselage->GetConfiguration().GetFuselageCount(); ++i) {
        tigl::CCPACSFuselage& f = fuselage->GetConfiguration().GetFuselage(i);
        if (fuselage->GetUID() == f.GetUID()) {
            findex = i;
            for (int j = 1; j <= f.GetSegmentCount(); j++) {
                CCPACSFuselageSegment& fs = f.GetSegment(j);
                if (GetUID() == fs.GetUID()) {
                    fsindex = j;
                    std::stringstream shortName;
                    shortName << "F" << findex << "S" << fsindex;
                    return shortName.str();
                }
            }
        }
    }
    return "UNKNOWN";
}

void CCPACSFuselageSegment::SetFaceTraits (PNamedShape loft, bool hasSymmetryPlane)
{
    // this is currently only valid without guides

    int nFaces = GetNumberOfFaces(loft->Shape());

    std::vector<std::string> names;
    names.push_back(loft->Name());
    names.push_back("symmetry");
    names.push_back("Front");
    names.push_back("Rear");


    int facesPerSegment = hasSymmetryPlane ? 2 : 1;
    int remainingFaces = nFaces - facesPerSegment;
    if (remainingFaces < 0 || remainingFaces > 2) {
        throw CTiglError("Fuselage shape seems to be invalid");
    }

    int iFaceTotal = 0;
    for (int iFace = 0; iFace < facesPerSegment; ++iFace) {
        loft->FaceTraits(iFaceTotal++).SetName(names[iFace].c_str());
    }

    // set the caps
    int iFace = 2;
    for (;iFaceTotal < nFaces; ++iFaceTotal) {
        loft->FaceTraits(iFaceTotal).SetName(names[iFace++].c_str());
    }
}

// Builds the loft between the two segment sections
PNamedShape CCPACSFuselageSegment::BuildLoft()
{
    // Build loft
    //BRepOffsetAPI_ThruSections generator(Standard_False, Standard_False, Precision::Confusion());
    BRepOffsetAPI_ThruSections generator(Standard_True, Standard_False, Precision::Confusion());
    generator.AddWire(GetStartWire());
    generator.AddWire(GetEndWire());
    generator.CheckCompatibility(Standard_False);
    generator.Build();
    TopoDS_Shape loftShape = generator.Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(loftShape, System);
    myVolume = System.Mass();

    // Calculate surface area
    TopExp_Explorer faceExplorer(loftShape, TopAbs_FACE);

    if (!faceExplorer.More()) {
        throw CTiglError("Invalid fuselage segment shape generated");
    }

    GProp_GProps AreaSystem;

    // The first face is the outer hull. We ignore symmetry planes and the front / back caps
    BRepGProp::SurfaceProperties(faceExplorer.Current(), AreaSystem);
    mySurfaceArea = AreaSystem.Mass();
        
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));

    bool hasSymmetryPlane = GetNumberOfEdges(GetEndWire()) > 1;
    SetFaceTraits(loft, hasSymmetryPlane);

    return loft;
}


// Returns the start section UID of this segment
const std::string& CCPACSFuselageSegment::GetStartSectionUID()
{
    return startConnection.GetSectionUID();
}

// Returns the end section UID of this segment
const std::string& CCPACSFuselageSegment::GetEndSectionUID()
{
    return endConnection.GetSectionUID();
}

// Returns the start section index of this segment
int CCPACSFuselageSegment::GetStartSectionIndex()
{
    return startConnection.GetSectionIndex();
}

// Returns the end section index of this segment
int CCPACSFuselageSegment::GetEndSectionIndex()
{
    return endConnection.GetSectionIndex();
}

// Returns the start section element UID of this segment
const std::string& CCPACSFuselageSegment::GetStartSectionElementUID()
{
    return startConnection.GetSectionElementUID();
}

// Returns the end section element UID of this segment
const std::string& CCPACSFuselageSegment::GetEndSectionElementUID()
{
    return endConnection.GetSectionElementUID();
}

// Returns the start section element index of this segment
int CCPACSFuselageSegment::GetStartSectionElementIndex()
{
    return startConnection.GetSectionElementIndex();
}

// Returns the end section element index of this segment
int CCPACSFuselageSegment::GetEndSectionElementIndex()
{
    return endConnection.GetSectionElementIndex();
}

// Returns the start section element index of this segment
CTiglFuselageConnection& CCPACSFuselageSegment::GetStartConnection()
{
    return( startConnection );
}

// Returns the end section element index of this segment
CTiglFuselageConnection& CCPACSFuselageSegment::GetEndConnection()
{
    return( endConnection );
}

// Returns the volume of this segment
double CCPACSFuselageSegment::GetVolume()
{
    // we have to trigger the build of the shape
    GetLoft();

    return( myVolume );
}

// Returns the surface area of this segment
double CCPACSFuselageSegment::GetSurfaceArea()
{
    // we have to trigger the build of the shape
    GetLoft();

    return( mySurfaceArea );
}


// Gets the count of segments connected to the start section of this segment
int CCPACSFuselageSegment::GetStartConnectedSegmentCount()
{
    int count = 0;
    for (int i = 1; i <= GetFuselage().GetSegmentCount(); i++) {
        CCPACSFuselageSegment& nextSegment = GetFuselage().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
            continue;
        }
        if (nextSegment.GetStartSectionUID() == GetStartSectionUID() ||
            nextSegment.GetEndSectionUID() == GetStartSectionUID()) {

            count++;
        }
    }
    return count;
}

// Gets the count of segments connected to the end section of this segment
int CCPACSFuselageSegment::GetEndConnectedSegmentCount()
{
    int count = 0;
    for (int i = 1; i <= GetFuselage().GetSegmentCount(); i++) {
        CCPACSFuselageSegment& nextSegment = GetFuselage().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
            continue;
        }
        if (nextSegment.GetStartSectionUID() == GetEndSectionUID() ||
            nextSegment.GetEndSectionUID()   == GetEndSectionUID()) {

            count++;
        }
    }
    return count;
}

// Gets the index (number) of the n-th segment connected to the start section
// of this segment. n starts at 1.
int CCPACSFuselageSegment::GetStartConnectedSegmentIndex(int n)
{
    if (n < 1 || n > GetStartConnectedSegmentCount()) {
        throw CTiglError("Invalid value for parameter n in CCPACSFuselageSegment::GetStartConnectedSegmentIndex", TIGL_INDEX_ERROR);
    }

    for (int i = 1, count = 0; i <= GetFuselage().GetSegmentCount(); i++) {
        CCPACSFuselageSegment& nextSegment = GetFuselage().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
            continue;
        }
        if (nextSegment.GetStartSectionUID() == GetStartSectionUID() ||
            nextSegment.GetEndSectionUID()   == GetStartSectionUID()) {

            if (++count == n) {
                return nextSegment.GetSegmentIndex();
            }
        }
    }

    throw CTiglError("No connected segment found in CCPACSFuselageSegment::GetStartConnectedSegmentIndex", TIGL_NOT_FOUND);
}

// Gets the index (number) of the n-th segment connected to the end section
// of this segment. n starts at 1.
int CCPACSFuselageSegment::GetEndConnectedSegmentIndex(int n)
{
    if (n < 1 || n > GetEndConnectedSegmentCount()) {
        throw CTiglError("Invalid value for parameter n in CCPACSFuselageSegment::GetEndConnectedSegmentIndex", TIGL_INDEX_ERROR);
    }

    for (int i = 1, count = 0; i <= GetFuselage().GetSegmentCount(); i++) {
        CCPACSFuselageSegment& nextSegment = (CCPACSFuselageSegment &) GetFuselage().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
            continue;
        }
        if (nextSegment.GetStartSectionUID() == GetEndSectionUID() ||
            nextSegment.GetEndSectionUID()   == GetEndSectionUID()) {

            if (++count == n) {
                return nextSegment.GetSegmentIndex();
            }
        }
    }

    throw CTiglError("No connected segment found in CCPACSFuselageSegment::GetEndConnectedSegmentIndex", TIGL_NOT_FOUND);
}

// Gets a point on the fuselage segment in dependence of parameters eta and zeta with
// 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
// profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
// the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
gp_Pnt CCPACSFuselageSegment::GetPoint(double eta, double zeta)
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSFuselageSegment::GetPoint", TIGL_ERROR);
    }

    CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
    CCPACSFuselageProfile& endProfile   = endConnection.GetProfile();

    gp_Pnt startProfilePoint = startProfile.GetPoint(zeta);
    gp_Pnt endProfilePoint   = endProfile.GetPoint(zeta);
    
    startProfilePoint = transformProfilePoint(GetFuselage().GetTransformationMatrix(), startConnection, startProfilePoint);
    endProfilePoint   = transformProfilePoint(GetFuselage().GetTransformationMatrix(), endConnection,   endProfilePoint);

    // Get point on fuselage segment in dependence of eta by linear interpolation
    Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(startProfilePoint, endProfilePoint);
    Standard_Real firstParam = profileLine->FirstParameter();
    Standard_Real lastParam  = profileLine->LastParameter();
    Standard_Real param = (lastParam - firstParam) * eta;
    gp_Pnt profilePoint;
    profileLine->D0(param, profilePoint);

    return profilePoint;
}



// Returns the start profile points as read from TIXI. The points are already transformed.
std::vector<CTiglPoint> CCPACSFuselageSegment::GetRawStartProfilePoints()
{
    CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
    if (startProfile.GetPointList_choice1()) {
        const std::vector<CTiglPoint>& points = startProfile.GetPointList_choice1()->AsVector();
        std::vector<CTiglPoint> pointsTransformed;
        for (std::vector<CTiglPoint>::size_type i = 0; i < points.size(); i++) {
            gp_Pnt pnt = points[i].Get_gp_Pnt();

            pnt = transformProfilePoint(fuselage->GetTransformationMatrix(), startConnection, pnt);

            pointsTransformed.push_back(CTiglPoint(pnt.X(), pnt.Y(), pnt.Z()));
        }
        return pointsTransformed;
    } else
        return std::vector<CTiglPoint>();
}


// Returns the outer profile points as read from TIXI. The points are already transformed.
std::vector<CTiglPoint> CCPACSFuselageSegment::GetRawEndProfilePoints()
{
    CCPACSFuselageProfile& endProfile = endConnection.GetProfile();
    if (endProfile.GetPointList_choice1()) {
        const std::vector<CTiglPoint>& points = endProfile.GetPointList_choice1()->AsVector();
        std::vector<CTiglPoint> pointsTransformed;
        for (std::vector<tigl::CTiglPoint>::size_type i = 0; i < points.size(); i++) {
            gp_Pnt pnt = points[i].Get_gp_Pnt();

            pnt = transformProfilePoint(fuselage->GetTransformationMatrix(), endConnection, pnt);

            pointsTransformed.push_back(CTiglPoint(pnt.X(), pnt.Y(), pnt.Z()));
        }
        return pointsTransformed;
    } else
        return std::vector<CTiglPoint>();
}

gp_Pnt CCPACSFuselageSegment::GetPointOnYPlane(double eta, double ypos, int pointIndex)
{
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    //build cutting plane for intersection
    gp_Pnt p1(-1.0e7, ypos, -1.0e7);
    gp_Pnt p2( 1.0e7, ypos, -1.0e7);
    gp_Pnt p3( 1.0e7, ypos,  1.0e7);
    gp_Pnt p4(-1.0e7, ypos,  1.0e7);

    Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
    Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
    Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
    Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

    TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
    TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
    TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
    TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

    TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
    TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

    // intersection-points
    BRepExtrema_DistShapeShape distSS;
    distSS.LoadS1(intersectionWire);
    distSS.LoadS2(shaft_face);
    distSS.Perform();

    return distSS.PointOnShape1(pointIndex);
}


gp_Pnt CCPACSFuselageSegment::GetPointOnXPlane(double eta, double xpos, int pointIndex)
{
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    //build cutting plane for intersection
    gp_Pnt p1(-1.0e7, -1.0e7, xpos);
    gp_Pnt p2( 1.0e7, -1.0e7, xpos);
    gp_Pnt p3( 1.0e7,  1.0e7, xpos);
    gp_Pnt p4(-1.0e7,  1.0e7, xpos);

    Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
    Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
    Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
    Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

    TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
    TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
    TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
    TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

    TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
    TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

    // intersection-points
    BRepExtrema_DistShapeShape distSS;
    distSS.LoadS1(intersectionWire);
    distSS.LoadS2(shaft_face);
    distSS.Perform();

    return distSS.PointOnShape1(pointIndex);
}


// Gets the wire on the loft at a given eta
TopoDS_Shape CCPACSFuselageSegment::getWireOnLoft(double eta)
{
    const TopoDS_Shape& loft = GetLoft()->Shape();

    TopExp_Explorer faceExplorer(loft, TopAbs_FACE);

    if (!faceExplorer.More()) {
        throw CTiglError("Internal error: invalid topology of fuselage segment shape.", TIGL_ERROR);
    }

    const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

    // we need to extract a iso-v curve
    Standard_Real umin = 0, vmin = 0, umax = 1., vmax = 1.;
    surf->Bounds(umin, umax, vmin, vmax);
    Handle(Geom_Curve) curve = surf->VIso(vmin * (1. - eta) + vmax * eta);

    BRepBuilderAPI_MakeWire wireMaker(BRepBuilderAPI_MakeEdge(curve).Edge());
    if (!curve->IsClosed()) {
        wireMaker.Add(BRepBuilderAPI_MakeEdge(curve->Value(curve->LastParameter()), curve->Value(curve->FirstParameter())));
    }

    return wireMaker.Wire();
}


int CCPACSFuselageSegment::GetNumPointsOnYPlane(double eta, double ypos)
{
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    //build cutting plane for intersection
    gp_Pnt p1(-1.0e7, ypos, -1.0e7);
    gp_Pnt p2( 1.0e7, ypos, -1.0e7);
    gp_Pnt p3( 1.0e7, ypos,  1.0e7);
    gp_Pnt p4(-1.0e7, ypos,  1.0e7);

    Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
    Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
    Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
    Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

    TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
    TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
    TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
    TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

    TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
    TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

    // intersection-points
    BRepExtrema_DistShapeShape distSS;
    distSS.LoadS1(intersectionWire);
    distSS.LoadS2(shaft_face);
    distSS.Perform();

    return distSS.NbSolution();
}


int CCPACSFuselageSegment::GetNumPointsOnXPlane(double eta, double xpos)
{
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    //build cutting plane for intersection
    gp_Pnt p1(-1.0e7, -1.0e7, xpos);
    gp_Pnt p2( 1.0e7, -1.0e7, xpos);
    gp_Pnt p3( 1.0e7,  1.0e7, xpos);
    gp_Pnt p4(-1.0e7,  1.0e7, xpos);

    Handle(Geom_TrimmedCurve) shaft_line1 = GC_MakeSegment(p1,p2);
    Handle(Geom_TrimmedCurve) shaft_line2 = GC_MakeSegment(p2,p3);
    Handle(Geom_TrimmedCurve) shaft_line3 = GC_MakeSegment(p3,p4);
    Handle(Geom_TrimmedCurve) shaft_line4 = GC_MakeSegment(p4,p1);

    TopoDS_Edge shaft_edge1 = BRepBuilderAPI_MakeEdge(shaft_line1);
    TopoDS_Edge shaft_edge2 = BRepBuilderAPI_MakeEdge(shaft_line2);
    TopoDS_Edge shaft_edge3 = BRepBuilderAPI_MakeEdge(shaft_line3);
    TopoDS_Edge shaft_edge4 = BRepBuilderAPI_MakeEdge(shaft_line4);

    TopoDS_Wire shaft_wire = BRepBuilderAPI_MakeWire(shaft_edge1, shaft_edge2, shaft_edge3, shaft_edge4);
    TopoDS_Face shaft_face = BRepBuilderAPI_MakeFace(shaft_wire);

    // intersection-points
    BRepExtrema_DistShapeShape distSS;
    distSS.LoadS1(intersectionWire);
    distSS.LoadS2(shaft_face);
    distSS.Perform();

    return distSS.NbSolution();
}


// Gets a point on the fuselage segment in dependence of an angle alpha (degree).
// The origin of the angle could be set via the parameters x_cs and z_cs,
// but in most cases x_cs and z_cs will be zero get the get center line of the profile.
gp_Pnt CCPACSFuselageSegment::GetPointAngle(double eta, double alpha, double y_cs, double z_cs)
{
    // get eta-y-coordinate
    gp_Pnt tmpPoint = GetPoint(eta, 0.0);

    // get outer wire
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    // compute approximate cross section of fuselage wire
    Bnd_Box boundingBox;
    BRepBndLib::Add(intersectionWire, boundingBox);
    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    double xw = xmax - xmin;
    double yw = ymax - ymin;
    double zw = zmax - zmin;

    double cross_section = max(xw, max(yw, zw));

    // This defines a length of a line intersecting with the fuselage
    double length = cross_section * 2.;
    double angle = alpha/180. * M_PI;
    // build a line
    gp_Pnt initPoint(tmpPoint.X(), y_cs, z_cs);
    gp_Pnt endPoint (tmpPoint.X(), y_cs - length*sin(angle),  z_cs + length*cos(angle));

    BRepBuilderAPI_MakeEdge edge1(initPoint, endPoint);
    TopoDS_Shape lineShape = edge1.Shape();

    // calculate intersection point
    BRepExtrema_DistShapeShape distSS;
    distSS.LoadS1(intersectionWire);
    distSS.LoadS2(lineShape);
    distSS.Perform();

    int numberOfIntersections = distSS.NbSolution();
    if (numberOfIntersections > 1) {
        gp_Pnt p1 = distSS.PointOnShape1(1);
        for (int iSol = 1; iSol <= distSS.NbSolution(); ++iSol){
            if (p1.Distance(distSS.PointOnShape1(1)) > 1e-7){
                LOG(WARNING) << "Multiple intersection points found in CCPACSFuselageSegment::GetPointAngle. Only the first is returned." << std::endl;
                break;
            }
        }
        return p1;
    }
    else if (numberOfIntersections <= 0) {
        LOG(ERROR) << "No solution found in CCPACSFuselageSegment::GetPointAngle. Return (0,0,0) instead." << std::endl;
        return gp_Pnt(0., 0., 0.);
    }
    else {
        return distSS.PointOnShape1(1);
    }
}



// Returns the circumference if the segment at a given eta
double CCPACSFuselageSegment::GetCircumference(const double eta)
{
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    GProp_GProps System;
    BRepGProp::LinearProperties(intersectionWire,System);
    myWireLength = System.Mass();
    return myWireLength;
}

// get guide curve for given UID
const CCPACSGuideCurve& CCPACSFuselageSegment::GetGuideCurve(std::string UID)
{
    return m_guideCurves->GetGuideCurve(UID);
}

// check if guide curve with a given UID exists
bool CCPACSFuselageSegment::GuideCurveExists(std::string UID)
{
    return m_guideCurves->GuideCurveExists(UID);
}

// Creates all guide curves
TopTools_SequenceOfShape& CCPACSFuselageSegment::BuildGuideCurves()
{
    guideCurveWires.Clear();
    if (m_guideCurves) {

        // get start and end profile
        CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
        CCPACSFuselageProfile& endProfile   = endConnection.GetProfile();

        // get wire and close it if the profile is not mirror symmetric
        TopoDS_Wire startWire = startProfile.GetWire(!startProfile.GetMirrorSymmetry());
        TopoDS_Wire endWire   = endProfile.GetWire(!endProfile.GetMirrorSymmetry());

        // get profile wires in world coordinates
        startWire = transformProfileWire(GetFuselage().GetTransformationMatrix(), startConnection, startWire);
        endWire = transformProfileWire(GetFuselage().GetTransformationMatrix(), endConnection, endWire);

        // put wires into container for guide curve algo
        TopTools_SequenceOfShape startWireContainer;
        startWireContainer.Append(startWire);
        TopTools_SequenceOfShape endWireContainer;
        endWireContainer.Append(endWire);

        // get chord lengths for inner profile in word coordinates
        TopoDS_Wire innerChordLineWire = transformProfileWire(GetFuselage().GetTransformationMatrix(), startConnection, startProfile.GetDiameterWire());
        TopoDS_Wire outerChordLineWire = transformProfileWire(GetFuselage().GetTransformationMatrix(), endConnection, endProfile.GetDiameterWire());
        double innerScale = GetWireLength(innerChordLineWire);
        double outerScale = GetWireLength(outerChordLineWire);

        // loop through all guide curves and construct the corresponding wires
        int nGuideCurves = m_guideCurves ? m_guideCurves->GetGuideCurveCount() : 0;
        for (int i=0; i!=nGuideCurves; i++) {
            // get guide curve
            const CCPACSGuideCurve& guideCurve = m_guideCurves->GetGuideCurve(i+1);
            double fromRelativeCircumference;
            // check if fromRelativeCircumference is given in the current guide curve
            if (guideCurve.GetFromRelativeCircumference_choice2()) {
                fromRelativeCircumference = *guideCurve.GetFromRelativeCircumference_choice2();
            }
            // otherwise get relative circumference from neighboring segment guide curve
            else {
                // get neighboring guide curve UID
                std::string neighborGuideCurveUID = *guideCurve.GetFromGuideCurveUID_choice1();
                // get neighboring guide curve
                const CCPACSGuideCurve& neighborGuideCurve = fuselage->GetGuideCurve(neighborGuideCurveUID);
                // get relative circumference from neighboring guide curve
                fromRelativeCircumference = neighborGuideCurve.GetToRelativeCircumference();
            }
            // get relative circumference of outer profile
            double toRelativeCircumference = guideCurve.GetToRelativeCircumference();
            // get guide curve profile UID
            std::string guideCurveProfileUID = guideCurve.GetGuideCurveProfileUID();
            // get relative circumference of inner profile

            // get guide curve profile
            CCPACSConfiguration& config = fuselage->GetConfiguration();
            CCPACSGuideCurveProfile& guideCurveProfile = config.GetGuideCurveProfile(guideCurveProfileUID);

            // construct guide curve algorithm
            TopoDS_Wire guideCurveWire = CCPACSGuideCurveAlgo<CCPACSFuselageProfileGetPointAlgo> (startWireContainer, endWireContainer, fromRelativeCircumference, toRelativeCircumference, innerScale, outerScale, guideCurveProfile);
            guideCurveWires.Append(guideCurveWire);
        }

        // return container for guide curve wires
        return guideCurveWires;
    }
    else {
        return guideCurveWires;
    }
}

} // end namespace tigl
