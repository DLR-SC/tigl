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
* @brief  Implementation of CPACS fuselage handling routines.
*/
#include <cmath>
#include <iostream>

#include "tigl_config.h"

#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSFuselageStringerFramePosition.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWingSegment.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"
#include "Debugging.h"
#include "CTiglCurveConnector.h"
#include "CTiglMakeLoft.h"
#include "CTiglBSplineAlgorithms.h"
#include "CTiglTopoAlgorithms.h"
#include "CCPACSDuctAssembly.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools.hxx"
#include "BRepGProp.hxx"
#include "BRepBuilderAPI_Transform.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "TopoDS_Edge.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "GC_MakeSegment.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "ShapeFix_Wire.hxx"
#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include <TopExp_Explorer.hxx>
#include <IntCurvesFace_Intersector.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepProj_Projection.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>


namespace tigl
{

CCPACSFuselage::CCPACSFuselage(CCPACSFuselages* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselage(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
    , cleanLoft(*this, &CCPACSFuselage::BuildCleanLoft)
{
    Cleanup();
    if (parent->IsParent<CCPACSAircraftModel>()) {
        configuration = &parent->GetParent<CCPACSAircraftModel>()->GetConfiguration();

        // register invalidation in CCPACSDucts
        if (configuration->HasDucts()) {
            configuration->GetDucts()->RegisterInvalidationCallback([&](){ this->Invalidate(); });
        }
    }
    else
        configuration = &parent->GetParent<CCPACSRotorcraftModel>()->GetConfiguration();

}

// Destructor
CCPACSFuselage::~CCPACSFuselage()
{
    Cleanup();
}

// Invalidates internal state
void CCPACSFuselage::InvalidateImpl(const boost::optional<std::string>& /*source*/) const
{
    cleanLoft.clear();
    loft.clear();
    m_segments.Invalidate();
    if (m_structure)
        m_structure->Invalidate();
}

// Cleanup routine
void CCPACSFuselage::Cleanup()
{
    m_name = "";
    m_transformation.reset();

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Read CPACS fuselage element
void CCPACSFuselage::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& fuselageXPath)
{
    Cleanup();

    generated::CPACSFuselage::ReadCPACS(tixiHandle, fuselageXPath);
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSFuselage::GetConfiguration() const
{
    return *configuration;
}

std::string CCPACSFuselage::GetDefaultedUID() const
{
    return generated::CPACSFuselage::GetUID();
}

void CCPACSFuselage::SetSymmetryAxis(const TiglSymmetryAxis& axis)
{
    CTiglRelativelyPositionedComponent::SetSymmetryAxis(axis);
    Invalidate();
}

void CCPACSFuselage::SetTransformation(const CCPACSTransformation& transform)
{
    CTiglRelativelyPositionedComponent::SetTransformation(transform);
    Invalidate();
}

void CCPACSFuselage::SetParentUID(const boost::optional<std::string>& value)
{
    generated::CPACSFuselage::SetParentUID(value);
    Invalidate();
}

// Get section count
int CCPACSFuselage::GetSectionCount() const
{
    return m_sections.GetSectionCount();
}

// Returns the face that has a given fuselage section as its boundary
TopoDS_Shape CCPACSFuselage::GetSectionFace(const std::string section_uid) const
{
    // search for the section in all segments
    for (int n = 0; n < GetSegmentCount(); ++n) {
        const CCPACSFuselageSegment& segment = GetSegment(n+1);

        if ( section_uid == segment.GetStartSectionUID() ) {
            return BuildFace( segment.GetStartWire() );
        }
        else if ( section_uid == segment.GetEndSectionUID() ) {
            return BuildFace( segment.GetEndWire() );
        }
    }
    throw CTiglError("GetSectionFace: Could not find a fuselage section for the given UID");
    return TopoDS_Shape();
}

// Returns the section for a given index
CCPACSFuselageSection& CCPACSFuselage::GetSection(int index) const
{
    return m_sections.GetSection(index);
}

// Get segment count
int CCPACSFuselage::GetSegmentCount() const
{
    return m_segments.GetSegmentCount();
}

// Returns the segment for a given index
CCPACSFuselageSegment& CCPACSFuselage::GetSegment(const int index)
{
    return m_segments.GetSegment(index);
}

const CCPACSFuselageSegment& CCPACSFuselage::GetSegment(const int index) const
{
    return m_segments.GetSegment(index);
}

// Returns the segment for a given uid
CCPACSFuselageSegment& CCPACSFuselage::GetSegment(std::string uid)
{
    return m_segments.GetSegment(uid);
}

// get short name for loft
std::string CCPACSFuselage::GetShortShapeName () const
{
    unsigned int findex = 0;
    for (int i = 1; i <= GetConfiguration().GetFuselageCount(); ++i) {
        tigl::CCPACSFuselage& f = GetConfiguration().GetFuselage(i);
        if (GetUID() == f.GetUID()) {
            findex = i;
            std::stringstream shortName;
            shortName << "F" << findex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

void CCPACSFuselage::SetFaceTraits (PNamedShape loft) const
{
    int nFacesTotal = GetNumberOfFaces(loft->Shape());
    int nFacesAero = nFacesTotal;
    bool hasSymmetryPlane = GetNumberOfEdges(m_segments.GetSegment(1).GetEndWire()) > 1;

    std::vector<std::string> names;
    names.push_back(loft->Name());
    names.push_back("symmetry");
    names.push_back("Front");
    names.push_back("Rear");

    if (!CTiglTopoAlgorithms::IsDegenerated(GetSegment(1).GetStartWire())) {
          nFacesAero-=1;
    }
    if (!CTiglTopoAlgorithms::IsDegenerated(GetSegment(GetSegmentCount()).GetEndWire())) {
          nFacesAero-=1;
    }

    // if we have a smooth surface, the whole fuslage is treatet as one segment
    int nSegments = this->GetSegmentCount();

    assert(nFacesAero >= nSegments);
    assert(nFacesAero % nSegments == 0);
    int facesPerSegment = nFacesAero/ nSegments;

    int iFaceTotal = 0;
    int nSymmetryFaces = (int) hasSymmetryPlane;
    for (int iSegment = 0; iSegment < nSegments; ++iSegment) {
        for (int iFace = 0; iFace < facesPerSegment - nSymmetryFaces; ++iFace) {
            loft->FaceTraits(iFaceTotal++).SetName(names[0].c_str());
        }
        for (int iFace = 0; iFace < nSymmetryFaces; ++iFace) {
            loft->FaceTraits(iFaceTotal++).SetName(names[1].c_str());
        }
    }

    // set the caps
    int iFace = 2;
    for (;iFaceTotal < nFacesTotal; ++iFaceTotal) {
        assert(iFace < names.size());
        loft->FaceTraits(iFaceTotal).SetName(names[iFace++].c_str());
    }
}

// Builds a fused shape of all fuselage segments
PNamedShape CCPACSFuselage::BuildLoft() const
{
    if (!GetConfiguration().HasDucts()) {
        return *cleanLoft;
    }

    return GetConfiguration().GetDucts()->LoftWithDuctCutouts(*cleanLoft, GetUID());
}

void CCPACSFuselage::BuildCleanLoft(PNamedShape& cache) const
{
    TiglContinuity cont = m_segments.GetSegment(1).GetContinuity();
    Standard_Boolean smooth = (cont == ::C0? false : true);

    CTiglMakeLoft lofter;
#ifdef TIGL_GORDON_SURFACE
    lofter.setAlgorithm(CTiglMakeLoft::Algorithm::GORDON_SURFACE);
#endif
    // add profiles
    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        lofter.addProfiles(m_segments.GetSegment(i).GetStartWire());
    }
    lofter.addProfiles(m_segments.GetSegment(m_segments.GetSegmentCount()).GetEndWire());

    // add guides
    lofter.addGuides(m_segments.GetGuideCurveWires());

    lofter.setMakeSolid(true);
    lofter.setMakeSmooth(smooth);

    TopoDS_Shape loftShape =  lofter.Shape();

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    cache = std::make_shared<CNamedShape>(loftShape, loftName.c_str(), loftShortName.c_str());
    SetFaceTraits(cache);
}

// Get the positioning transformation for a given section index
boost::optional<CTiglTransformation> CCPACSFuselage::GetPositioningTransformation(const std::string &sectionUID)
{
    boost::optional<CTiglTransformation> ret;
    if (m_positionings)
        ret = m_positionings->GetPositioningTransformation(sectionUID);
    return ret;
}

// Gets a point on the given fuselage segment in dependence of a parameters eta and zeta with
// 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start
// profile of the segment, for eta = 1.0 on the end profile of the segment. For zeta = 0.0
// the point is the start point of the profile wire, for zeta = 1.0 the last profile wire point.
gp_Pnt CCPACSFuselage::GetPoint(int segmentIndex, double eta, double zeta)
{
    return ((CCPACSFuselageSegment &) GetSegment(segmentIndex)).GetPoint(eta, zeta, getPointBehavior);
}

// Sets the GetPoint behavior to asParameterOnSurface or onLinearLoft
void CCPACSFuselage::SetGetPointBehavior(TiglGetPointBehavior behavior)
{
    getPointBehavior = behavior;
}

// Gets the getPointBehavior
TiglGetPointBehavior CCPACSFuselage::GetGetPointBehavior() const
{
    return getPointBehavior;
}


// Returns the volume of this fuselage
double CCPACSFuselage::GetVolume()
{
    const TopoDS_Shape fusedSegments = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Returns the circumference of the segment "segmentIndex" at a given eta
double CCPACSFuselage::GetCircumference(const int segmentIndex, const double eta)
{
    return static_cast<CCPACSFuselageSegment&>(GetSegment(segmentIndex)).GetCircumference(eta);
}
    
// Returns the surface area of this fuselage
double CCPACSFuselage::GetSurfaceArea()
{
    const PNamedShape& fusedSegments = GetLoft();

    // loop over all faces that are not symmetry, front or rear
    double myArea = 0.;

    TopTools_IndexedMapOfShape shapeMap;
    TopExp::MapShapes(fusedSegments->Shape(), TopAbs_FACE, shapeMap);
    for (int i = 1; i <= shapeMap.Extent(); ++i) {
        if (GetUID() == fusedSegments->GetFaceTraits(i-1).Name()) {
            const TopoDS_Face& f = TopoDS::Face(shapeMap(i));
            GProp_GProps System;
            BRepGProp::SurfaceProperties(f, System);
            myArea += System.Mass();
        }
    }

    // Calculate surface area
    return myArea;
}

// Returns the point where the distance between the selected fuselage and the ground is at minimum.
// The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
gp_Pnt CCPACSFuselage::GetMinumumDistanceToGround(gp_Ax1 RAxis, double angle)
{

    TopoDS_Shape fusedFuselage = GetLoft()->Shape();

    // now rotate the fuselage
    gp_Trsf myTrsf;
    myTrsf.SetRotation(RAxis, angle * M_PI / 180.);
    BRepBuilderAPI_Transform xform(fusedFuselage, myTrsf);
    fusedFuselage = xform.Shape();

    // build cutting plane for intersection
    // We move the "ground" to "-1000" to be sure it is _under_ the fuselage
    gp_Pnt p1(-1.0e7, -1.0e7, -1000);
    gp_Pnt p2( 1.0e7, -1.0e7, -1000);
    gp_Pnt p3( 1.0e7,  1.0e7, -1000);
    gp_Pnt p4(-1.0e7,  1.0e7, -1000);

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

    // calculate extrema
    BRepExtrema_DistShapeShape extrema(fusedFuselage, shaft_face);
    extrema.Perform();

    return extrema.PointOnShape1(1);
}

// Get the guide curve with a given UID
CCPACSGuideCurve& CCPACSFuselage::GetGuideCurveSegment(std::string uid)
{
    return const_cast<CCPACSGuideCurve&>(static_cast<const CCPACSFuselage&>(*this).GetGuideCurveSegment(uid));
}

const CCPACSGuideCurve& CCPACSFuselage::GetGuideCurveSegment(std::string uid) const
{
    for (int i = 1; i <= m_segments.GetSegmentCount(); i++) {
        const CCPACSFuselageSegment& segment = m_segments.GetSegment(i);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        if (segment.GetGuideCurves()->GuideCurveExists(uid)) {
            return segment.GetGuideCurves()->GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

std::vector<gp_Pnt> CCPACSFuselage::GetGuideCurvePoints() const
{
    std::vector<gp_Pnt> points;

    // connect the belonging guide curve segments
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        const CCPACSFuselageSegment& segment = m_segments.GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        const CCPACSGuideCurves& segmentCurves = *segment.GetGuideCurves();
        for (int iguide = 1; iguide <=  segmentCurves.GetGuideCurveCount(); ++iguide) {
            const CCPACSGuideCurve& curve = segmentCurves.GetGuideCurve(iguide);
            std::vector<gp_Pnt> curPoints = curve.GetCurvePoints();
            points.insert(points.end(), curPoints.begin(), curPoints.end());
        }
    }
    return points;
}



TopoDS_Shape transformFuselageProfileGeometry(const CTiglTransformation& fuselTransform, const CTiglFuselageConnection& connection, const TopoDS_Shape& shape)
{
    // Do section element transformation on points
    tigl::CTiglTransformation trafo = connection.GetSectionElementTransformation();

    // Do section transformations
    trafo.PreMultiply(connection.GetSectionTransformation());

    // Do positioning transformations
    boost::optional<CTiglTransformation> posTrans = connection.GetPositioningTransformation();
    if (posTrans) {
        trafo.PreMultiply(*posTrans);
    }

    trafo.PreMultiply(fuselTransform);

    return trafo.Transform(shape);

}

} // end namespace tigl
