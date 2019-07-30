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
#include "CCPACSFuselageSection.h"
#include "ListFunctions.h"
#include "tiglmathfunctions.h"


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
#include "CTiglFuselageHelper.h"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

namespace tigl
{

CCPACSFuselage::CCPACSFuselage(CCPACSFuselages* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselage(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
    , guideCurves(*this, &CCPACSFuselage::BuildGuideCurves)
    , fuselageHelper(*this, &CCPACSFuselage::SetFuselageHelper)
    {
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
void CCPACSFuselage::Invalidate()
{
    loft.clear();
    guideCurves.clear();
    m_segments.Invalidate();
    if (m_positionings)
        m_positionings->Invalidate();
    if (m_structure)
        m_structure->Invalidate();
    fuselageHelper.clear();
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

    ConnectGuideCurveSegments();
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

// Get section count
int CCPACSFuselage::GetSectionCount() const
{
    return m_sections.GetSectionCount();
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
    int nFaces = GetNumberOfFaces(loft->Shape());
    bool hasSymmetryPlane = GetNumberOfEdges(m_segments.GetSegment(1).GetEndWire()) > 1;

    std::vector<std::string> names;
    names.push_back(loft->Name());
    names.push_back("symmetry");
    names.push_back("Front");
    names.push_back("Rear");

    // if we have a smooth surface, the whole fuslage is treatet as one segment
    int nSegments = this->GetSegmentCount();

    // the number of faces per segment depends on the number of guide curves and the existence of the symmetry plane
    int facesPerSegment = GetSegment(1).GetNumberOfLoftFaces();
    int remainingFaces = nFaces - facesPerSegment * nSegments;
    if (facesPerSegment == 0 || remainingFaces < 0 || remainingFaces > 2) {
        LOG(WARNING) << "Fuselage faces cannot be names properly";
        return;
    }

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
    for (;iFaceTotal < nFaces; ++iFaceTotal) {
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
    return ((CCPACSFuselageSegment &) GetSegment(segmentIndex)).GetPoint(eta, zeta);
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

void CCPACSFuselage::ConnectGuideCurveSegments(void)
{
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        CCPACSFuselageSegment& segment = GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        CCPACSGuideCurves& curves = *segment.GetGuideCurves();
        for (int icurve = 1; icurve <= curves.GetGuideCurveCount(); ++icurve) {
            CCPACSGuideCurve& curve = curves.GetGuideCurve(icurve);
            if (!curve.GetFromRelativeCircumference_choice2()) {
                std::string fromUID = *curve.GetFromGuideCurveUID_choice1();
                CCPACSGuideCurve& fromCurve = GetGuideCurveSegment(fromUID);
                curve.SetFromRelativeCircumference_choice2(fromCurve.GetToRelativeCircumference());
            }
        }
    }
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


CTiglPoint CCPACSFuselage::GetNoseCenter()
{
    CTiglPoint center;
    if (! fuselageHelper->HasShape()) {  // fuselage has no element case
        return center;
    }
    std::string noiseUID                  = fuselageHelper->GetNoseUID();
    CTiglFuselageSectionElement* cElement = fuselageHelper->GetCTiglElementOfFuselage(noiseUID);
    center = cElement->GetCenter(TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM);
    return center;
}

void CCPACSFuselage::SetNoseCenter(const tigl::CTiglPoint &newCenter)
{
    // Remark, this method work even if the fuselage has no element, because we set the fuselage transformation
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

double CCPACSFuselage::GetLength()
{
    double length = 0;
    if (!fuselageHelper->HasShape()) { // fuselage has no element case
        return length;
    }
    CTiglFuselageSectionElement* nose = fuselageHelper->GetCTiglElementOfFuselage(fuselageHelper->GetNoseUID());
    CTiglFuselageSectionElement* tail = fuselageHelper->GetCTiglElementOfFuselage(fuselageHelper->GetTailUID());
    length = (tail->GetCenter(GLOBAL_COORDINATE_SYSTEM) - nose->GetCenter(GLOBAL_COORDINATE_SYSTEM)).norm2();
    return length;
}

void CCPACSFuselage::SetLength(double newLength)
{
    //
    //              To set the length, we basicaly follow these steps:
    //
    //              1)  Computation of the transformations needed to perform the desired effect.
    //                  The desired effect can be perform as:
    //                      a) Put the start center point on the world origin
    //                      b) Rotation to get the end center on the X axis
    //                      c) Perform a scaling on X to obtain the desired length value
    //                      d) inverse of of b) to put the fuselage in the right direction
    //                      e) inverse of a) to shift the fuselage to its origin place
    //
    //              2) Compute the new center point for each element using the previous transformation

    if (!fuselageHelper->HasShape()) {
        LOG(WARNING) << "PACSFuselage::SetLength: Impossible to set the length because this fuselage has no segments.";
        return;
    }

    std::vector<std::string> elementUIDS = fuselageHelper->GetElementUIDsInOrder();
    std::map<std::string, CTiglPoint> oldCenterPoints;

    // Retrieve the current center of each element
    for (int i = 0; i < elementUIDS.size(); i++) {
        oldCenterPoints[elementUIDS[i]] = fuselageHelper->GetCTiglElementOfFuselage(elementUIDS[i])->GetCenter();
    }

    CTiglPoint noseP = oldCenterPoints[fuselageHelper->GetNoseUID()];
    CTiglPoint tailP = oldCenterPoints[fuselageHelper->GetTailUID()];

    // bring noseP (aka Start) to Origin
    CTiglTransformation startToO;
    startToO.SetIdentity();
    startToO.AddTranslation(-noseP.x, -noseP.y, -noseP.z);

    noseP = startToO * noseP;
    tailP = startToO * tailP;

    // bring tailP on the x axis
    // We perform a extrinsic rotation in the order Z -Y -X, so it should be equivalent to the intrinsic cpacs rotation
    // in the order X Y' Z''
    CTiglTransformation rotEndToX4d;
    rotEndToX4d.SetIdentity();
    double rotGradZ = atan2(tailP.y, tailP.x);
    double rotZ     = Degrees(rotGradZ);
    rotEndToX4d.AddRotationZ(-rotZ);
    double rotGradY = atan2(tailP.z, sqrt((tailP.x * tailP.x) + (tailP.y * tailP.y)));
    double rotY     = Degrees(rotGradY);
    rotEndToX4d.AddRotationY(rotY);

    tailP = rotEndToX4d * tailP;

    // Compute the needed scaling in x
    double oldLength = GetLength();
    if (oldLength == 0) {
        // todo cover the case where length is 0
        throw CTiglError("CCPACSFuselage::SetLengthBetween: the old length is 0, impossible to scale the length");
    }
    double xScale = newLength / oldLength;
    CTiglTransformation scaleM;
    scaleM.SetIdentity();
    scaleM.AddScaling(xScale, 1.0, 1.0);

    // Compute the new center point and the new origin of each element and set it
    CTiglTransformation totalTransformation =
        startToO.Inverted() * rotEndToX4d.Inverted() * scaleM * rotEndToX4d * startToO;
    CTiglPoint newCenter;
    for (int i = 0; i < elementUIDS.size(); i++) {
        newCenter = totalTransformation * oldCenterPoints.at(elementUIDS[i]);
        fuselageHelper->GetCTiglElementOfFuselage(elementUIDS[i])->SetCenter(newCenter);
    }

    // Remark the saving in tixi is not done, it should be perform by the user using "WriteCPACS" function
}

double CCPACSFuselage::GetMaximalHeight()
{
    // Todo: evaluate the possiblity to use the a cache for this operation in fuselageHelper

    // First compute the rotation to bring the fuselage in the standard direction
    // We do not invert the fuselage transformation, because we want to keep the the scaling apply by it
    CTiglTransformation fuselageRot;
    fuselageRot.AddRotationIntrinsicXYZ(GetRotation().x,GetRotation().y, GetRotation().z) ;
    CTiglTransformation fuselageRotInv = fuselageRot.Inverted();

    // Then comput the loft in this coordinate system
    PNamedShape loftCopy = GetLoft()->DeepCopy(); // make a deep copy because we gonna to transform it
    TopoDS_Shape transformedLoft = fuselageRotInv.Transform(loftCopy->Shape());
    BRepMesh_IncrementalMesh mesh(transformedLoft, 0.01);   // tessellate the loft to have a more accurate bounding box.

    Bnd_Box boundingBox;
    BRepBndLib::Add(transformedLoft, boundingBox);
    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    return zmax - zmin;
}

void CCPACSFuselage::SetMaxHeight(double newHeight)
{
    // Remark the inverse rotation is needed to bring the fuselage in the "intutive" direction
    // And we need to bring the nose to the origin because otherwise the scaling will change the position of the fuselage
    // if the fuselage is not on the X axis.
    // Furthermore is impossible to use scaling of the fuselage, because sometimes the fuselage has already a position
    // before that the scaling of the fuselage is applied.
    // So for each transformation of element, E', we get the following equation
    //
    // FPSE' = R⁻¹*T⁻¹*S*T*R*FPSE   Where T is the translation from nose to origin
    //                                and R the inverse transformation of the rotation.


    CTiglPoint nose = GetNoseCenter();

    CTiglTransformation RI;     // fuselage rotation
    RI.AddRotationIntrinsicXYZ(GetRotation().x,GetRotation().y, GetRotation().z) ;
    CTiglTransformation R = RI.Inverted();

    nose = R*nose;  // nose after rotation apply on it
    CTiglTransformation T ;
    T.AddTranslation(- nose.x, -nose.y, -nose.z);
    CTiglTransformation TI = T.Inverted();

    double currentHeight = GetMaximalHeight();
    if (currentHeight < 0.00000000001 ){
        LOG(WARNING) << "CCPACSFuselage::SetMaxHeight: The current height is near 0, we do not support for the moment setting the height if the current height is zero, sorry.";
        return;
    }
    double scalingZ = newHeight / currentHeight;
    CTiglTransformation S;
    S.AddScaling(1,1,scalingZ);

    std::vector<std::string> elementUIDs =  fuselageHelper->GetElementUIDsInOrder();
    CTiglFuselageSectionElement* cElement = nullptr;
    CTiglTransformation newTotalTransformationForE;
    for (int i = 0; i < elementUIDs.size(); i++ ) {
        cElement = fuselageHelper->GetCTiglElementOfFuselage(elementUIDs[i]);
        newTotalTransformationForE = RI * TI * S * T * R * cElement->GetTotalTransformation();
        cElement->SetTotalTransformation(newTotalTransformationForE);
    }

    Invalidate();

}

double CCPACSFuselage::GetMaximalWidth()
{

    // First compute the rotation to bring the fuselage in the standard direction
    // We do not invert the fuselage transformation, because we want to keep the the scaling apply by it
    CTiglTransformation fuselageRot;
    fuselageRot.AddRotationIntrinsicXYZ(GetRotation().x,GetRotation().y, GetRotation().z) ;
    CTiglTransformation fuselageRotInv = fuselageRot.Inverted();

    // Then comput the loft in this coordinate system
    PNamedShape loftCopy = GetLoft()->DeepCopy(); // make a deep copy because we gonna to transform it
    TopoDS_Shape transformedLoft = fuselageRotInv.Transform(loftCopy->Shape());
    BRepMesh_IncrementalMesh mesh(transformedLoft, 0.01); // tessellate the loft to have a more accurate bounding box.

    Bnd_Box boundingBox;
    BRepBndLib::Add(transformedLoft, boundingBox);
    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    return ymax - ymin;
}

void CCPACSFuselage::SetMaxWidth(double newWidth)
{

    // Remark the inverse rotation is needed to bring the fuselage in the "intutive" direction
    // And we need to bring the nose to the origin because otherwise the scaling will change the position of the fuselage
    // if the fuselage is not on the X axis.
    // Furthermore is impossible to use scaling of the fuselage, because sometimes the fuselage has already a position
    // before that the scaling of the fuselage is applied.
    // So for each transformation of element, E', we get the following equation
    //
    // FPSE' = R⁻¹*T⁻¹*S*T*R*FPSE   Where T is the translation from nose to origin
    //                                and R the inverse transformation of the rotation.


    CTiglPoint nose = GetNoseCenter();

    CTiglTransformation RI;     // fuselage rotation
    RI.AddRotationIntrinsicXYZ(GetRotation().x,GetRotation().y, GetRotation().z) ;
    CTiglTransformation R = RI.Inverted();

    nose = R*nose;  // nose after rotation apply on it
    CTiglTransformation T ;
    T.AddTranslation(- nose.x, -nose.y, -nose.z);
    CTiglTransformation TI = T.Inverted();

    double currentWidth = GetMaximalWidth();
    if (currentWidth < 0.00000000001 ){
        LOG(WARNING) << "CCPACSFuselage::SetMaxHeight: The current height is near 0, we do not support for the moment setting the height if the current height is zero, sorry.";
        return;
    }
    double scalingY = newWidth / currentWidth;
    CTiglTransformation S;
    S.AddScaling(1,scalingY,1);

    std::vector<std::string> elementUIDs =  fuselageHelper->GetElementUIDsInOrder();
    CTiglFuselageSectionElement* cElement = nullptr;
    CTiglTransformation newTotalTransformationForE;
    for (int i = 0; i < elementUIDs.size(); i++ ) {
        cElement = fuselageHelper->GetCTiglElementOfFuselage(elementUIDs[i]);
        newTotalTransformationForE = RI * TI * S * T * R * cElement->GetTotalTransformation();
        cElement->SetTotalTransformation(newTotalTransformationForE);
    }

    Invalidate();
}

void CCPACSFuselage::SetFuselageHelper(CTiglFuselageHelper& cache) const
{
    cache.SetFuselage(const_cast<CCPACSFuselage*>(this));
}


void CCPACSFuselage::CreateNewConnectedElementBetween(std::string startElementUID, std::string endElementUID)
{

    std::string segmentToSplit = GetSegments().GetSegmentFromTo(startElementUID, endElementUID).GetUID();
    CTiglFuselageSectionElement* startElement = fuselageHelper->GetCTiglElementOfFuselage(startElementUID);
    CTiglFuselageSectionElement* endElement = fuselageHelper->GetCTiglElementOfFuselage(endElementUID);

    // compute the new parameters for the new element
    CTiglPoint center = ( startElement->GetCenter() + endElement->GetCenter() ) * 0.5;
    CTiglPoint normal = ( startElement->GetNormal() + endElement->GetNormal() );
    if ( isNear( normal.norm2(), 0) ){
        normal = startElement->GetNormal();
    }
    normal.normalize();
    double angleN = ( startElement->GetRotationAroundNormal() + endElement->GetRotationAroundNormal() ) * 0.5;
    double area = (startElement->GetArea() + endElement->GetArea() ) * 0.5;

    // create new section and element
    CTiglUIDManager& uidManager = GetUIDManager();
    std::string baseUID = uidManager.MakeUIDUnique(startElement->GetSectionUID() + "Bis" );
    CCPACSFuselageSection& newSection = GetSections().CreateSection(baseUID, startElement->GetProfileUID());
    CTiglFuselageSectionElement* newElement = newSection.GetSectionElement(1).GetCTiglSectionElement();

    // set the new parameters
    newElement->SetCenter(center);
    newElement->SetArea(area);
    newElement->SetNormal(normal);
    newElement->SetRotationAroundNormal(angleN);


    // connect the element with segment and update old segment
    GetSegments().SplitSegment(segmentToSplit, newElement->GetSectionElementUID() );



}

void CCPACSFuselage::CreateNewConnectedElementAfter(std::string startElementUID)
{

    std::vector<std::string>  elementsAfter = ListFunctions::GetElementsAfter(fuselageHelper->GetElementUIDsInOrder(), startElementUID);
    if ( elementsAfter.size() > 0 ) {
        // In this case we insert the elemenet between the start element and the next one
        this->CreateNewConnectedElementBetween(startElementUID, elementsAfter[0] );
    }
    else {
        // in this case we simply need to find the previous element and call the appropriate function
        std::vector<std::string>  elementsBefore = ListFunctions::GetElementsInBetween(fuselageHelper->GetElementUIDsInOrder(), fuselageHelper->GetNoseUID(),startElementUID);
        if ( elementsBefore.size() < 2) {
            throw  CTiglError("Impossible to add a element after if there is no previous element");
        }
        std::string  previousElementUID = elementsBefore[elementsBefore.size()-2];

        CTiglFuselageSectionElement* previousElement = fuselageHelper->GetCTiglElementOfFuselage(previousElementUID);
        CTiglFuselageSectionElement* startElement = fuselageHelper->GetCTiglElementOfFuselage(startElementUID);

        // Compute the parameters for the new section base on the start element and the previous element.
        // We try to create a continuous fuselage
        CTiglPoint normal  =  startElement->GetNormal() + (startElement->GetNormal() - previousElement->GetNormal() ) ;
        CTiglPoint center = startElement->GetCenter() + (startElement->GetCenter() - previousElement->GetCenter() );
        double angleN = startElement->GetRotationAroundNormal() + (startElement->GetRotationAroundNormal() -previousElement->GetRotationAroundNormal());
        double area = startElement->GetArea();
        if (previousElement->GetArea() > 0) {
            double scaleF = startElement->GetArea() / previousElement->GetArea();
            area = scaleF * area;
        }
        std::string profileUID = startElement->GetProfileUID();
        std::string sectionUID = startElement->GetSectionUID() + "After";


        CCPACSFuselageSection& newSection = GetSections().CreateSection(sectionUID, profileUID);
        CTiglFuselageSectionElement* newElement = newSection.GetSectionElement(1).GetCTiglSectionElement();

        newElement->SetNormal(normal);
        newElement->SetRotationAroundNormal(angleN);
        newElement->SetCenter(center);
        newElement->SetArea(area);

        // Connect the element with the segment
        CCPACSFuselageSegment&  newSegment = GetSegments().AddSegment();
        std::string newSegmentUID = GetUIDManager().MakeUIDUnique("SegGenerated");

        newSegment.SetUID(newSegmentUID);
        newSegment.SetName(newSegmentUID);
        newSegment.SetFromElementUID(startElementUID);
        newSegment.SetToElementUID(newElement->GetSectionElementUID());

    }

}

void CCPACSFuselage::CreateNewConnectedElementBefore(std::string startElementUID)
{
    std::vector<std::string> elementsBefore = ListFunctions::GetElementsInBetween(fuselageHelper->GetElementUIDsInOrder(), fuselageHelper->GetNoseUID(),startElementUID);
    if ( elementsBefore.size() > 1 ) {
        this->CreateNewConnectedElementBetween(elementsBefore[elementsBefore.size()-2], startElementUID);
    }
    else {
        std::vector<std::string> elementsAfter  =  ListFunctions::GetElementsAfter(fuselageHelper->GetElementUIDsInOrder(), startElementUID);
        if (elementsAfter.size() < 1 ) {
            throw  CTiglError("Impossible to add a element before if there is no previous element");
        }
        std::string  previousElementUID = elementsAfter[0];

        CTiglFuselageSectionElement* previousElement = fuselageHelper->GetCTiglElementOfFuselage(previousElementUID);
        CTiglFuselageSectionElement* startElement = fuselageHelper->GetCTiglElementOfFuselage(startElementUID);

        // Compute the parameters for the new section base on the start element and the previous element.
        // We try to create a continuous fuselage
        CTiglPoint normal  =  startElement->GetNormal() + (startElement->GetNormal() - previousElement->GetNormal() ) ;
        CTiglPoint center = startElement->GetCenter() + (startElement->GetCenter() - previousElement->GetCenter() );
        double angleN = startElement->GetRotationAroundNormal() + (startElement->GetRotationAroundNormal() -previousElement->GetRotationAroundNormal());
        double area = startElement->GetArea();
        if (previousElement->GetArea() > 0) {
            double scaleF = startElement->GetArea() / previousElement->GetArea();
            area = scaleF * area;
        }
        std::string profileUID = startElement->GetProfileUID();
        std::string sectionUID = startElement->GetSectionUID() + "Before";


        CCPACSFuselageSection& newSection = GetSections().CreateSection(sectionUID, profileUID);
        CTiglFuselageSectionElement* newElement = newSection.GetSectionElement(1).GetCTiglSectionElement();

        newElement->SetNormal(normal);
        newElement->SetRotationAroundNormal(angleN);
        newElement->SetCenter(center);
        newElement->SetArea(area);

        // Connect the element with the segment
        CCPACSFuselageSegment&  newSegment = GetSegments().AddSegment();
        std::string newSegmentUID = GetUIDManager().MakeUIDUnique("SegGenerated");

        newSegment.SetUID(newSegmentUID);
        newSegment.SetName(newSegmentUID);
        newSegment.SetFromElementUID(newElement->GetSectionElementUID());
        newSegment.SetToElementUID(startElementUID);

        GetSegments().Invalidate(); // to reorder the segment if needed.

    }
}

std::vector<std::string> CCPACSFuselage::GetOrderedConnectedElement()
{
    return fuselageHelper->GetElementUIDsInOrder();
}



} // end namespace tigl
