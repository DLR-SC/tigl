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
* @brief  Implementation of CPACS wing segment handling routines.
*/

#include <cmath>
#include <iostream>
#include <string>
#include <cassert>
#include <cfloat>
#include <map>

#include "CCPACSWingSegment.h"
#include "CTiglWingSegmentGuidecurveBuilder.h"
#include "CCPACSWing.h"
#include "CCPACSWings.h"
#include "CCPACSEnginePylon.h"
#include "CCPACSWingSegments.h"
#include "CCPACSWingProfiles.h"
#include "CCPACSGuideCurveProfiles.h"
#include "CCPACSGuideCurveAlgo.h"
#include "CCPACSWingProfileGetPointAlgo.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include "CTiglMakeLoft.h"
#include "CTiglPatchShell.h"
#include "tiglcommonfunctions.h"
#include "tigl_config.h"
#include "math/tiglmathfunctions.h"
#include "CNamedShape.h"

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
    gp_Pnt transformProfilePoint(const tigl::CTiglTransformation& wingTransform, const tigl::CTiglWingConnection& connection, const gp_Pnt& point)
    {
        CTiglTransformation trafo = connection.GetSectionElementTransformation();
        trafo.PreMultiply(connection.GetSectionTransformation());
        trafo.PreMultiply(connection.GetPositioningTransformation());
        trafo.PreMultiply(wingTransform);
        return trafo.Transform(point);
    }

    // Set the face traits
    void SetFaceTraits (PNamedShape loft, std::string shapeUID) 
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
        TopExp::MapShapes(loft->Shape(), TopAbs_FACE, map);
        
        for (int iFace = 0; iFace < map.Extent(); ++iFace) {
            loft->FaceTraits(iFace).SetComponentUID(shapeUID);
        }

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

CCPACSWingSegment::CCPACSWingSegment(CCPACSWingSegments* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSegment(parent, uidMgr)
    , CTiglAbstractSegment<CCPACSWingSegment>(parent->GetSegments(), parent->GetParentComponent())
    , innerConnection(this)
    , outerConnection(this)
    , surfaceCoordCache(*this, &CCPACSWingSegment::MakeChordSurface)
    , areaCache(*this, &CCPACSWingSegment::ComputeArea)
    , volumeCache(*this, &CCPACSWingSegment::ComputeVolume)
    , m_guideCurveBuilder(make_unique<CTiglWingSegmentGuidecurveBuilder>(*this))
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
    CTiglAbstractSegment<CCPACSWingSegment>::Reset();
    areaCache.clear();
    volumeCache.clear();
}

// Cleanup routine
void CCPACSWingSegment::Cleanup()
{
    Invalidate();
}

// Update internal segment data
void CCPACSWingSegment::Update()
{
    Invalidate();
}

// Read CPACS segment elements
void CCPACSWingSegment::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& segmentXPath)
{
    Cleanup();
    generated::CPACSWingSegment::ReadCPACS(tixiHandle, segmentXPath);

    // trigger creation of connections
    SetFromElementUID(m_fromElementUID);
    SetToElementUID(m_toElementUID);

    if (m_guideCurves) {
        for (int iguide = 1; iguide <= m_guideCurves->GetGuideCurveCount(); ++iguide) {
            m_guideCurves->GetGuideCurve(iguide).SetGuideCurveBuilder(*m_guideCurveBuilder);
        }
    }

    // TODO: check in case of guide curves, that the curves of the first
    // segment have a relativeFromCircumference set, the others not


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

std::string CCPACSWingSegment::GetDefaultedUID() const {
    return generated::CPACSWingSegment::GetUID();
}

void CCPACSWingSegment::SetFromElementUID(const std::string& value) {
    generated::CPACSWingSegment::SetFromElementUID(value);
    innerConnection.SetElementUID(value);
}

void CCPACSWingSegment::SetToElementUID(const std::string& value) {
    generated::CPACSWingSegment::SetToElementUID(value);
    outerConnection.SetElementUID(value);
}


// helper function to get the inner transformed chord line wire
TopoDS_Wire CCPACSWingSegment::GetInnerWire(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    const CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    TopoDS_Wire w;

    /*
    * The loft algorithm with guide curves does not like splitted
    * wing profiles, we have to give him the unsplitted one.
    * In all other cases, we need the splitted wire to distiguish
    * upper und lower wing surface
    */
    if (m_guideCurves && m_guideCurves->GetGuideCurveCount() > 0) {
        w = innerProfile.GetWire(mod);
    }
    else {
        w = innerProfile.GetSplitWire(mod);
    }

    CTiglTransformation identity;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformWingProfileGeometry(identity, innerConnection, w));
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformWingProfileGeometry(GetParentTransformation(), innerConnection, w));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetInnerWire");
    }
}

// helper function to get the outer transformed chord line wire
TopoDS_Wire CCPACSWingSegment::GetOuterWire(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    const CCPACSWingProfile& outerProfile = outerConnection.GetProfile();
    TopoDS_Wire w;

    /*
    * The loft algorithm with guide curves does not like splitted
    * wing profiles, we have to give him the unsplitted one.
    * In all other cases, we need the splitted wire to distiguish
    * upper und lower wing surface
    */
    if (m_guideCurves && m_guideCurves->GetGuideCurveCount() > 0) {
        w = outerProfile.GetWire(mod);
    }
    else {
        w = outerProfile.GetSplitWire(mod);
    }

    CTiglTransformation identity;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformWingProfileGeometry(identity, outerConnection, w));
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Wire(transformWingProfileGeometry(GetParentTransformation(), outerConnection, w));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingSegment::GetOuterWire");
    }
}

// helper function to get the inner closing of the wing segment
// using shape generated in MakeSurfaces
TopoDS_Shape CCPACSWingSegment::GetInnerClosure(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    TopoDS_Wire wire = GetInnerWire(referenceCS, mod);
    return BRepBuilderAPI_MakeFace(wire).Face();
}

// helper function to get the inner closing of the wing segment
// using shape generated in MakeSurfaces
TopoDS_Shape CCPACSWingSegment::GetOuterClosure(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    TopoDS_Wire wire = GetOuterWire(referenceCS, mod);
    return BRepBuilderAPI_MakeFace(wire).Face();
}

// get short name for loft
std::string CCPACSWingSegment::GetShortShapeName () const
{
    size_t windex = 0;
    int wsindex = GetSegmentIndex();

    std::stringstream shortName;

    if (GetParent()->IsParent<CCPACSWing>()) {
        const CCPACSWing* wing = GetParent()->GetParent<CCPACSWing>();
        if (wing->IsParent<CCPACSRotorBlades>()) {
            windex = IndexFromUid(wing->GetParent<CCPACSRotorBlades>()->GetRotorBlades(), wing->GetUID());
            shortName << "R";
        }
        else {
            windex = IndexFromUid(wing->GetParent<CCPACSWings>()->GetWings(), wing->GetUID());
            shortName << "W";
        }

    }
    else if (GetParent()->IsParent<CCPACSEnginePylon>()) {
        const CCPACSEnginePylon* pylon = GetParent()->GetParent<CCPACSEnginePylon>();
        windex = IndexFromUid(pylon->GetParent()->GetEnginePylons(), pylon->GetUID());
        shortName << "P";
    }
    else {
        LOG(WARNING) << "Cannot create short shape name for component \"" + GetUID() + "\"";
        return "UNKNOWN";
    }

    shortName << windex+1 << "S" << wsindex;
    return shortName.str();
}

// Builds the loft between the two segment sections
// build loft out of faces (for compatibility with component segmen loft)
PNamedShape CCPACSWingSegment::BuildLoft() const
{
    TopoDS_Shape loftShape;
    if ( loftLinearly ) {
        // build loft using inner and outer wires and possibly guidecurves
        TopoDS_Wire innerWire = GetInnerWire();
        TopoDS_Wire outerWire = GetOuterWire();

        // Build loft
        CTiglMakeLoft lofter;
        lofter.addProfiles(innerWire);
        lofter.addProfiles(outerWire);


        if (m_guideCurves) {
            const CCPACSGuideCurves& curves = *m_guideCurves;
            bool hasTrailingEdge = !innerConnection.GetProfile().GetTrailingEdge().IsNull();

            // order guide curves according to fromRelativeCircumeference
            std::multimap<double, const CCPACSGuideCurve*> guideMap;
            for (int iguide = 1; iguide <= curves.GetGuideCurveCount(); ++iguide) {
                const CCPACSGuideCurve* curve = &curves.GetGuideCurve(iguide);
                double value = *(curve->GetFromRelativeCircumference_choice2());
                if (value >= 1. && !hasTrailingEdge) {
                    // this is a trailing edge profile, we should add it first
                    value = -1.;
                }
                guideMap.insert(std::make_pair(value, curve));
            }

            std::multimap<double, const CCPACSGuideCurve*>::iterator it;
            for (it = guideMap.begin(); it != guideMap.end(); ++it) {
                const CCPACSGuideCurve* curve = it->second;
                BRepBuilderAPI_MakeWire wireMaker(curve->GetCurve());
                lofter.addGuides(wireMaker.Wire());
            }
        }

        loftShape = lofter.Shape();
        if (loftShape.IsNull()) {
            LOG(ERROR) << "Cannot compute wing segment loft " << GetUID();
            return PNamedShape();
        }

        Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
        sfs->Init ( loftShape );
        sfs->Perform();
        loftShape = sfs->Shape();
    }
    else {
        const CCPACSWingSegments* segments = GetParent();
        PNamedShape wingLoft = segments->GetParentComponent()->GetLoft();


        TopoDS_Shell loftShell;
        BRep_Builder BB;
        BB.MakeShell(loftShell);

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(wingLoft->Shape(), TopAbs_FACE, faceMap);
        int nFaces = faceMap.Extent();
        int nSegments = segments->GetSegmentCount();
        int nFacesPerSegment = (nFaces - 2)/nSegments;

        // determine index of segment to retrieve the correct subshapes of the wing
        // Here we explicitly require the subshapes to be ordered consistently
        for (int j = 1; j <= segments->GetSegmentCount(); j++) {
            const CCPACSWingSegment& ws = segments->GetSegment(j);
            if (GetUID() == ws.GetUID()) {
                for(int i=0; i<nFacesPerSegment; ++i) {
                    BB.Add(loftShell, TopoDS::Face(faceMap((j-1)*nFacesPerSegment + i + 1))); // guides
                }
                break;
            }
        }

        //close the shell with sidecaps and make them a solid
        TopoDS_Wire innerWire = GetInnerWire();
        TopoDS_Wire outerWire = GetOuterWire();

        CTiglPatchShell patcher(loftShell);
        patcher.AddSideCap(innerWire);
        patcher.AddSideCap(outerWire);
        loftShape = patcher.PatchedShape();
    }

    // Set Names
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft (new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft, GetUID());
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
CTiglWingConnection& CCPACSWingSegment::GetInnerConnection()
{
    return( innerConnection );
}

const CTiglWingConnection& CCPACSWingSegment::GetInnerConnection() const
{
    return(innerConnection);
}

// Returns the end section element index of this segment
CTiglWingConnection& CCPACSWingSegment::GetOuterConnection()
{
    return( outerConnection );
}

const CTiglWingConnection& CCPACSWingSegment::GetOuterConnection() const
{
    return(outerConnection);
}

// Returns the volume of this segment
double CCPACSWingSegment::GetVolume()
{
    return *volumeCache;
}

// Returns the surface area of this segment
double CCPACSWingSegment::GetSurfaceArea() const
{
    return *areaCache;
}

void CCPACSWingSegment::etaXsiToUV(bool isFromUpper, double eta, double xsi, double& u, double& v) const
{
    gp_Pnt pnt = GetPoint(eta,xsi, isFromUpper);

    Handle(Geom_Surface) surf;
    if (isFromUpper) {
        surf = GetUpperSurface();
    }
    else {
        surf = GetLowerSurface();
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
    TopoDS_Face face;
    if (fromUpper) {
        face = TopoDS::Face(GetUpperShape());
    }
    else {
        face = TopoDS::Face(GetLowerShape());
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
    const CCPACSWingSegments* segments = GetParent();
    if (!segments) {
        return 0;
    }


    int count = 0;
    for (int i = 1; i <= segments->GetSegmentCount(); i++) {
        const CCPACSWingSegment& nextSegment = segments->GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
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
    const CCPACSWingSegments* segments = GetParent();
    if (!segments) {
        return 0;
    }

    for (int i = 1; i <= segments->GetSegmentCount(); i++) {
        const CCPACSWingSegment& nextSegment = segments->GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
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
        throw CTiglError("Invalid value for parameter n in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_INDEX_ERROR);
    }

    const CCPACSWingSegments* segments = GetParent();
    if (!segments) {
        throw CTiglError("No connected segment found in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    for (int i = 1, count = 0; i <= segments->GetSegmentCount(); i++) {
        const CCPACSWingSegment& nextSegment = segments->GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
            continue;
        }
        if (nextSegment.GetInnerSectionUID() == GetInnerSectionUID() ||
            nextSegment.GetOuterSectionUID() == GetInnerSectionUID()) {

            if (++count == n) {
                return nextSegment.GetSegmentIndex();
            }
        }
    }

    throw CTiglError("No connected segment found in CCPACSWingSegment::GetInnerConnectedSegmentIndex", TIGL_NOT_FOUND);
}

// Gets the index (number) of the n-th segment connected to the outer section
// of this segment. n starts at 1.
int CCPACSWingSegment::GetOuterConnectedSegmentIndex(int n) const
{
    if (n < 1 || n > GetOuterConnectedSegmentCount()) {
        throw CTiglError("Invalid value for parameter n in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_INDEX_ERROR);
    }

    const CCPACSWingSegments* segments = GetParent();
    if (!segments) {
        throw CTiglError("No connected segment found in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_NOT_FOUND);
    }

    for (int i = 1, count = 0; i <= segments->GetSegmentCount(); i++) {
        const CCPACSWingSegment& nextSegment = segments->GetSegment(i);
        if (nextSegment.GetSegmentIndex() == GetSegmentIndex()) {
            continue;
        }
        if (nextSegment.GetInnerSectionUID() == GetOuterSectionUID() ||
            nextSegment.GetOuterSectionUID() == GetOuterSectionUID()) {

            if (++count == n) {
                return nextSegment.GetSegmentIndex();
            }
        }
    }

    throw CTiglError("No connected segment found in CCPACSWingSegment::GetOuterConnectedSegmentIndex", TIGL_NOT_FOUND);
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
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
    }

    const CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    const CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

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
        innerProfilePoint = transformProfilePoint(GetParentTransformation(), innerConnection, innerProfilePoint);
        outerProfilePoint = transformProfilePoint(GetParentTransformation(), outerConnection, outerProfilePoint);
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
    if (!GetParent()->IsParent<CCPACSWing>()) {
        throw ("GetPointDirection not implemented for \"" + GetUID() + "\".");
    }

    CCPACSWing* wing = const_cast<CCPACSWing*>(GetParent()->GetParent<CCPACSWing>());

    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingSegment::GetPoint", TIGL_ERROR);
    }

    if (dirx*dirx + diry*diry + dirz*dirz < 1e-10) {
        // invalid direction
        throw CTiglError("Direction must not be a null vector in CCPACSWingSegment::GetPointDirection.", TIGL_MATH_ERROR);
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

CTiglTransformation CCPACSWingSegment::GetParentTransformation() const
{
    return GetParent()->GetParentComponent()->GetTransformationMatrix();
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
    bool isOnLoft = CTiglAbstractSegment<CCPACSWingSegment>::GetIsOn(pnt);

    if (isOnLoft) {
        return true;
    }

    // check if point on chord surface
    double tolerance = 0.03;
    GeomAPI_ProjectPointOnSurf Proj(pnt, surfaceCoordCache->cordFace);
    if (Proj.NbPoints() > 0 && Proj.LowerDistance() < tolerance) {
        return true;
    }
    else {
        return false;
    }
}

gp_Pnt CCPACSWingSegment::GetInnerProfilePoint(double xsi)  const
{
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingSegment::GetPointOnInnerProfile", TIGL_ERROR);
    }

    gp_Pnt untransformed = innerConnection.GetProfile().GetChordPoint(xsi);
    return transformProfilePoint(GetParentTransformation(), innerConnection, untransformed);
}

gp_Pnt CCPACSWingSegment::GetOuterProfilePoint(double xsi) const
{
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingSegment::GetPointOnInnerProfile", TIGL_ERROR);
    }

    gp_Pnt untransformed = outerConnection.GetProfile().GetChordPoint(xsi);
    return transformProfilePoint(GetParentTransformation(), outerConnection, untransformed);
}

void CCPACSWingSegment::MakeChordSurface(SurfaceCoordCache& cache) const
{
    const CCPACSWingProfile& innerProfile = innerConnection.GetProfile();
    const CCPACSWingProfile& outerProfile = outerConnection.GetProfile();

    // Compute points on wing profiles for the given xsi
    gp_Pnt inner_lep = innerProfile.GetChordPoint(0.);
    gp_Pnt outer_lep = outerProfile.GetChordPoint(0.);
    gp_Pnt inner_tep = innerProfile.GetChordPoint(1.);
    gp_Pnt outer_tep = outerProfile.GetChordPoint(1.);

    // Do section element transformation on points
    inner_lep = transformProfilePoint(GetParentTransformation(), innerConnection, inner_lep);
    inner_tep = transformProfilePoint(GetParentTransformation(), innerConnection, inner_tep);
    outer_lep = transformProfilePoint(GetParentTransformation(), outerConnection, outer_lep);
    outer_tep = transformProfilePoint(GetParentTransformation(), outerConnection, outer_tep);

    cache.cordSurface.setQuadriangle(inner_lep.XYZ(), outer_lep.XYZ(), inner_tep.XYZ(), outer_tep.XYZ());

    Handle(Geom_TrimmedCurve) innerEdge = GC_MakeSegment(inner_lep, inner_tep).Value();
    Handle(Geom_TrimmedCurve) outerEdge = GC_MakeSegment(outer_lep, outer_tep).Value();
    cache.cordFace = GeomFill::Surface(innerEdge, outerEdge);
}


void CCPACSWingSegment::ComputeVolume(double& cache) const
{
    const TopoDS_Shape loft = GetLoft()->Shape();
    GProp_GProps gprops;
    BRepGProp::VolumeProperties(loft, gprops);
    cache = gprops.Mass();
}

const CTiglPointTranslator& CCPACSWingSegment::ChordFace() const
{
    return surfaceCoordCache->cordSurface;
}

// Builds upper/lower surfaces as shapes
// we split the wing profile into upper and lower wire.
// To do so, we have to determine, what is up
void CCPACSWingSegment::ComputeArea(double& cache) const
{
    // compute total surface area
    GProp_GProps sprops;
    BRepGProp::SurfaceProperties(GetUpperShape(), sprops);
    const double upperArea = sprops.Mass();
    BRepGProp::SurfaceProperties(GetLowerShape(), sprops);
    const double lowerArea = sprops.Mass();
    cache = upperArea + lowerArea;
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

PNamedShape CCPACSWingSegment::GetLoft(TiglShapeModifier mod) const
{
    if (mod != UNMODIFIED_SHAPE)
        throw CTiglError("TiglShapeModifier is not yet supported for CCPACSWingSegment::GetLoft");

    return GetLoft();
}

// Returns the lower Surface of this Segment
Handle(Geom_Surface) CCPACSWingSegment::GetLowerSurface(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    return BRep_Tool::Surface(TopoDS::Face(GetLowerShape(referenceCS, mod)));
}

// Returns the upper Surface of this Segment
Handle(Geom_Surface) CCPACSWingSegment::GetUpperSurface(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    return BRep_Tool::Surface(TopoDS::Face(GetUpperShape(referenceCS, mod)));
}

// Returns the upper wing shape of this Segment
TopoDS_Shape CCPACSWingSegment::GetUpperShape(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    TopoDS_Shape s = GetFacesByName(GetLoft(mod), "Top");
    if (referenceCS == GLOBAL_COORDINATE_SYSTEM)
        return s;
    return GetParent()->GetParent<CCPACSWing>()->GetTransformationMatrix().Inverted().Transform(s);
}

// Returns the lower wing shape of this Segment
TopoDS_Shape CCPACSWingSegment::GetLowerShape(TiglCoordinateSystem referenceCS, TiglShapeModifier mod) const
{
    TopoDS_Shape s = GetFacesByName(GetLoft(mod), "Bottom");
    if (referenceCS == GLOBAL_COORDINATE_SYSTEM)
        return s;
    return GetParent()->GetParent<CCPACSWing>()->GetTransformationMatrix().Inverted().Transform(s);
}

TopoDS_Shape CCPACSWingSegment::GetTrailingEdgeShape(TiglCoordinateSystem referenceCS,
                                                                 TiglShapeModifier mod) const
{
    TopoDS_Shape s = GetFacesByName(GetLoft(mod), "TrailingEdge");
    if (referenceCS == GLOBAL_COORDINATE_SYSTEM)
        return s;
    return GetParent()->GetParent<CCPACSWing>()->GetTransformationMatrix().Inverted().Transform(s);
}

} // end namespace tigl


