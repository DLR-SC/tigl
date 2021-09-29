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

#include "ListFunctions.h"
#include <map>
#include "CCPACSFuselageSectionElement.h"

namespace tigl
{

CCPACSFuselage::CCPACSFuselage(CCPACSFuselages* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselage(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
    , guideCurves(*this, &CCPACSFuselage::BuildGuideCurves) {
    Cleanup();
    if (parent->IsParent<CCPACSAircraftModel>())
        configuration = &parent->GetParent<CCPACSAircraftModel>()->GetConfiguration();
    else
        configuration = &parent->GetParent<CCPACSRotorcraftModel>()->GetConfiguration();
}

// Destructor
CCPACSFuselage::~CCPACSFuselage()
{
    Cleanup();
}

// Invalidates internal state
void CCPACSFuselage::InvalidateImpl(const boost::optional<std::string>& source) const
{
    loft.clear();
    guideCurves.clear();
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

PNamedShape CCPACSFuselage::GetLoft(TiglCoordinateSystem cs) const
{
    PNamedShape loft = CTiglRelativelyPositionedComponent::GetLoft();
    if (!loft) {
        return loft;
    }

    if (cs == GLOBAL_COORDINATE_SYSTEM) {
        return loft;
    }
    else {
        // we want to modify the shape. we have to create a copy first
        loft = loft->DeepCopy();
        TopoDS_Shape transformedLoft = GetTransformationMatrix().Inverted().Transform(loft->Shape());
        loft->SetShape(transformedLoft);
        return loft;
    }
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
        loft->FaceTraits(iFaceTotal).SetName(names[iFace++].c_str());
    }
}

// Builds a fused shape of all fuselage segments
PNamedShape CCPACSFuselage::BuildLoft() const
{
    TiglContinuity cont = m_segments.GetSegment(1).GetContinuity();
    Standard_Boolean smooth = (cont == ::C0? false : true);

    CTiglMakeLoft lofter;
    // add profiles
    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        lofter.addProfiles(m_segments.GetSegment(i).GetStartWire());
    }
    lofter.addProfiles(m_segments.GetSegment(m_segments.GetSegmentCount()).GetEndWire());

    // add guides
    lofter.addGuides(GetGuideCurveWires());

    lofter.setMakeSolid(true);
    lofter.setMakeSmooth(smooth);

    TopoDS_Shape loftShape =  lofter.Shape();

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft);

    return loft;
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
TiglGetPointBehavior const CCPACSFuselage::GetGetPointBehavior() const
{
    return getPointBehavior;
}

// Gets the getPointBehavior
TiglGetPointBehavior CCPACSFuselage::GetGetPointBehavior()
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

const TopoDS_Compound &CCPACSFuselage::GetGuideCurveWires() const
{
    return *guideCurves;
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

gp_Lin CCPACSFuselage::Intersection(gp_Pnt pRef, double angleRef) const
{
    // to have a left-handed coordinates system for the intersection computation (see documentation)
    const gp_Ax1 xAxe(pRef, gp_Dir(1, 0, 0));
    const gp_Dir zReference(0, 0, 1);
    const gp_Dir angleDir = zReference.Rotated(xAxe, angleRef);

    // build a line to position the intersection with the fuselage shape
    gp_Lin line(pRef, angleDir);

    // fuselage loft
    const TopoDS_Shape loft = GetLoft(FUSELAGE_COORDINATE_SYSTEM)->Shape();

    // get the list of shape from the fuselage shape
    TopExp_Explorer exp;
    for (exp.Init(loft, TopAbs_FACE); exp.More(); exp.Next()) {
        IntCurvesFace_Intersector intersection(TopoDS::Face(exp.Current()), 0.1); // intersection builder
        intersection.Perform(line, 0, std::numeric_limits<Standard_Real>::max());
        if (intersection.IsDone() && intersection.NbPnt() > 0) {
            gp_Lin result(intersection.Pnt(1), line.Direction());
            // return the line with the point on the fuselage as the origin, and the previous line's direction
            return result;
        }
    }

    TRACE_POINT(debug);
    debug.dumpShape(loft, "loft");
    debug.dumpShape(BRepBuilderAPI_MakeEdge(pRef, pRef.XYZ() + angleDir.XYZ() * 1000), "line");

    throw std::logic_error("Error computing intersection line");
}

gp_Lin CCPACSFuselage::Intersection(const CCPACSFuselageStringerFramePosition& pos) const
{
    const gp_Pnt pRef        = pos.GetRefPoint();
    const double angleRefRad = (M_PI / 180.) * pos.GetReferenceAngle();
    return Intersection(pRef, angleRefRad);
}

namespace
{
    TopoDS_Wire project(TopoDS_Shape wireOrEdge, BRepProj_Projection& proj, DebugScope& debug)
    {
        BRepBuilderAPI_MakeWire wireBuilder;
        for (; proj.More(); proj.Next())
            wireBuilder.Add(proj.Current());

        TopTools_ListOfShape wireList;
        BuildWiresFromConnectedEdges(proj.Shape(), wireList);

        if (wireList.Extent() == 0) {
            debug.addShape(proj.Shape(), "projection");

            throw CTiglError("Projection returned no wires");
        }
        if (wireList.Extent() == 1)
            return TopoDS::Wire(wireList.First());

        // select the wire which is closest to the wire we projected
        for (TopTools_ListIteratorOfListOfShape it(wireList); it.More(); it.Next()) {
            const TopoDS_Wire w                = TopoDS::Wire(it.Value());
            const gp_Pnt wStart     = GetFirstPoint(w);
            const gp_Pnt wEnd       = GetLastPoint(w);
            const gp_Pnt inputStart = GetFirstPoint(wireOrEdge);
            const gp_Pnt inputEnd   = GetLastPoint(wireOrEdge);

            const double pointEqualEpsilon = 1e-4;
            if ((wStart.IsEqual(inputStart, pointEqualEpsilon) && wEnd.IsEqual(inputEnd, pointEqualEpsilon)) ||
                (wEnd.IsEqual(inputStart, pointEqualEpsilon) && wStart.IsEqual(inputEnd, pointEqualEpsilon))) {
                return w;
            }
        }

        TopoDS_Compound c;
        TopoDS_Builder b;
        b.MakeCompound(c);
        for (TopTools_ListIteratorOfListOfShape it(wireList); it.More(); it.Next()) {
            b.Add(c, it.Value());
        }
        debug.addShape(proj.Shape(), "projection");
        debug.addShape(c, "wireList");

        // give up
        throw CTiglError("Failed to project wire/edge onto fuselage");
    }
}

TopoDS_Wire CCPACSFuselage::projectConic(TopoDS_Shape wireOrEdge, gp_Pnt origin) const
{
    const TopoDS_Shape loft = GetLoft(FUSELAGE_COORDINATE_SYSTEM)->Shape();

    DEBUG_SCOPE(debug);
    debug.addShape(wireOrEdge, "wireOrEdge");
    debug.addShape(loft, "loft");
    debug.addShape(BRepBuilderAPI_MakeVertex(origin), "origin");

    BRepProj_Projection proj(wireOrEdge, loft, origin);
    return project(wireOrEdge, proj, debug);
}

TopoDS_Wire CCPACSFuselage::projectParallel(TopoDS_Shape wireOrEdge, gp_Dir direction) const
{
    const TopoDS_Shape loft = GetLoft(FUSELAGE_COORDINATE_SYSTEM)->Shape();

    const TopoDS_Shape directionLine = BRepBuilderAPI_MakeEdge(
        BRepBuilderAPI_MakeVertex(gp_Pnt(0, 0, 0)).Vertex(),
        BRepBuilderAPI_MakeVertex(gp_Pnt(direction.XYZ() * 1000)).Vertex()
    ).Shape();

    DEBUG_SCOPE(debug);
    debug.addShape(wireOrEdge, "wireOrEdge");
    debug.addShape(loft, "loft");
    debug.addShape(directionLine, "direction");

    BRepProj_Projection proj(wireOrEdge, loft, direction);
    return project(wireOrEdge, proj, debug);
}

void CCPACSFuselage::BuildGuideCurves(TopoDS_Compound& cache) const
{
    std::map<double, const CCPACSGuideCurve*> roots;

    // get section centers for the centripetal parametrization
    std::vector<gp_Pnt> sectionCenters(GetSegmentCount()+1);

    // get center of inner section of first segment
    const CCPACSFuselageSegment& innerSegment = m_segments.GetSegment(1);
    sectionCenters[0] = innerSegment.GetTransformedProfileOriginStart();
    
    // find roots and connect the belonging guide curve segments
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        const CCPACSFuselageSegment& segment = m_segments.GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        // get center of outer section
        sectionCenters[isegment] = segment.GetTransformedProfileOriginEnd();

        const CCPACSGuideCurves& segmentCurves = *segment.GetGuideCurves();
        for (int iguide = 1; iguide <=  segmentCurves.GetGuideCurveCount(); ++iguide) {
            const CCPACSGuideCurve& curve = segmentCurves.GetGuideCurve(iguide);
            if (!curve.GetFromGuideCurveUID_choice1()) {
                // this is a root curve
                double relCirc= *curve.GetFromRelativeCircumference_choice2();
                //TODO: determine if half fuselage or not. If not
                //the guide curve at relCirc=1 should be inserted at relCirc=0
                roots.insert(std::make_pair(relCirc, &curve));
            }
        }
    }

    // get the parameters at the section centers
    std::vector<double> sectionParams = CTiglBSplineAlgorithms::computeParamsBSplineCurve(OccArray(sectionCenters), 0., 1., 0.5);

    // connect guide curve segments to a spline with given continuity conditions and tangents
    CTiglCurveConnector connector(roots, sectionParams);
    cache = connector.GetConnectedGuideCurves();
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

std::string CCPACSFuselage::GetNoseUID()
{
    // todo check error?
    return GetSegment(1).GetStartSectionElementUID();
}

CTiglPoint CCPACSFuselage::GetNoseCenter()
{
    std::string noiseUID                  = GetNoseUID();
    CTiglFuselageSectionElement* cElement = m_sections.GetCTiglElements()[noiseUID];
    return cElement->GetCenter(TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM);
}

void CCPACSFuselage::SetNoseCenter(const tigl::CTiglPoint &newCenter)
{
    CTiglPoint oldCenter                         = GetNoseCenter();
    CTiglPoint delta                             = newCenter - oldCenter;
    CCPACSTransformation& fuselageTransformation = GetTransformation();
    CTiglPoint currentTranslation                = fuselageTransformation.getTranslationVector();
    fuselageTransformation.setTranslation(currentTranslation + delta);
    Invalidate();
}

void CCPACSFuselage::SetRotation(const CTiglPoint& newRot)
{
    CCPACSTransformation& transformation = GetTransformation();
    transformation.setRotation(newRot);
    Invalidate();
}

std::string CCPACSFuselage::GetTailUID()
{
    return GetSegment(GetSegmentCount()).GetEndSectionElementUID();
}

std::vector<std::string> CCPACSFuselage::GetElementUIDsInOrder()
{
    return m_segments.GetElementUIDsInOrder();
}

double CCPACSFuselage::GetLength()
{
    // todo need to take care of the case where not segment are there?
    std::string noiseUID = GetNoseUID();
    std::string tailUID  = GetTailUID();
    return GetLengthBetween(noiseUID, tailUID);
}

double CCPACSFuselage::GetLengthBetween(const std::string& startElementUID, const std::string& endElementUID)
{
    CCPACSFuselageSectionElement& startElement = GetUIDManager().ResolveObject<CCPACSFuselageSectionElement>(startElementUID);
    CTiglPoint startCenter = startElement.GetCTiglSectionElement()->GetCenter();

    CCPACSFuselageSectionElement& endElement = GetUIDManager().ResolveObject<CCPACSFuselageSectionElement>(endElementUID);
    CTiglPoint endCenter = endElement.GetCTiglSectionElement()->GetCenter();

    CTiglPoint delta = endCenter - startCenter;
    return delta.norm2();
}

void CCPACSFuselage::SetLength(double newLength)
{
    std::string noise = GetNoseUID();
    std::string tail  = GetTailUID();
    SetLengthBetween(noise, tail, newLength);
}

void CCPACSFuselage::SetLengthBetween(const std::string& startElementUID, const std::string& endElementUID,
                                      double newPartialLength)
{

    // Prepare the required staff

    std::vector<std::string> elementUIDs = m_segments.GetElementUIDsInOrder();
    std::map<std::string, CTiglFuselageSectionElement*> ctiglElementsMap = m_sections.GetCTiglElements();

    std::map<std::string, CTiglPoint> oldCenterPoints;
    std::map<std::string, CTiglPoint> newCenterPoints;

    std::string tempElementUID = "";

    for (int i = 0; i < elementUIDs.size(); i++) {
        tempElementUID = elementUIDs[i];
        oldCenterPoints[tempElementUID]  = ctiglElementsMap[tempElementUID]->GetCenter();
    }

    // Divide the elements in 3 categories:
    // 1) Elements before start that need not to be modified
    // 2) Elements between that need to create the partial length
    // 3) Elements after end that need to be shifted has the last between element

    std::vector<std::string> elementsBetween =
        ListFunctions::GetElementsInBetween(elementUIDs, startElementUID, endElementUID);
    std::vector<std::string> elementsAfter = ListFunctions::GetElementsAfter(elementUIDs, endElementUID);

    if (elementsBetween.size() < 2) {
        throw CTiglError(
            "CCPACSFuselage::SetLengthBetween: At least two elements should be contains between stratUID and endUID");
    }

    // Prepare the transformation to apply

    //    BETWEEN ELEMENT SCALING

    //
    //              This part follow basically these steps:
    //
    //              1)  Computation of the affine transformations needed to perform the desired effect.
    //                  The desired effect can be perform as:
    //                      a) Put the start center point on the world origin
    //                      b) Rotation to get the end center on the X axis
    //                      c) Perform a scaling on X to obtain the desired length value
    //                      d) inverse of of b) to put the fuselage in the right direction
    //                      e) inverse of a) to shift the fuselage to its origin place
    //
    //
    //              2) Compute the new center point for each element

    CTiglPoint startP = oldCenterPoints[startElementUID];
    CTiglPoint endP   = oldCenterPoints[endElementUID];

    // bring StartP to Origin
    CTiglTransformation startToO;
    startToO.SetIdentity();
    startToO.AddTranslation(-startP.x, -startP.y, -startP.z);

    startP = startToO * startP;
    endP   = startToO * endP;

    // bring endP on the x axis
    // We perform a extrinsic rotation in the order Z -Y -X, so it should be equivalent to the intrinsic cpacs rotation
    // in the order X Y' Z''
    CTiglTransformation rotEndToX4d;
    rotEndToX4d.SetIdentity();
    double rotGradZ = atan2(endP.y, endP.x);
    double rotZ     = CTiglTransformation::RadianToDegree(rotGradZ);
    rotEndToX4d.AddRotationZ(-rotZ);
    double rotGradY = atan2(endP.z, sqrt((endP.x * endP.x) + (endP.y * endP.y)));
    double rotY     = CTiglTransformation::RadianToDegree(rotGradY);
    rotEndToX4d.AddRotationY(rotY);

    endP                    = rotEndToX4d * endP;
    double oldPartialLength = GetLengthBetween(startElementUID, endElementUID);

    // Compute the needed scaling in x
    if (oldPartialLength == 0) {
        // todo cover the case where length is 0
        throw CTiglError("CCPACSFuselage::SetLengthBetween: the old length is 0, impossible to scale the length");
    }

    double xScale = newPartialLength / oldPartialLength;
    CTiglTransformation scaleM;
    scaleM.SetIdentity();
    scaleM.AddScaling(xScale, 1.0, 1.0);

    // Compute the new center point and the new origin of each element in Between
    CTiglTransformation totalTransformation =
        startToO.Inverted() * rotEndToX4d.Inverted() * scaleM * rotEndToX4d * startToO;
    CTiglPoint tempDelatOtoP;
    for (int i = 0; i < elementsBetween.size(); i++) {
        newCenterPoints[elementsBetween[i]] = totalTransformation * oldCenterPoints.at(elementsBetween[i]);
    }


    // SHIFT THE END OF THE FUSELAGE

    CTiglPoint shiftEndElementG = newCenterPoints[endElementUID] - oldCenterPoints[endElementUID];
    for (int i = 0; i < elementsAfter.size(); i++) {
        newCenterPoints[elementsAfter[i]] = oldCenterPoints[elementsAfter[i]] + shiftEndElementG;
    }

    // SET THE NEW CENTERS OF EACH ELEMENT

    std::map<std::string, CTiglPoint>::iterator it;
    for ( it = newCenterPoints.begin(); it != newCenterPoints.end(); it++ )
    {
        ctiglElementsMap[it->first]->SetCenter(it->second);  // will call Invalidate();
    }

    // Remark the saving in tixi is not done, it should be perform by the user using "WriteCPACS" function
}

double CCPACSFuselage::GetMaximalCircumference()
{
    return GetMaximalCircumferenceBetween(GetNoseUID(), GetTailUID());
}

double CCPACSFuselage::GetMaximalCircumferenceBetween(const std::string& startElementUID,
                                                      const std::string& endElementUID)
{
    return GetMaxBetween(&CTiglFuselageSectionElement::GetCircumference, startElementUID, endElementUID);
}

void CCPACSFuselage::SetMaximalCircumference(double newMaximalCircumference)
{
    SetMaximalCircumferenceBetween(GetNoseUID(), GetTailUID(), newMaximalCircumference);
}

void CCPACSFuselage::SetMaximalCircumferenceBetween(const std::string& startElementUID,
                                                    const std::string& endElementUID, double newMaximalCircumference)
{

    if (newMaximalCircumference < 0) {
        throw CTiglError("CCPACSFuselage::SetMaximalCircumferenceBetween: Circumference should be bigger than 0");
    }

    double oldMaximalCircumference = GetMaximalCircumferenceBetween(startElementUID, endElementUID);

    // todo manage the 0 scale case
    double scaleFactor = newMaximalCircumference / oldMaximalCircumference;

    std::vector<std::string> elementsBetween =
        ListFunctions::GetElementsInBetween(m_segments.GetElementUIDsInOrder(), startElementUID, endElementUID);

    if (elementsBetween.size() < 1) {
        throw CTiglError("CCPACSFuselage::GetMaximalCircumferenceBetween: No elements in between was found");
    }

    std::map<std::string, CTiglFuselageSectionElement*> cTiglElementsMap = m_sections.GetCTiglElements();
    for (int i = 0; i < elementsBetween.size(); i++) {
        cTiglElementsMap[elementsBetween[i]]->ScaleCircumference(scaleFactor);
    }
}

double CCPACSFuselage::GetMaximalHeightBetween(const std::string& startElementUID, const std::string& endElementUID)
{
    return GetMaxBetween(&CTiglFuselageSectionElement::GetHeight, startElementUID, endElementUID);
}

double CCPACSFuselage::GetMaximalHeight()
{
    return GetMaximalHeightBetween(GetNoseUID(), GetTailUID());
}

double CCPACSFuselage::GetMaximalWidthBetween(const std::string& startElementUID, const std::string& endElementUID)
{
    return GetMaxBetween(&CTiglFuselageSectionElement::GetWidth, startElementUID, endElementUID);
}

double CCPACSFuselage::GetMaximalWidth()
{
    return GetMaximalWidthBetween(GetNoseUID(), GetTailUID());
}

double CCPACSFuselage::GetMaximalWireAreaBetween(const std::string& startElementUID, const std::string& endElementUID)
{
    return GetMaxBetween(&CTiglFuselageSectionElement::GetArea, startElementUID, endElementUID);
}

double CCPACSFuselage::GetMaximalWireArea()
{
    return GetMaximalWireAreaBetween(GetNoseUID(), GetTailUID());
}

double CCPACSFuselage::GetMaxBetween(pGetProperty func, const std::string& startElementUID,
                                     const std::string& endElementUID)
{
    std::vector<std::string> elementUIDs                                 = m_segments.GetElementUIDsInOrder();
    std::map<std::string, CTiglFuselageSectionElement*> cTiglElementsMap = m_sections.GetCTiglElements();
    std::vector<std::string> elementsInBetween =
        ListFunctions::GetElementsInBetween(elementUIDs, startElementUID, endElementUID);

    if (elementsInBetween.size() < 1) {
        LOG(WARNING) << "CCPACSFuselage::GetMaxBetween: No elements in between was found!";
    }

    double max = -1;
    double tempValue;
    for (int i = 0; i < elementsInBetween.size(); i++) {
        CTiglFuselageSectionElement temp = (*(cTiglElementsMap[elementsInBetween[i]]));
        tempValue                        = (temp.*func)(GLOBAL_COORDINATE_SYSTEM);
        if (tempValue > max) {
            max = tempValue;
        }
    }

    return max;
}

void CCPACSFuselage::ScaleWiresUniformly(double scaleFactor)
{
    ScaleWiresUniformlyBetween(scaleFactor, GetNoseUID(), GetTailUID());
}

void CCPACSFuselage::ScaleWiresUniformlyBetween(double scaleFactor, const std::string& startElementUID,
                                                const std::string& endElementUID)
{
    if (scaleFactor < 0) {
        throw CTiglError(
            " CCPACSFuselage::ScaleWiresUniformlyBetween: For the moment only positive scale factor are supported.");
    }

    pSetProperty func = &CTiglFuselageSectionElement::ScaleUniformly;
    ApplyFunctionBetween(func, scaleFactor, startElementUID, endElementUID);
}

void CCPACSFuselage::SetMaxHeight(double newMaxHeight)
{
    SetMaxHeightBetween(newMaxHeight, GetNoseUID(), GetTailUID());
}

void CCPACSFuselage::SetMaxHeightBetween(double newMaxHeight, const std::string& startUID, const std::string& endUID)
{
    double oldHeight = GetMaximalHeightBetween(startUID, endUID);
    if (fabs(oldHeight) < 0.0001) {
        // in this case we will call setHeight on each element because a scaling is impossible
        // -> all the height of the element will be the same
        pSetProperty func = &CTiglFuselageSectionElement::SetHeight;
        ApplyFunctionBetween(func, newMaxHeight, startUID, endUID);
    }
    else {
        // in this scale we perform a uniform scaling on the wires to keep the shape
        double scaleFactor = newMaxHeight / oldHeight;
        ScaleWiresUniformlyBetween(scaleFactor, startUID, endUID);
    }
}

void CCPACSFuselage::SetMaxWidth(double newMaxWidth)
{
    SetMaxHeightBetween(newMaxWidth, GetNoseUID(), GetTailUID());
}

void CCPACSFuselage::SetMaxWidthBetween(double newMaxWidth, const std::string& startUID, const std::string& endUID)
{
    double oldMaxWidth = GetMaximalWidthBetween(startUID, endUID);
    if (fabs(oldMaxWidth) < 0.0001) {
        // in this case we will call setWidth on each element because a scaling is impossible
        // -> all the width of the element will be the same
        pSetProperty func = &CTiglFuselageSectionElement::SetWidth;
        ApplyFunctionBetween(func, newMaxWidth, startUID, endUID);
    }
    else {
        // in this scale we perform a uniform scaling on the wires to keep the shape
        double scaleFactor = newMaxWidth / oldMaxWidth;
        ScaleWiresUniformlyBetween(scaleFactor, startUID, endUID);
    }
}

void CCPACSFuselage::SetMaxArea(double newMaxArea)
{
    SetMaxAreaBetween(newMaxArea, GetNoseUID(), GetTailUID());
}

void CCPACSFuselage::SetMaxAreaBetween(double newMaxArea, const std::string& startUID, const std::string& endUID)
{
    double oldArea = GetMaximalWireAreaBetween(startUID, endUID);
    if (fabs(oldArea) < 0.0001) {
        // in this case we will call setArea on each element because a scaling is impossible
        // -> all the area of the element will be the same
        pSetProperty func = &CTiglFuselageSectionElement::SetArea;
        ApplyFunctionBetween(func, newMaxArea, startUID, endUID);
    }
    else {
        // in this scale we perform a uniform scaling on the wires to keep the shape
        double scaleFactor = sqrt(newMaxArea / oldArea);
        ScaleWiresUniformlyBetween(scaleFactor, startUID, endUID);
    }
}

double CCPACSFuselage::ApplyFunctionBetween(pSetProperty func, double value, const std::string& startElementUID,
                                            const std::string& endElementUID)
{
    std::vector<std::string> elementUIDs                                 = m_segments.GetElementUIDsInOrder();
    std::map<std::string, CTiglFuselageSectionElement*> cTiglElementsMap = m_sections.GetCTiglElements();
    std::vector<std::string> elementsInBetween =
        ListFunctions::GetElementsInBetween(elementUIDs, startElementUID, endElementUID);

    if (elementsInBetween.size() < 1) {
        LOG(WARNING) << "CCPACSFuselage::GetMaxBetween: No elements in between was found!";
    }

    for (int i = 0; i < elementsInBetween.size(); i++) {
        CTiglFuselageSectionElement temp = (*(cTiglElementsMap[elementsInBetween[i]]));
        (temp.*func)(value, GLOBAL_COORDINATE_SYSTEM);
    }
}

} // end namespace tigl
