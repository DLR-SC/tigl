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
* @brief  Implementation of CPACS wing segment handling routines.
*/

#include <cmath>
#include <iostream>
#include <string>
#include <cassert>
#include <cfloat>

#include "CCPACSWingSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegments.h"
#include "CCPACSWingProfiles.h"
#include "CCPACSGuideCurveProfiles.h"
#include "CCPACSGuideCurveAlgo.h"
#include "CCPACSWingProfileGetPointAlgo.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "tigl_config.h"
#include "math/tiglmathfunctions.h"
#include "TixiSaveExt.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shell.hxx"
#include "TopoDS_Wire.hxx"
#include "gp_Trsf.hxx"
#include "gp_Lin.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GC_MakeSegment.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "GeomAPI_IntCS.hxx"
#include "Geom_Surface.hxx"
#include "Geom_Line.hxx"
#include "gce_MakeLin.hxx"
#include "IntCurvesFace_Intersector.hxx"
#include "Precision.hxx"
#include "TopLoc_Location.hxx"
#include "Poly_Triangulation.hxx"
#include "Poly_Array1OfTriangle.hxx"
#include "BRep_Tool.hxx"
#include "BRep_Builder.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepMesh.hxx"
#include "BRepTools.hxx"
#include "BRepLib.hxx"
#include "BRepBndLib.hxx"
#include "BRepGProp.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "Bnd_Box.hxx"
#include "BRepLib_FindSurface.hxx"
#include "ShapeAnalysis_Surface.hxx"
#include "GProp_GProps.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "GeomAdaptor_Surface.hxx"
#include "GC_MakeLine.hxx"
#include "BRepTools_WireExplorer.hxx"

#include "Geom_BSplineCurve.hxx"
#include "GeomAPI_PointsToBSpline.hxx"
#include "GeomAdaptor_HCurve.hxx"
#include "GeomFill.hxx"
#include "GeomFill_SimpleBound.hxx"
#include "GeomFill_BSplineCurves.hxx"
#include "GeomFill_FillingStyle.hxx"
#include "Geom_BSplineSurface.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "BRepIntCurveSurface_Inter.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "BRepTools.hxx"
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>

namespace tigl
{

namespace
{
    gp_Pnt transformProfilePoint(const tigl::CTiglTransformation& wingTransform, const tigl::CCPACSWingConnection& connection, const gp_Pnt& pointOnProfile)
    {
        gp_Pnt transformedPoint(pointOnProfile);

        // Do section element transformation on points
        CTiglTransformation trafo = connection.GetSectionElementTransformation();

        // Do section transformations
        trafo.PreMultiply(connection.GetSectionTransformation());

        // Do positioning transformations
        trafo.PreMultiply(connection.GetPositioningTransformation());

        trafo.PreMultiply(wingTransform);

        transformedPoint = trafo.Transform(transformedPoint);

        return transformedPoint;
    }

    TopoDS_Shape transformProfileWire(const tigl::CTiglTransformation& wingTransform, const tigl::CCPACSWingConnection& connection, const TopoDS_Shape& wire)
    {
        TopoDS_Shape transformedWire(wire);

        // Do section element transformation on points
        CTiglTransformation trafo = connection.GetSectionElementTransformation();

        // Do section transformations
        trafo.PreMultiply(connection.GetSectionTransformation());

        // Do positioning transformations
        trafo.PreMultiply(connection.GetPositioningTransformation());

        trafo.PreMultiply(wingTransform);

        transformedWire = trafo.Transform(transformedWire);

        return transformedWire;
    }

    // Set the face traits
    void SetFaceTraits (PNamedShape loft) 
    { 
        // designated names of the faces
        std::vector<std::string> names(5);
        names[0]="Bottom";
        names[1]="Top";
        names[2]="TrailingEdge";
        names[3]="Inside";
        names[4]="Outside";

        // map of faces
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(loft->Shape(),   TopAbs_FACE, map);

        // check if number of faces is correct (only valid for ruled surfaces lofts)
        if (map.Extent() != 5 && map.Extent() != 4) {
            LOG(ERROR) << "CCPACSWingSegment: Unable to determine face names in ruled surface loft";
            return;
        }
        // remove trailing edge name if there is no trailing edge
        if (map.Extent() == 4) {
            names.erase(names.begin()+2);
        }
        // set face trait names
        for (int i = 0; i < map.Extent(); i++) {
            CFaceTraits traits = loft->GetFaceTraits(i);
            traits.SetName(names[i].c_str());
            loft->SetFaceTraits(i, traits);
        }
    }
    
    /**
     * @brief getFaceTrimmingEdge Creates an edge in parameter space to trim a face
     * @return 
     */
    TopoDS_Edge getFaceTrimmingEdge(const TopoDS_Face& face, double ustart, double vstart, double uend, double vend)
    {
        Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
        Handle(Geom2d_TrimmedCurve) line = GCE2d_MakeSegment(gp_Pnt2d(ustart,vstart), gp_Pnt2d(uend,vend));
    
        BRepBuilderAPI_MakeEdge edgemaker(line, surf);
        TopoDS_Edge edge =  edgemaker.Edge();
        
        // this here is really important
        BRepLib::BuildCurves3d(edge);
        return edge;
    }
}

CCPACSWingSegment::CCPACSWingSegment(CCPACSWingSegments* parent)
    : generated::CPACSWingSegment(parent)
    , CTiglAbstractSegment(parent->GetSegmentCount() + 1) // TODO: this is a hack, as we depend on the implementation of the vector reader in generated::CCPACSWingSegments::ReadCPACS() but the current CodeGen does not support passing indices into ctors
    , wing(parent->GetParent<CCPACSWing>()) {
    Cleanup();
}

// Constructor
CCPACSWingSegment::CCPACSWingSegment(CCPACSWing* aWing, int aSegmentIndex)
    : generated::CPACSWingSegment(&aWing->GetSegments())
    , CTiglAbstractSegment(aSegmentIndex)
    , wing(aWing)
{
    Cleanup();
}

// Destructor
CCPACSWingSegment::~CCPACSWingSegment()
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWingSegment::Invalidate()
{
    CTiglAbstractSegment::Invalidate();
    surfaceCache.valid = false;
    surfaceCache.chordsurfaceValid = false;
    guideCurveWires.Clear();
}

// Cleanup routine
void CCPACSWingSegment::Cleanup()
{
    m_name = "";
    m_description = "";
    surfaceCache.upperShape.Nullify();
    surfaceCache.lowerShape.Nullify();
    surfaceCache.trailingEdgeShape.Nullify();
    surfaceCache.valid = false;
    surfaceCache.chordsurfaceValid = false;
    CTiglAbstractSegment::Cleanup();
}

// Update internal segment data
void CCPACSWingSegment::Update()
{
    Invalidate();
}

// Read CPACS segment elements
void CCPACSWingSegment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
{
    Cleanup();
    generated::CPACSWingSegment::ReadCPACS(tixiHandle, segmentXPath);

    GetWing().GetConfiguration().GetUIDManager().AddUID(m_uID, this);

    innerConnection = CCPACSWingConnection(m_fromElementUID, this);
    outerConnection = CCPACSWingConnection(m_toElementUID, this);

    // check that the profiles are consistent
    if (innerConnection.GetProfile().HasBluntTE() !=
        outerConnection.GetProfile().HasBluntTE()) {

        throw CTiglError("The wing profiles " + innerConnection.GetProfile().GetUID() +
                         " and " + outerConnection.GetProfile().GetUID() +
                         " in segment " + m_uID + " are not consistent. "
                         "All profiles must either have a sharp or a blunt trailing edge. "
                         "Mixing different profile types is not allowed.");
    }

    Update();
}

const std::string& CCPACSWingSegment::GetUID() const {
    return generated::CPACSWingSegment::GetUID();
}

void CCPACSWingSegment::SetUID(const std::string& uid) {
    generated::CPACSWingSegment::SetUID(uid);
}

// Returns the wing this segment belongs to
CCPACSWing& CCPACSWingSegment::GetWing() const
{
    return *wing;
}

// helper function to get the inner transformed chord line wire
TopoDS_Wire CCPACSWingSegment::GetInnerWire(TiglCoordinateSystem referenceCS) const
{
    CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    TopoDS_Wire w;
    
    /*
    * The loft algorithm with guide curves does not like splitted
    * wing profiles, we have to give him the unsplitted one.
    * In all other cases, we need the splitted wire to distiguish
    * upper und lower wing surface
    */
#ifdef LOFTALGO_FOUND
    if (guideCurves.GetGuideCurveCount() > 0) {
        w = innerProfile.GetWire();
    }
    else {
        w = innerProfile.GetSplitWire();
    }
#else
    w = innerProfile.GetSplitWire();
#endif

        CTiglTransformation identity;
    
    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(identity, innerConnection, w));
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(GetWing().GetTransformation(), innerConnection, w));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetInnerWire");
    }
}

// helper function to get the outer transformed chord line wire
TopoDS_Wire CCPACSWingSegment::GetOuterWire(TiglCoordinateSystem referenceCS) const
{
    CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
    TopoDS_Wire w;
    
    /*
    * The loft algorithm with guide curves does not like splitted
    * wing profiles, we have to give him the unsplitted one.
    * In all other cases, we need the splitted wire to distiguish
    * upper und lower wing surface
    */
#ifdef LOFTALGO_FOUND
    if (guideCurves.GetGuideCurveCount() > 0) {
        w = outerProfile.GetWire();
    }
    else {
        w = outerProfile.GetSplitWire();
    }
#else
    w = outerProfile.GetSplitWire();
#endif

        CTiglTransformation identity;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(identity, outerConnection, w));
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(GetWing().GetTransformation(), outerConnection, w));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetOuterWire");
    }
}

// Getter for inner wire of opened profile (containing trailing edge)
TopoDS_Wire CCPACSWingSegment::GetInnerWireOpened(TiglCoordinateSystem referenceCS) const
{
    CCPACSWingProfile& innerProfile = innerConnection.GetProfile();

        CTiglTransformation identity;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(identity, innerConnection, innerProfile.GetWireOpened()));
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(GetWing().GetTransformation(), innerConnection, innerProfile.GetWireOpened()));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetInnerWireOpened");
    }
}

// Getter for outer wire of opened profile (containing trailing edge)
TopoDS_Wire CCPACSWingSegment::GetOuterWireOpened(TiglCoordinateSystem referenceCS) const
{
    CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

        CTiglTransformation identity;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(identity, outerConnection, outerProfile.GetWireOpened()));
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformProfileWire(GetWing().GetTransformation(), outerConnection, outerProfile.GetWireOpened()));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetInnerWireOpened");
    }
}

// helper function to get the inner closing of the wing segment
// using shape generated in MakeSurfaces
TopoDS_Shape CCPACSWingSegment::GetInnerClosure(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Wire wire = GetInnerWire(referenceCS);
    return BRepBuilderAPI_MakeFace(wire).Face();
}

// helper function to get the inner closing of the wing segment
// using shape generated in MakeSurfaces
TopoDS_Shape CCPACSWingSegment::GetOuterClosure(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Wire wire = GetOuterWire(referenceCS);
    return BRepBuilderAPI_MakeFace(wire).Face();
}

// get short name for loft
std::string CCPACSWingSegment::GetShortShapeName () 
{
    unsigned int windex = 0;
    unsigned int wsindex = 0;
    for (int i = 1; i <= wing->GetConfiguration().GetWingCount(); ++i) {
        tigl::CCPACSWing& w = wing->GetConfiguration().GetWing(i);
        if (wing->GetUID() == w.GetUID()) {
            windex = i;
            for (int j = 1; j <= w.GetSegmentCount(); j++) {
                tigl::CTiglAbstractSegment& ws = w.GetSegment(j);
                if (GetUID() == ws.GetUID()) {
                    wsindex = j;
                    std::stringstream shortName;
                    shortName << "W" << windex << "S" << wsindex;
                    return shortName.str();
                }
            }
        }
    }
    return "UNKNOWN";
}

// Builds the loft between the two segment sections
// build loft out of faces (for compatibility with component segmen loft)
PNamedShape CCPACSWingSegment::BuildLoft()
{
    TopoDS_Wire innerWire = GetInnerWire();
    TopoDS_Wire outerWire = GetOuterWire();

    // Build loft
    //BRepOffsetAPI_ThruSections generator(Standard_False, Standard_False, Precision::Confusion());
    BRepOffsetAPI_ThruSections generator(/* is solid (else shell) */ Standard_True, /* ruled (else smoothed out) */ Standard_False, Precision::Confusion());
    generator.AddWire(innerWire);
    generator.AddWire(outerWire);
    generator.CheckCompatibility(/* check (defaults to true) */ Standard_False);
    generator.Build();
    TopoDS_Shape loftShape = generator.Shape();

    Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
    sfs->Init ( loftShape );
    sfs->Perform();
    loftShape = sfs->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(loftShape, System);
    myVolume = System.Mass();

    // Set Names
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft (new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft);
    return loft;
}

// Gets the upper point in relative wing coordinates for a given eta and xsi
gp_Pnt CCPACSWingSegment::GetUpperPoint(double eta, double xsi) const
{
    return GetPoint(eta, xsi, true);
}

// Gets the lower point in relative wing coordinates for a given eta and xsi
gp_Pnt CCPACSWingSegment::GetLowerPoint(double eta, double xsi) const
{
    return GetPoint(eta, xsi, false);
}

// Returns the inner section UID of this segment
const std::string& CCPACSWingSegment::GetInnerSectionUID() const
{
    return innerConnection.GetSectionUID();
}

// Returns the outer section UID of this segment
const std::string& CCPACSWingSegment::GetOuterSectionUID() const
{
    return outerConnection.GetSectionUID();
}

// Returns the inner section element UID of this segment
const std::string& CCPACSWingSegment::GetInnerSectionElementUID() const
{
    return innerConnection.GetSectionElementUID();
}

// Returns the outer section element UID of this segment
const std::string& CCPACSWingSegment::GetOuterSectionElementUID() const
{
    return outerConnection.GetSectionElementUID();
}

// Returns the inner section index of this segment
int CCPACSWingSegment::GetInnerSectionIndex() const
{
    return innerConnection.GetSectionIndex();
}

// Returns the outer section index of this segment
int CCPACSWingSegment::GetOuterSectionIndex() const
{
    return outerConnection.GetSectionIndex();
}

// Returns the inner section element index of this segment
int CCPACSWingSegment::GetInnerSectionElementIndex() const
{
    return innerConnection.GetSectionElementIndex();
}

// Returns the outer section element index of this segment
int CCPACSWingSegment::GetOuterSectionElementIndex() const
{
    return outerConnection.GetSectionElementIndex();
}

// Returns the start section element index of this segment
CCPACSWingConnection& CCPACSWingSegment::GetInnerConnection()
{
    return( innerConnection );
}

// Returns the end section element index of this segment
CCPACSWingConnection& CCPACSWingSegment::GetOuterConnection()
{
    return( outerConnection );
}

// Returns the volume of this segment
double CCPACSWingSegment::GetVolume()
{
    Update();
    return( myVolume );
}

// Returns the surface area of this segment
double CCPACSWingSegment::GetSurfaceArea() const
{
    MakeSurfaces();
    return(surfaceCache.mySurfaceArea);
}

void CCPACSWingSegment::etaXsiToUV(bool isFromUpper, double eta, double xsi, double& u, double& v) const
{
    gp_Pnt pnt = GetPoint(eta,xsi, isFromUpper);

    Handle(Geom_Surface) surf;
    if (isFromUpper) {
        surf = surfaceCache.upperSurface;
    }
    else {
        surf = surfaceCache.lowerSurface;
    }

    GeomAPI_ProjectPointOnSurf Proj(pnt, surf);
    if (Proj.NbPoints() > 0) {
        Proj.LowerDistanceParameters(u,v);
    }
    else {
        LOG(WARNING) << "Could not project point on wing segment surface in CCPACSWingSegment::etaXsiToUV";

        double umin, umax, vmin, vmax;
        surf->Bounds(umin,umax,vmin, vmax);
        if (isFromUpper) {
            u = umin*(1-xsi) + umax;
            v = vmin*(1-eta) + vmax;
        }
        else {
            u = umin*xsi + umax*(1-xsi);
            v = vmin*eta + vmax*(1-eta);
        }
    }
}

double CCPACSWingSegment::GetSurfaceArea(bool fromUpper, 
                                         double eta1, double xsi1,
                                         double eta2, double xsi2,
                                         double eta3, double xsi3,
                                         double eta4, double xsi4) const
{
    MakeSurfaces();
    
    TopoDS_Face face;
    if (fromUpper) {
        face = TopoDS::Face(surfaceCache.upperShape);
    }
    else {
        face = TopoDS::Face(surfaceCache.lowerShape);
    }

    // convert eta xsi coordinates to u,v
    double u1, u2, u3, u4, v1, v2, v3, v4;
    etaXsiToUV(fromUpper, eta1, xsi1, u1, v1);
    etaXsiToUV(fromUpper, eta2, xsi2, u2, v2);
    etaXsiToUV(fromUpper, eta3, xsi3, u3, v3);
    etaXsiToUV(fromUpper, eta4, xsi4, u4, v4);
    
    TopoDS_Edge e1 = getFaceTrimmingEdge(face, u1, v1, u2, v2);
    TopoDS_Edge e2 = getFaceTrimmingEdge(face, u2, v2, u3, v3);
    TopoDS_Edge e3 = getFaceTrimmingEdge(face, u3, v3, u4, v4);
    TopoDS_Edge e4 = getFaceTrimmingEdge(face, u4, v4, u1, v1);

    TopoDS_Wire w = BRepBuilderAPI_MakeWire(e1,e2,e3,e4);
    TopoDS_Face f = BRepBuilderAPI_MakeFace(BRep_Tool::Surface(face), w);

    // compute the surface area
    GProp_GProps sprops;
    BRepGProp::SurfaceProperties(f, sprops);
    
    return sprops.Mass();
}

// Gets the count of segments connected to the inner section of this segment // TODO can this be optimized instead of iterating over all segments?
int CCPACSWingSegment::GetInnerConnectedSegmentCount() const
{
    int count = 0;
    for (int i = 1; i <= GetWing().GetSegmentCount(); i++) {
        CCPACSWingSegment& nextSegment = (CCPACSWingSegment&) GetWing().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == mySegmentIndex) {
            continue;
        }
        if (nextSegment.GetInnerSectionUID() == GetInnerSectionUID() ||
            nextSegment.GetOuterSectionUID() == GetInnerSectionUID()) {

            count++;
        }
    }
    return count;
}

// Gets the count of segments connected to the outer section of this segment
int CCPACSWingSegment::GetOuterConnectedSegmentCount() const
{
    int count = 0;
    for (int i = 1; i <= GetWing().GetSegmentCount(); i++) {
        CCPACSWingSegment& nextSegment = (CCPACSWingSegment&) GetWing().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == mySegmentIndex) {
            continue;
        }
        if (nextSegment.GetInnerSectionUID() == GetOuterSectionUID() ||
            nextSegment.GetOuterSectionUID() == GetOuterSectionUID()) {

            count++;
        }
    }
    return count;
}

// Gets the index (number) of the n-th segment connected to the inner section
// of this segment. n starts at 1.
int CCPACSWingSegment::GetInnerConnectedSegmentIndex(int n) const
{
    if (n < 1 || n > GetInnerConnectedSegmentCount()) {
        throw CTiglError("Error: Invalid value for parameter n in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_INDEX_ERROR);
    }

    for (int i = 1, count = 0; i <= GetWing().GetSegmentCount(); i++) {
        CCPACSWingSegment& nextSegment = (CCPACSWingSegment&) GetWing().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == mySegmentIndex) {
            continue;
        }
        if (nextSegment.GetInnerSectionUID() == GetInnerSectionUID() ||
            nextSegment.GetOuterSectionUID() == GetInnerSectionUID()) {

            if (++count == n) {
                return nextSegment.GetSegmentIndex();
            }
        }
    }

    throw CTiglError("Error: No connected segment found in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_NOT_FOUND);
}

// Gets the index (number) of the n-th segment connected to the outer section
// of this segment. n starts at 1.
int CCPACSWingSegment::GetOuterConnectedSegmentIndex(int n) const
{
    if (n < 1 || n > GetOuterConnectedSegmentCount()) {
        throw CTiglError("Error: Invalid value for parameter n in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_INDEX_ERROR);
    }

    for (int i = 1, count = 0; i <= GetWing().GetSegmentCount(); i++) {
        CCPACSWingSegment& nextSegment = (CCPACSWingSegment&) GetWing().GetSegment(i);
        if (nextSegment.GetSegmentIndex() == mySegmentIndex) {
            continue;
        }
        if (nextSegment.GetInnerSectionUID() == GetOuterSectionUID() ||
            nextSegment.GetOuterSectionUID() == GetOuterSectionUID()) {

            if (++count == n) {
                return nextSegment.GetSegmentIndex();
            }
        }
    }

    throw CTiglError("Error: No connected segment found in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_NOT_FOUND);
}

// Returns an upper or lower point on the segment surface in
// dependence of parameters eta and xsi, which range from 0.0 to 1.0.
// For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
// inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
// edge on the outer wing profile. If fromUpper is true, a point
// on the upper surface is returned, otherwise from the lower.
gp_Pnt CCPACSWingSegment::GetPoint(double eta, double xsi, bool fromUpper, TiglCoordinateSystem referenceCS) const
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
    }

    CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

    // Compute points on wing profiles for the given xsi
    gp_Pnt innerProfilePoint;
    gp_Pnt outerProfilePoint;
    if (fromUpper == true) {
        innerProfilePoint = innerProfile.GetUpperPoint(xsi);
        outerProfilePoint = outerProfile.GetUpperPoint(xsi);
    }
    else {
        innerProfilePoint = innerProfile.GetLowerPoint(xsi);
        outerProfilePoint = outerProfile.GetLowerPoint(xsi);
    }

        CTiglTransformation identity;
    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        innerProfilePoint = transformProfilePoint(identity, innerConnection, innerProfilePoint);
        outerProfilePoint = transformProfilePoint(identity, outerConnection, outerProfilePoint);
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        innerProfilePoint = transformProfilePoint(GetWing().GetWingTransformation(), innerConnection, innerProfilePoint);
        outerProfilePoint = transformProfilePoint(GetWing().GetWingTransformation(), outerConnection, outerProfilePoint);
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetPoint");
    }

    // Get point on wing segment in dependence of eta by linear interpolation
    Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(innerProfilePoint, outerProfilePoint);
    Standard_Real firstParam = profileLine->FirstParameter();
    Standard_Real lastParam  = profileLine->LastParameter();
    Standard_Real param = firstParam + (lastParam - firstParam) * eta;
    gp_Pnt profilePoint;
    profileLine->D0(param, profilePoint);

    return profilePoint;
}

gp_Pnt CCPACSWingSegment::GetPointDirection(double eta, double xsi, double dirx, double diry, double dirz, bool fromUpper, double& deviation) const
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
    }

    if (dirx*dirx + diry*diry + dirz*dirz < 1e-10) {
        // invalid direction
        throw CTiglError("Direction must not be a null vector in CCPACSWingSegment::GetPointDirection.", TIGL_MATH_ERROR);
    }

    if (!surfaceCache.valid) {
        MakeSurfaces();
    }

    CTiglPoint tiglPoint;
    ChordFace().translate(eta, xsi, &tiglPoint);

    gp_Dir direction(dirx, diry, dirz);
    gp_Lin line(tiglPoint.Get_gp_Pnt(), direction);
    
    TopoDS_Shape skin;
    if (fromUpper) {
        skin = wing->GetUpperShape();
    }
    else {
        skin = wing->GetLowerShape();
    }
    
    BRepIntCurveSurface_Inter inter;
    double tol = 1e-6;
    inter.Init(skin, line, tol);
    
    if (inter.More()) {
        deviation = 0.;
        return inter.Pnt();
    }
    else {
        // there is not intersection, lets compute the point next to the line
        BRepExtrema_DistShapeShape extrema;
        extrema.LoadS1(BRepBuilderAPI_MakeEdge(line));
        extrema.LoadS2(skin);
        extrema.Perform();
        
        if (!extrema.IsDone()) {
            throw CTiglError("Could not calculate intersection of line with wing shell in CCPACSWingSegment::GetPointDirection", TIGL_NOT_FOUND);
        }
        
        gp_Pnt p1 = extrema.PointOnShape1(1);
        gp_Pnt p2 = extrema.PointOnShape2(1);

        deviation = p1.Distance(p2);
        return p2;
    }
}

gp_Pnt CCPACSWingSegment::GetChordPoint(double eta, double xsi) const
{
    CTiglPoint profilePoint; 
    ChordFace().translate(eta,xsi, &profilePoint);

    return profilePoint.Get_gp_Pnt();
}

gp_Pnt CCPACSWingSegment::GetChordNormal(double eta, double xsi) const
{
    CTiglPoint normal; 
    ChordFace().getNormal(eta,xsi, &normal);

    return normal.Get_gp_Pnt();
}

// TODO: remove this function if favour of Standard GetEta
double CCPACSWingSegment::GetEta(gp_Pnt pnt, double xsi) const
{
    // Build virtual eta line.
    // eta is in x = 0
    gp_Pnt pnt0 = GetChordPoint(0, xsi);
    pnt0 = gp_Pnt(0, pnt0.Y(), pnt0.Z());

    gp_Pnt pnt1 = GetChordPoint(1, xsi);
    pnt1 = gp_Pnt(0, pnt1.Y(), pnt1.Z());

    BRepBuilderAPI_MakeWire etaWireBuilder;
    TopoDS_Edge etaEdge = BRepBuilderAPI_MakeEdge(pnt0, pnt1);
    etaWireBuilder.Add(etaEdge);
    TopoDS_Wire etaLine = etaWireBuilder.Wire();

    // intersection line
    Handle(Geom_TrimmedCurve) profileLine = GC_MakeSegment(pnt, gp_Pnt(-1e9, 0, 0));
    BRepBuilderAPI_MakeEdge ME(profileLine);
    TopoDS_Shape aCrv(ME.Edge());

    // now find intersection point
    BRepExtrema_DistShapeShape extrema(etaLine, aCrv);
    extrema.Perform();
    gp_Pnt intersectionPoint = extrema.PointOnShape1(1);

    //length of the eta line
    Standard_Real len1 = pnt0.Distance(pnt1);
    // now the small line, a fraction of the original eta line
    Standard_Real len2 = pnt0.Distance(intersectionPoint);

    assert(len1 != 0.);

    return len2/len1;
}


// Returns eta as parametric distance from a given point on the surface
double CCPACSWingSegment::GetEta(gp_Pnt pnt, bool isUpper) const
{
    double eta = 0., xsi = 0.;
    GetEtaXsi(pnt, eta, xsi);
    return eta;
}

// Returns xsi as parametric distance from a given point on the surface
double CCPACSWingSegment::GetXsi(gp_Pnt pnt, bool isUpper) const
{
    double eta = 0., xsi = 0.;
    GetEtaXsi(pnt, eta, xsi);
    return xsi;
}

// Returns xsi as parametric distance from a given point on the surface
void CCPACSWingSegment::GetEtaXsi(gp_Pnt pnt, double& eta, double& xsi) const
{
    CTiglPoint tmpPnt(pnt.XYZ());
    if (ChordFace().translate(tmpPnt, &eta, &xsi) != TIGL_SUCCESS) {
        throw tigl::CTiglError("Cannot determine eta, xsi coordinates of current point in CCPACSWingSegment::GetEtaXsi!", TIGL_MATH_ERROR);
    }
}

// Returns if the given point is ont the Top of the wing or on the lower side.
bool CCPACSWingSegment::GetIsOnTop(gp_Pnt pnt) const
{
    double tolerance = 0.03; // 3cm

    GeomAPI_ProjectPointOnSurf Proj(pnt, GetUpperSurface());
    if (Proj.NbPoints() > 0 && Proj.LowerDistance() < tolerance) {
        return true;
    }
    else {
        return false;
    }
}


bool CCPACSWingSegment::GetIsOn(const gp_Pnt& pnt)
{
    bool isOnLoft = CTiglAbstractSegment::GetIsOn(pnt);

    if (isOnLoft) {
        return true;
    }

    MakeChordSurface();

    // check if point on chord surface
    double tolerance = 0.03;
    GeomAPI_ProjectPointOnSurf Proj(pnt, surfaceCache.cordFace);
    if (Proj.NbPoints() > 0 && Proj.LowerDistance() < tolerance) {
        return true;
    }
    else {
        return false;
    }
}

void CCPACSWingSegment::MakeChordSurface() const
{
    if (surfaceCache.chordsurfaceValid) {
        return;
    }
    
    CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

    // Compute points on wing profiles for the given xsi
    gp_Pnt inner_lep = innerProfile.GetChordPoint(0.);
    gp_Pnt outer_lep = outerProfile.GetChordPoint(0.);
    gp_Pnt inner_tep = innerProfile.GetChordPoint(1.);
    gp_Pnt outer_tep = outerProfile.GetChordPoint(1.);

    // Do section element transformation on points
    inner_lep = transformProfilePoint(wing->GetTransformation(), innerConnection, inner_lep);
    inner_tep = transformProfilePoint(wing->GetTransformation(), innerConnection, inner_tep);
    outer_lep = transformProfilePoint(wing->GetTransformation(), outerConnection, outer_lep);
    outer_tep = transformProfilePoint(wing->GetTransformation(), outerConnection, outer_tep);

    surfaceCache.cordSurface.setQuadriangle(inner_lep.XYZ(), outer_lep.XYZ(), inner_tep.XYZ(), outer_tep.XYZ());

    Handle(Geom_TrimmedCurve) innerEdge = GC_MakeSegment(inner_lep, inner_tep).Value();
    Handle(Geom_TrimmedCurve) outerEdge = GC_MakeSegment(outer_lep, outer_tep).Value();
    surfaceCache.cordFace = GeomFill::Surface(innerEdge, outerEdge);

    surfaceCache.chordsurfaceValid = true;
}

CTiglPointTranslator& CCPACSWingSegment::ChordFace() const
{
    if (!surfaceCache.chordsurfaceValid) {
        MakeChordSurface();
    }

    return surfaceCache.cordSurface;
}

// Builds upper/lower surfaces as shapes
// we split the wing profile into upper and lower wire.
// To do so, we have to determine, what is up
void CCPACSWingSegment::MakeSurfaces() const
{
    if (surfaceCache.valid) {
        return;
    }

    TopoDS_Edge iu_wire = innerConnection.GetProfile().GetUpperWire();
    TopoDS_Edge ou_wire = outerConnection.GetProfile().GetUpperWire();
    TopoDS_Edge il_wire = innerConnection.GetProfile().GetLowerWire();
    TopoDS_Edge ol_wire = outerConnection.GetProfile().GetLowerWire();

    CTiglTransformation identity;
    TopoDS_Edge iu_wire_local = TopoDS::Edge(transformProfileWire(identity, innerConnection, iu_wire));
    TopoDS_Edge ou_wire_local = TopoDS::Edge(transformProfileWire(identity, outerConnection, ou_wire));
    TopoDS_Edge il_wire_local = TopoDS::Edge(transformProfileWire(identity, innerConnection, il_wire));
    TopoDS_Edge ol_wire_local = TopoDS::Edge(transformProfileWire(identity, outerConnection, ol_wire));
    iu_wire = TopoDS::Edge(transformProfileWire(wing->GetTransformation(), innerConnection, iu_wire));
    ou_wire = TopoDS::Edge(transformProfileWire(wing->GetTransformation(), outerConnection, ou_wire));
    il_wire = TopoDS::Edge(transformProfileWire(wing->GetTransformation(), innerConnection, il_wire));
    ol_wire = TopoDS::Edge(transformProfileWire(wing->GetTransformation(), outerConnection, ol_wire));

    BRepOffsetAPI_ThruSections upperSections(Standard_False,Standard_True);
    upperSections.AddWire(BRepBuilderAPI_MakeWire(iu_wire));
    upperSections.AddWire(BRepBuilderAPI_MakeWire(ou_wire));
    upperSections.Build();

    BRepOffsetAPI_ThruSections lowerSections(Standard_False,Standard_True);
    lowerSections.AddWire(BRepBuilderAPI_MakeWire(il_wire));
    lowerSections.AddWire(BRepBuilderAPI_MakeWire(ol_wire));
    lowerSections.Build();

    BRepOffsetAPI_ThruSections upperSectionsLocal(Standard_False, Standard_True);
    upperSectionsLocal.AddWire(BRepBuilderAPI_MakeWire(iu_wire_local));
    upperSectionsLocal.AddWire(BRepBuilderAPI_MakeWire(ou_wire_local));
    upperSectionsLocal.Build();

    BRepOffsetAPI_ThruSections lowerSectionsLocal(Standard_False, Standard_True);
    lowerSectionsLocal.AddWire(BRepBuilderAPI_MakeWire(il_wire_local));
    lowerSectionsLocal.AddWire(BRepBuilderAPI_MakeWire(ol_wire_local));
    lowerSectionsLocal.Build();

#ifndef NDEBUG
    assert(GetNumberOfFaces(upperSections.Shape()) == 1);
    assert(GetNumberOfFaces(lowerSections.Shape()) == 1);
    assert(GetNumberOfFaces(upperSectionsLocal.Shape()) == 1);
    assert(GetNumberOfFaces(lowerSectionsLocal.Shape()) == 1);
#endif

    TopExp_Explorer faceExplorer;
    faceExplorer.Init(upperSections.Shape(), TopAbs_FACE);
#ifndef NDEBUG
    assert(faceExplorer.More());
#endif
    surfaceCache.upperShape = faceExplorer.Current();
    surfaceCache.upperSurface = BRep_Tool::Surface(TopoDS::Face(surfaceCache.upperShape));

    faceExplorer.Init(upperSectionsLocal.Shape(), TopAbs_FACE);
#ifndef NDEBUG
    assert(faceExplorer.More());
#endif
    surfaceCache.upperShapeLocal = faceExplorer.Current();
    surfaceCache.upperSurfaceLocal = BRep_Tool::Surface(TopoDS::Face(surfaceCache.upperShapeLocal));

    faceExplorer.Init(lowerSections.Shape(), TopAbs_FACE);
#ifndef NDEBUG
    assert(faceExplorer.More());
#endif
    surfaceCache.lowerShape = faceExplorer.Current();
    surfaceCache.lowerSurface = BRep_Tool::Surface(TopoDS::Face(surfaceCache.lowerShape));

    faceExplorer.Init(lowerSectionsLocal.Shape(), TopAbs_FACE);
#ifndef NDEBUG
    assert(faceExplorer.More());
#endif
    surfaceCache.lowerShapeLocal = faceExplorer.Current();
    surfaceCache.lowerSurfaceLocal = BRep_Tool::Surface(TopoDS::Face(surfaceCache.lowerShapeLocal));

    // compute total surface area
    GProp_GProps sprops;
    BRepGProp::SurfaceProperties(surfaceCache.upperShape, sprops);
    double upperArea = sprops.Mass();
    BRepGProp::SurfaceProperties(surfaceCache.lowerShape, sprops);
    double lowerArea = sprops.Mass();
    
    surfaceCache.mySurfaceArea = upperArea + lowerArea;

    // compute shapes for opened profiles
    TopoDS_Edge iu_wire_open = innerConnection.GetProfile().GetUpperWireOpened();
    TopoDS_Edge ou_wire_open = outerConnection.GetProfile().GetUpperWireOpened();
    TopoDS_Edge il_wire_open = innerConnection.GetProfile().GetLowerWireOpened();
    TopoDS_Edge ol_wire_open = outerConnection.GetProfile().GetLowerWireOpened();
    iu_wire_open = TopoDS::Edge(transformProfileWire(identity, innerConnection, iu_wire_open));
    ou_wire_open = TopoDS::Edge(transformProfileWire(identity, outerConnection, ou_wire_open));
    il_wire_open = TopoDS::Edge(transformProfileWire(identity, innerConnection, il_wire_open));
    ol_wire_open = TopoDS::Edge(transformProfileWire(identity, outerConnection, ol_wire_open));

    // compute upper and lower shapes for opened profile
    BRepOffsetAPI_ThruSections upperSectionsOpened(Standard_False,Standard_True);
    upperSectionsOpened.AddWire(BRepBuilderAPI_MakeWire(iu_wire_open));
    upperSectionsOpened.AddWire(BRepBuilderAPI_MakeWire(ou_wire_open));
    upperSectionsOpened.Build();

    BRepOffsetAPI_ThruSections lowerSectionsOpened(Standard_False,Standard_True);
    lowerSectionsOpened.AddWire(BRepBuilderAPI_MakeWire(il_wire_open));
    lowerSectionsOpened.AddWire(BRepBuilderAPI_MakeWire(ol_wire_open));
    lowerSectionsOpened.Build();

#ifndef NDEBUG
    assert(GetNumberOfFaces(upperSectionsOpened.Shape()) == 1);
    assert(GetNumberOfFaces(lowerSectionsOpened.Shape()) == 1);
#endif

    faceExplorer.Init(upperSectionsOpened.Shape(), TopAbs_FACE);
#ifndef NDEBUG
    assert(faceExplorer.More());
#endif
    surfaceCache.upperShapeOpened = faceExplorer.Current();

    faceExplorer.Init(lowerSectionsOpened.Shape(), TopAbs_FACE);
#ifndef NDEBUG
    assert(faceExplorer.More());
#endif
    surfaceCache.lowerShapeOpened = faceExplorer.Current();

    CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

    // get trailing edge wires from inner and outer profile
    TopoDS_Edge innerTEWire = innerProfile.GetTrailingEdgeOpened();
    TopoDS_Edge outerTEWire = outerProfile.GetTrailingEdgeOpened();
    if (innerTEWire.IsNull() || outerTEWire.IsNull()) {
        throw CTiglError("ERROR: trailing edge geometry of Wing Section Profile not found!");
    }
    // transform wires
    innerTEWire = TopoDS::Edge(transformProfileWire(identity, innerConnection, innerTEWire));
    outerTEWire = TopoDS::Edge(transformProfileWire(identity, outerConnection, outerTEWire));
    // generate face
    BRepOffsetAPI_ThruSections teGenerator(Standard_False, Standard_True);
    teGenerator.AddWire(BRepBuilderAPI_MakeWire(innerTEWire));
    teGenerator.AddWire(BRepBuilderAPI_MakeWire(outerTEWire));
    teGenerator.Build();
    surfaceCache.trailingEdgeShape = teGenerator.Shape();

    surfaceCache.valid = true;
}



// Returns the reference area of the quadrilateral portion of the wing segment
// by projecting the wing segment into the plane defined by the user
double CCPACSWingSegment::GetReferenceArea(TiglSymmetryAxis symPlane) const
{
    CTiglPoint innerLepProj(GetChordPoint(0, 0.).XYZ());
    CTiglPoint outerLepProj(GetChordPoint(0, 1.).XYZ());
    CTiglPoint innerTepProj(GetChordPoint(1, 0.).XYZ());
    CTiglPoint outerTepProj(GetChordPoint(1, 1.).XYZ());

    // project into plane
    switch (symPlane) {
    case TIGL_X_Y_PLANE:
        innerLepProj.z = 0.;
        outerLepProj.z = 0.;
        innerTepProj.z = 0.;
        outerTepProj.z = 0.;
        break;

    case TIGL_X_Z_PLANE:
        innerLepProj.y = 0.;
        outerLepProj.y = 0.;
        innerTepProj.y = 0.;
        outerTepProj.y = 0.;
        break;

    case TIGL_Y_Z_PLANE:
        innerLepProj.x = 0.;
        outerLepProj.x = 0.;
        innerTepProj.x = 0.;
        outerTepProj.x = 0.;
        break;
    default:
        // don't project
        break;
    }

    return quadrilateral_area(innerTepProj, outerTepProj, outerLepProj, innerLepProj);
}

// Returns the lower Surface of this Segment
Handle(Geom_Surface) CCPACSWingSegment::GetLowerSurface(TiglCoordinateSystem referenceCS) const
{
    if (!surfaceCache.valid) {
        MakeSurfaces();
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return surfaceCache.lowerSurfaceLocal;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return surfaceCache.lowerSurface;
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetLowerSurface");
    }
}

// Returns the upper Surface of this Segment
Handle(Geom_Surface) CCPACSWingSegment::GetUpperSurface(TiglCoordinateSystem referenceCS) const
{
    if (!surfaceCache.valid) {
        MakeSurfaces();
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return surfaceCache.upperSurfaceLocal;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return surfaceCache.upperSurface;
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetUpperSurface");
    }
}

// Returns the upper wing shape of this Segment
TopoDS_Shape& CCPACSWingSegment::GetUpperShape(TiglCoordinateSystem referenceCS) const
{
    if (!surfaceCache.valid) {
        MakeSurfaces();
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return surfaceCache.upperShapeLocal;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return surfaceCache.upperShape;
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetUpperShape");
    }
}

// Returns the lower wing shape of this Segment
TopoDS_Shape& CCPACSWingSegment::GetLowerShape(TiglCoordinateSystem referenceCS) const
{
    if (!surfaceCache.valid) {
        MakeSurfaces();
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return surfaceCache.lowerShapeLocal;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return surfaceCache.lowerShape;
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetLowerShape");
    }
}

// get guide curve for given UID
const CCPACSGuideCurve& CCPACSWingSegment::GetGuideCurve(std::string UID) const
{
    return m_guideCurves->GetGuideCurve(UID);
}

// check if guide curve with a given UID exists
bool CCPACSWingSegment::GuideCurveExists(std::string UID) const
{
    return m_guideCurves->GuideCurveExists(UID);
}

TopTools_SequenceOfShape& CCPACSWingSegment::GetGuideCurveWires() const
{
    if (guideCurveWires.IsEmpty()) {
        BuildGuideCurveWires();
    }
    return guideCurveWires;
}

// Creates all guide curves
void CCPACSWingSegment::BuildGuideCurveWires() const
{
    guideCurveWires.Clear();
    if (HasGuideCurves()) {
        const auto& wingTransform = GetWing().GetTransformation();

        // get upper and lower part of inner profile in world coordinates
        CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
        TopoDS_Edge upperInnerWire = TopoDS::Edge(transformProfileWire(wingTransform, innerConnection, innerProfile.GetUpperWire()));
        TopoDS_Edge lowerInnerWire = TopoDS::Edge(transformProfileWire(wingTransform, innerConnection, innerProfile.GetLowerWire()));

        // get upper and lower part of outer profile in world coordinates
        CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
        TopoDS_Edge upperOuterWire = TopoDS::Edge(transformProfileWire(wingTransform, outerConnection, outerProfile.GetUpperWire()));
        TopoDS_Edge lowerOuterWire = TopoDS::Edge(transformProfileWire(wingTransform, outerConnection, outerProfile.GetLowerWire()));

        // concatenate inner profile wires for guide curve construction algorithm
        TopTools_SequenceOfShape concatenatedInnerWires;
        concatenatedInnerWires.Append(lowerInnerWire);
        concatenatedInnerWires.Append(upperInnerWire);

        // concatenate outer profile wires for guide curve construction algorithm
        TopTools_SequenceOfShape concatenatedOuterWires;
        concatenatedOuterWires.Append(lowerOuterWire);
        concatenatedOuterWires.Append(upperOuterWire);

        // get chord lengths for inner profile in word coordinates
        TopoDS_Wire innerChordLineWire = TopoDS::Wire(transformProfileWire(wingTransform, innerConnection, innerProfile.GetChordLineWire()));
        TopoDS_Wire outerChordLineWire = TopoDS::Wire(transformProfileWire(wingTransform, outerConnection, outerProfile.GetChordLineWire()));
        double innerScale = GetWireLength(innerChordLineWire);
        double outerScale = GetWireLength(outerChordLineWire);

        // loop through all guide curves and construct the corresponding wires
        int nGuideCurves = m_guideCurves->GetGuideCurveCount();
        for (int i=0; i!=nGuideCurves; i++) {
            // get guide curve
            const CCPACSGuideCurve& guideCurve = m_guideCurves->GetGuideCurve(i+1);
            double fromRelativeCircumference;
            // check if fromRelativeCircumference is given in the current guide curve
            if (guideCurve.GetFromRelativeCircumferenceIsSet()) {
                fromRelativeCircumference = guideCurve.GetFromRelativeCircumference();
            }
            // otherwise get relative circumference from neighboring segment guide curve
            else {
                // get neighboring guide curve UID
                std::string neighborGuideCurveUID = guideCurve.GetFromGuideCurveUID_choice1();
                // get neighboring guide curve
                const CCPACSGuideCurve& neighborGuideCurve = wing->GetGuideCurve(neighborGuideCurveUID);
                // get relative circumference from neighboring guide curve
                fromRelativeCircumference = neighborGuideCurve.GetToRelativeCircumference();
            }
            // get relative circumference of outer profile
            double toRelativeCircumference = guideCurve.GetToRelativeCircumference();
            // get guide curve profile UID
            std::string guideCurveProfileUID = guideCurve.GetGuideCurveProfileUID();
            // get relative circumference of inner profile

            // get guide curve profile
            CCPACSConfiguration& config = wing->GetConfiguration();
            CCPACSGuideCurveProfile& guideCurveProfile = config.GetGuideCurveProfile(guideCurveProfileUID);

            // construct guide curve algorithm
            TopoDS_Wire guideCurveWire = CCPACSGuideCurveAlgo<CCPACSWingProfileGetPointAlgo>(concatenatedInnerWires, concatenatedOuterWires, fromRelativeCircumference, toRelativeCircumference, innerScale, outerScale, guideCurveProfile);
            guideCurveWires.Append(guideCurveWire);
        }
    }
}

int CCPACSWingSegment::GetGuideCurveCount() const
{
    if (m_guideCurves)
        return m_guideCurves->GetGuideCurveCount();
    else
        return 0;
}

} // end namespace tigl


