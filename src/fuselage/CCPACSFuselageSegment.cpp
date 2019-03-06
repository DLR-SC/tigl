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
* @brief  Implementation of CPACS fuselage segment handling routines.
*/
#include <iostream>
#include <cmath>

#include "CCPACSFuselageSegment.h"
#include "CTiglFuselageSegmentGuidecurveBuilder.h"
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
#include "CTiglMakeLoft.h"
#include "CTiglPatchShell.h"
#include "Debugging.h"

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
// #include "BRepMesh.hxx"
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
#include <TopExp.hxx>
#include "TopTools_IndexedMapOfShape.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "CTiglTopoAlgorithms.h"

namespace
{
    gp_Pnt transformProfilePoint(const tigl::CTiglTransformation& fuselTransform, const tigl::CTiglFuselageConnection& connection, const gp_Pnt& pointOnProfile)
    {
        // Do section element transformation on points
        tigl::CTiglTransformation trafo = connection.GetSectionElementTransformation();

        // Do section transformations
        trafo.PreMultiply(connection.GetSectionTransformation());

        // Do positioning transformations
        boost::optional<tigl::CTiglTransformation> connectionTransform = connection.GetPositioningTransformation();
        if (connectionTransform)
            trafo.PreMultiply(*connectionTransform);

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
        boost::optional<tigl::CTiglTransformation> connectionTransform = connection.GetPositioningTransformation();
        if (connectionTransform)
            trafo.PreMultiply(*connectionTransform);

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
    , CTiglAbstractSegment<CCPACSFuselageSegment>(parent->GetSegments(), parent->GetParent())
    , fuselage(parent->GetParent())
    , surfacePropertiesCache(*this, &CCPACSFuselageSegment::UpdateSurfaceProperties)
    , surfaceCache(*this, &CCPACSFuselageSegment::BuildSurfaces)
    , m_guideCurveBuilder(make_unique<CTiglFuselageSegmentGuidecurveBuilder>(*this))
{
    Cleanup();
}

// Destructor
CCPACSFuselageSegment::~CCPACSFuselageSegment()
{
    Cleanup();
}

// Cleanup routine
void CCPACSFuselageSegment::Cleanup()
{
    m_name = "";
    surfacePropertiesCache.clear();
    _continuity    = C2;
    CTiglAbstractGeometricComponent::Reset();
}

void CCPACSFuselageSegment::InvalidateImpl(const boost::optional<std::string>& source) const
{
    CTiglAbstractSegment<CCPACSFuselageSegment>::Reset();
    // forward invalidation to parent fuselage
    const auto* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate(GetUID());
    }
}

// Read CPACS segment elements
void CCPACSFuselageSegment::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& segmentXPath)
{
    Cleanup();
    generated::CPACSFuselageSegment::ReadCPACS(tixiHandle, segmentXPath);

    // trigger creation of connections
    SetFromElementUID(m_fromElementUID);
    SetToElementUID(m_toElementUID);

    // TODO: continuity does not exist in CPACS spec

    if (m_guideCurves) {
        for (int iguide = 1; iguide <= m_guideCurves->GetGuideCurveCount(); ++iguide) {
            m_guideCurves->GetGuideCurve(iguide).SetGuideCurveBuilder(*m_guideCurveBuilder);
        }
    }

    Invalidate();
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
TopoDS_Wire CCPACSFuselageSegment::GetStartWire(TiglCoordinateSystem referenceCS) const
{
    const CCPACSFuselageProfile& startProfile = startConnection.GetProfile();
    TopoDS_Wire startWire = startProfile.GetWire(true);

    CTiglTransformation identity;
    switch (referenceCS) {
    case FUSELAGE_COORDINATE_SYSTEM:
        return transformProfileWire(identity, startConnection, startWire);
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(
            transformFuselageProfileGeometry(GetFuselage().GetTransformationMatrix(), startConnection, startWire));
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSFuselageSegment::GetStartWire");
    }
}

// helper function to get the wire of the end section
TopoDS_Wire CCPACSFuselageSegment::GetEndWire(TiglCoordinateSystem referenceCS) const
{
    const CCPACSFuselageProfile& endProfile = endConnection.GetProfile();
    TopoDS_Wire endWire = endProfile.GetWire(true);

    CTiglTransformation identity;
    switch (referenceCS) {
    case FUSELAGE_COORDINATE_SYSTEM:
        return transformProfileWire(identity, endConnection, endWire);
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(
            transformFuselageProfileGeometry(GetFuselage().GetTransformationMatrix(), endConnection, endWire));
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSFuselageSegment::GetEndWire");
    }
}

// get short name for loft
std::string CCPACSFuselageSegment::GetShortShapeName() const
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

void CCPACSFuselageSegment::SetFaceTraits (PNamedShape loft) const
{
    int nFaces = GetNumberOfFaces(loft->Shape());
    bool hasSymmetryPlane = GetNumberOfEdges(GetEndWire()) > 1;

    std::vector<std::string> names;
    names.push_back(loft->Name());
    names.push_back("symmetry");
    names.push_back("Front");
    names.push_back("Rear");


    int facesPerSegment = GetNumberOfLoftFaces();
    int remainingFaces = nFaces - facesPerSegment;
    if (facesPerSegment == 0 || remainingFaces < 0 || remainingFaces > 2) {
        LOG(WARNING) << "Fuselage segment faces cannot be names properly (maybe due to Guide Curves?)";
        return;
    }

    int iFaceTotal = 0;
    int nSymmetryFaces = (int) hasSymmetryPlane;
    for (int iFace = 0; iFace < facesPerSegment - nSymmetryFaces; ++iFace) {
        loft->FaceTraits(iFaceTotal++).SetName(names[0].c_str());
    }
    for (int iFace = 0; iFace < nSymmetryFaces; ++iFace) {
        loft->FaceTraits(iFaceTotal++).SetName(names[1].c_str());
    }

    // set the caps
    int iFace = 2;
    for (;iFaceTotal < nFaces; ++iFaceTotal) {
        loft->FaceTraits(iFaceTotal).SetName(names[iFace++].c_str());
    }
}

// Builds the loft between the two segment sections
PNamedShape CCPACSFuselageSegment::BuildLoft() const
{
    // Build loft
    TopoDS_Shape loftShape;
    if( loftLinearly ) {
        CTiglMakeLoft lofter;
        lofter.addProfiles(GetStartWire());
        lofter.addProfiles(GetEndWire());
        loftShape = lofter.Shape();
    }
    else {
        // retrieve segment loft as subshape of the fuselage loft
        CCPACSFuselage& fuselage = GetFuselage();
        PNamedShape fuselageLoft = fuselage.GetLoft();

        TopoDS_Shell loftShell;
        BRep_Builder BB;
        BB.MakeShell(loftShell);

        // get subshapes of the fuselage
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(fuselageLoft->Shape(), TopAbs_FACE, faceMap);

        //determine the number of faces per segment
        int nFacesPerSegment = GetNumberOfLoftFaces();

        const int mySegmentIndex = GetSegmentIndex();
        for (int i = 1; i <= nFacesPerSegment; ++i) {
            BB.Add(loftShell, TopoDS::Face(faceMap(nFacesPerSegment*(mySegmentIndex-1) + i)));
        }

        //close the shell with sidecaps and make them a solid
        TopoDS_Wire startWire = GetStartWire();
        TopoDS_Wire endWire  = GetEndWire();

        CTiglPatchShell patcher(loftShell);
        patcher.AddSideCap(startWire);
        patcher.AddSideCap(endWire);
        loftShape = patcher.PatchedShape();
    }

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));

    SetFaceTraits(loft);

    return loft;
}

void CCPACSFuselageSegment::UpdateSurfaceProperties(SurfacePropertiesCache& cache) const
{
    const TopoDS_Shape loftShape = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(loftShape, System);
    cache.myVolume = System.Mass();

    // Calculate surface area
    TopExp_Explorer faceExplorer(loftShape, TopAbs_FACE);

    if (!faceExplorer.More()) {
        throw CTiglError("Invalid fuselage segment shape generated");
    }

    GProp_GProps AreaSystem;

    // The first face is the outer hull. We ignore symmetry planes and the front / back caps
    BRepGProp::SurfaceProperties(faceExplorer.Current(), AreaSystem);
    cache.mySurfaceArea = AreaSystem.Mass();
}

void CCPACSFuselageSegment::BuildSurfaces(SurfaceCache& cache) const
{
    TopTools_IndexedMapOfShape faceMap;
    TopoDS_Shape s = GetFacesByName(GetLoft(), GetUID());
    TopExp::MapShapes(s, TopAbs_FACE, faceMap);

    std::vector<double> parameters;
    std::vector<Handle(Geom_BoundedSurface)> bsurfaces;

    for (int idx = 1; idx <= faceMap.Extent(); ++idx) {
        // get uv coordinates and 3d point on the face
        TopoDS_Face face = TopoDS::Face(faceMap(idx));
        Handle(Geom_BoundedSurface) surface = Handle(Geom_BoundedSurface)::DownCast(BRep_Tool::Surface(face));
        bsurfaces.push_back(surface);
    }

    if ( GetGuideCurves() ) {
        const CCPACSGuideCurves& segmentCurves = *GetGuideCurves();
        parameters = segmentCurves.GetRelativeCircumferenceParameters();

    }
    else {
        double umax = 0.;
        for (const auto& surf : bsurfaces) {
            double umin, vmin, vmax;
            surf->Bounds(umin, umax, vmin, vmax);
            parameters.push_back(umin);
        }
        parameters.push_back(umax);
    }

    cache.surface = CTiglCompoundSurface(bsurfaces, parameters);
}

// Returns the start section UID of this segment
const std::string& CCPACSFuselageSegment::GetStartSectionUID() const
{
    return startConnection.GetSectionUID();
}

// Returns the end section UID of this segment
const std::string& CCPACSFuselageSegment::GetEndSectionUID() const
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
    return surfacePropertiesCache->myVolume;
}

// Returns the surface area of this segment
double CCPACSFuselageSegment::GetSurfaceArea()
{
    return surfacePropertiesCache->mySurfaceArea;
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
gp_Pnt CCPACSFuselageSegment::GetPoint(double eta, double zeta, TiglGetPointBehavior behavior)
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSFuselageSegment::GetPoint", TIGL_ERROR);
    }
    if (zeta < 0.0 || zeta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSFuselageSegment::GetPoint", TIGL_ERROR);
    }

    gp_Pnt profilePoint;
    if ( behavior == onLinearLoft ) {

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
        profileLine->D0(param, profilePoint);
    }
    else if ( behavior == asParameterOnSurface) {
        return surfaceCache->surface.Value(zeta, eta);
    }
    else {
        throw CTiglError("CCPACSFuselageSegment::GetPoint: Unknown TiglGetPointBehavior passed as argument.", TIGL_INDEX_ERROR);
    }

    return profilePoint;
}

TIGL_EXPORT gp_Pnt CCPACSFuselageSegment::GetTransformedProfileOriginStart() const
{
    return transformProfilePoint(GetFuselage().GetTransformationMatrix(), startConnection, gp_Pnt(0., 0., 0.));
}

TIGL_EXPORT gp_Pnt CCPACSFuselageSegment::GetTransformedProfileOriginEnd() const
{
    return transformProfilePoint(GetFuselage().GetTransformationMatrix(), endConnection, gp_Pnt(0., 0., 0.));
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

    TopoDS_Shape s = GetFacesByName(GetLoft(), GetUID());

    BRepBuilderAPI_MakeWire wireMaker;
    for(TopExp_Explorer faceExplorer(s, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {

        const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
        Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

        // we need to extract a iso-v curve
        Standard_Real umin = 0, vmin = 0, umax = 1., vmax = 1.;
        surf->Bounds(umin, umax, vmin, vmax);
        Handle(Geom_Curve) curve = surf->VIso(vmin * (1. - eta) + vmax * eta);
        wireMaker.Add(BRepBuilderAPI_MakeEdge(curve).Edge());
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
// The origin of the angle could be set via the parameters y_cs and z_cs.
// y_cs and z_cs are assumed to be offsets from the cross section center. Set absolute = true
// if the values should be interpreted as absolute coordinates
gp_Pnt CCPACSFuselageSegment::GetPointAngle(double eta, double alpha, double y_cs, double z_cs, bool absolute )
{
    //CAUTION: This functions assumes the fuselage to be aligned along the x-axis
    // and xsi=0 to be at the top of the fuselage (in z-direction)

    // get outer wire
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    if ( !absolute ) {
        // get cross section center
        gp_Pnt csc = GetCenterOfMass(intersectionWire);
        y_cs += csc.Y();
        z_cs += csc.Z();
    }

    // compute approximate cross section of fuselage wire
    Bnd_Box boundingBox;
    BRepBndLib::Add(intersectionWire, boundingBox);
    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    double yw = ymax - ymin;
    double zw = zmax - zmin;
    double length = 2*std::max(yw, zw);

    //create a rectangular face
    double angle = alpha/180. * M_PI;
    gp_Pnt p1(xmin, y_cs, z_cs);
    gp_Pnt p2(xmax, y_cs, z_cs);
    gp_Pnt p3(xmax, y_cs - length*sin(angle), z_cs + length*cos(angle) );
    gp_Pnt p4(xmin, y_cs - length*sin(angle), z_cs + length*cos(angle) );
    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
    TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(p3, p4);
    TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(p4, p1);
    TopoDS_Wire rectangleWire = BRepBuilderAPI_MakeWire(edge1, edge2, edge3, edge4);
    BRepBuilderAPI_MakeFace FaceMaker(rectangleWire);

    gp_Pnt result;
    if (!GetIntersectionPoint( FaceMaker.Face(), TopoDS::Wire(intersectionWire), result)) {
        throw CTiglError("Cannot compute fuselage point at angle");
    }
    return result;
}



// Returns the circumference if the segment at a given eta
double CCPACSFuselageSegment::GetCircumference(const double eta)
{
    TopoDS_Shape intersectionWire = getWireOnLoft(eta);

    GProp_GProps System;
    BRepGProp::LinearProperties(intersectionWire,System);
    return System.Mass();
}

// Returns the number of faces in the loft. This depends on the number of guide curves as well as if the fuselage has a symmetry plane.
TIGL_EXPORT int CCPACSFuselageSegment::GetNumberOfLoftFaces() const
{

    // no guide curves, therefore we either have one or two faces, depending on the symmetry plane
    bool hasSymmetryPlane = GetNumberOfEdges(GetEndWire()) > 1;
    int nfaces = GetNumberOfFaces(GetFuselage().GetLoft()->Shape());
    int nSegments = GetFuselage().GetSegmentCount();

    if (!CTiglTopoAlgorithms::IsDegenerated(GetFuselage().GetSegment(1).GetStartWire())) {
          nfaces-=1;
    }
    if (!CTiglTopoAlgorithms::IsDegenerated(GetFuselage().GetSegment(GetFuselage().GetSegmentCount()).GetEndWire())) {
          nfaces-=1;
    }

    int facesPerSegment = nfaces / nSegments;
    return facesPerSegment;
}
} // end namespace tigl
