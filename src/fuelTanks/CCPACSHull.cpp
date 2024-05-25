/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-15 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Implementation of CPACS duct handling routines.
*/

#include "CCPACSHull.h"
#include "CCPACSHulls.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglMakeLoft.h"
#include "CNamedShape.h"
#include "CTiglTopoAlgorithms.h"
#include "tiglcommonfunctions.h"
#include "CCPACSGenericFuelTank.h"
#include "CCPACSGenericFuelTanks.h"
#include "generated/CPACSDomeType.h"
#include "generated/CPACSEllipsoidDome.h"
#include "generated/CPACSTorisphericalDome.h"

#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopExp.hxx"

#include <gp_Elips.hxx>
#include <gp_Circ.hxx>
#include <GC_MakeArcOfEllipse.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <GeomAPI_PointsToBSpline.hxx>

namespace tigl
{

CCPACSHull::CCPACSHull(CCPACSHulls* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSHull(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(GetParent()->GetParent(), &m_transformation, true, true)
{
}

CCPACSConfiguration& CCPACSHull::GetConfiguration() const
{
    return GetParent()->GetParent()->GetConfiguration();
}

std::string CCPACSHull::GetDefaultedUID() const
{
    return generated::CPACSHull::GetUID();
}

TiglGeometricComponentType CCPACSHull::GetComponentType() const
{
    return TIGL_COMPONENT_FUSELAGE_TANK_HULL;
}

TiglGeometricComponentIntent CCPACSHull::GetComponentIntent() const
{
    // needs to be physical, so that transformation relative to parent works
    return TIGL_INTENT_PHYSICAL;
}

int CCPACSHull::GetSectionCount() const
{
    // ToDo: add exception handling
    // ToDo: .get() or .get_ptr()?
    return m_sections_choice1.get().GetSectionCount();
}

CCPACSFuselageSection& CCPACSHull::GetSection(int index) const
{
    // ToDo: add exception handling
    return m_sections_choice1.get().GetSection(index);
}

TopoDS_Shape CCPACSHull::GetSectionFace(const std::string section_uid) const
{
    // search for the section in all segments
    for (int n = 0; n < GetSegmentCount(); ++n) {
        const CCPACSFuselageSegment& segment = GetSegment(n + 1);

        if (section_uid == segment.GetStartSectionUID()) {
            return BuildFace(segment.GetStartWire());
        }
        else if (section_uid == segment.GetEndSectionUID()) {
            return BuildFace(segment.GetEndWire());
        }
    }
    throw CTiglError("GetSectionFace: Could not find a section for the given UID");
    return TopoDS_Shape();
}

int CCPACSHull::GetSegmentCount() const
{
    // ToDo: add exception handling
    return m_segments_choice1.get().GetSegmentCount();
}

CCPACSFuselageSegment& CCPACSHull::GetSegment(const int index)
{
    // ToDo: add exception handling
    return m_segments_choice1.get().GetSegment(index);
}

const CCPACSFuselageSegment& CCPACSHull::GetSegment(const int index) const
{
    // ToDo: add exception handling
    return m_segments_choice1.get().GetSegment(index);
}

CCPACSFuselageSegment& CCPACSHull::GetSegment(std::string uid)
{
    // ToDo: add exception handling
    return m_segments_choice1.get().GetSegment(uid);
}

double CCPACSHull::GetVolume()
{
    const TopoDS_Shape fusedSegments = GetLoft()->Shape();

    GProp_GProps hull;
    BRepGProp::VolumeProperties(fusedSegments, hull);
    double myVolume = hull.Mass();
    return myVolume;
}

double CCPACSHull::GetSurfaceArea()
{
    const PNamedShape& fusedSegments = GetLoft();

    // loop over all faces that are not symmetry, front or rear
    double myArea = 0.;

    TopTools_IndexedMapOfShape shapeMap;
    TopExp::MapShapes(fusedSegments->Shape(), TopAbs_FACE, shapeMap);
    for (int i = 1; i <= shapeMap.Extent(); ++i) {
        if (GetUID() == fusedSegments->GetFaceTraits(i - 1).Name()) {
            const TopoDS_Face& f = TopoDS::Face(shapeMap(i));
            GProp_GProps System;
            BRepGProp::SurfaceProperties(f, System);
            myArea += System.Mass();
        }
    }

    // Calculate surface area
    return myArea;
}

double CCPACSHull::GetCircumference(const int segmentIndex, const double eta)
{
    return static_cast<CCPACSFuselageSegment&>(GetSegment(segmentIndex)).GetCircumference(eta);
}

void CCPACSHull::IsotensoidContour(double rCyl, double rPolarOpening, int nodeNumber, std::vector<double>& x,
                                   std::vector<double>& r) const
{
    double dPhi = 1.0 / nodeNumber;

    r.push_back(rCyl);
    x.push_back(0.0);

    double phi   = 0.0;
    double alpha = std::asin(rPolarOpening / rCyl);

    while (std::tan(alpha) * std::tan(alpha) < 2 && r.back() > 1.22 * rPolarOpening) {
        phi += dPhi;
        alpha     = std::asin(rPolarOpening / r.back());
        double rm = r.back() / (std::cos(phi) * (2 - std::tan(alpha) * std::tan(alpha)));
        double dr = rm * dPhi * std::sin(phi);
        r.push_back(r.back() - dr);
        double dx = rm * dPhi * std::cos(phi);
        x.push_back(x.back() + dx);
    }

    double slope = (r.back() - r[r.size() - 2]) / (x.back() - x[x.size() - 2]);
    for (int i = 0; i < 5; ++i) {
        x.push_back(x.back() + (rPolarOpening - r.back()) / slope);
        r.push_back(r.back() + slope * (x.back() - x[x.size() - 2]));
    }

    double dx = x.back() - x.front();
    for (auto& val : x) {
        val -= dx;
    }
}

void CCPACSHull::BuildShapeFromSegments(TopoDS_Shape& loftShape) const
{
    const auto& segments    = m_segments_choice1.get();
    TiglContinuity cont     = segments.GetSegment(1).GetContinuity();
    Standard_Boolean smooth = (cont == ::C0 ? false : true);

    CTiglMakeLoft lofter;
    // add profiles
    for (int i = 1; i <= segments.GetSegmentCount(); i++) {
        lofter.addProfiles(segments.GetSegment(i).GetStartWire());
    }
    lofter.addProfiles(segments.GetSegment(segments.GetSegmentCount()).GetEndWire());

    // add guides
    lofter.addGuides(segments.GetGuideCurveWires());

    lofter.setMakeSolid(true);
    lofter.setMakeSmooth(smooth);

    loftShape = lofter.Shape();
}

void CCPACSHull::BuildTankWire(std::vector<TopoDS_Edge>& edges, BRepBuilderAPI_MakeWire& wire) const
{
    for (const auto& edge : edges) {
        wire.Add(edge);
    }

    TopoDS_Vertex v2 = GetLastVertex(edges.back());
    gp_Pnt mirror_point(BRep_Tool::Pnt(v2).X(), 0, 0);
    gp_Trsf mirror;
    mirror.SetMirror(gp_Ax2(mirror_point, gp_Dir(1, 0, 0)));

    for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
        TopoDS_Shape mirrored_edge = BRepBuilderAPI_Transform(*it, mirror).Shape();
        wire.Add(TopoDS::Edge(mirrored_edge));
    }
}

void CCPACSHull::BuildTankWireEllipsoid(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double R = cylinderRadius, h = cylinderRadius;

    double axRatio = m_domeType_choice2->GetEllipsoid_choice1().get_ptr()->GetHalfAxisFraction();
    if (axRatio < 0.0 || axRatio > 1.0) {
        throw CTiglError("Half axis fraction (" + std::to_string(axRatio) + ") of hull \"" + GetName() + "\" (uID=\"" +
                         GetUID() + "\") must be between 0 and 1!");
    }

    h = R * axRatio;

    gp_Dir dir(0.0, 1.0, 0.0);

    gp_Pnt p(h, 0.0, 0.0);

    gp_Elips ellips(gp_Ax2(p, dir), R, R * axRatio);

    GC_MakeArcOfEllipse arc(ellips, -M_PI / 2, 0.0, Standard_True);

    TopoDS_Edge dome_edge = BRepBuilderAPI_MakeEdge(arc.Value()).Edge();

    TopoDS_Vertex v1 = GetLastVertex(dome_edge);
    TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

    TopoDS_Edge cylinder_edge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();

    std::vector<TopoDS_Edge> edges = {dome_edge, cylinder_edge};
    BuildTankWire(edges, wire);
}

void CCPACSHull::BuildTankWireTorispherical(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    const auto* torispherical = m_domeType_choice2->GetTorispherical_choice2().get_ptr();

    double R = torispherical->GetDishRadius(); // Radius of the sphere (crown radius)
    double r = torispherical->GetKnuckleRadius(); // Radius of the torus (knuckle radius)

    if (R <= cylinderRadius) {
        throw CTiglError("The dish radius (" + std::to_string(R) + ") of hull \"" + GetName() + "\" (uID=\"" +
                         GetUID() + "\") must be larger than the cylinder radius (" + std::to_string(cylinderRadius) +
                         ")!");
    }
    if (r <= 0. || r >= cylinderRadius) {
        throw CTiglError("The knuckle radius (" + std::to_string(r) + ") of hull \"" + GetName() + "\" (uID=\"" +
                         GetUID() + "\") must be larger than 0 and smaller than the cylinder Radius (" +
                         std::to_string(cylinderRadius) + ")!");
    }

    double c     = cylinderRadius - r; // Distance from the center to the center of the torus tube
    double h     = R - sqrt((r + c - R) * (r - c - R)); // Height from the base of the dome to the top
    double rt    = c * (1.0 + pow((R / r - 1), -1.0)); // Radius where the transition from sphere to torus occurs
    double alpha = acos((rt - c) / r); // Angle under which the transition from sphere to torus occurs

    gp_Dir dir(0.0, 1.0, 0.0);

    gp_Pnt p1(R, 0.0, 0.0);
    gp_Pnt p2(h, 0.0, cylinderRadius - r);

    gp_Circ circ1(gp_Ax2(p1, dir), R);
    gp_Circ circ2(gp_Ax2(p2, dir), r);

    GC_MakeArcOfCircle arc1(circ1, -M_PI / 2, -alpha, Standard_True);
    GC_MakeArcOfCircle arc2(circ2, -alpha, 0, Standard_True);

    TopoDS_Edge dome_edge1 = BRepBuilderAPI_MakeEdge(arc1.Value()).Edge();
    TopoDS_Edge dome_edge2 = BRepBuilderAPI_MakeEdge(arc2.Value()).Edge();

    TopoDS_Vertex v1 = GetLastVertex(dome_edge2);
    TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

    TopoDS_Edge cylinder_edge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();

    std::vector<TopoDS_Edge> edges = {dome_edge1, dome_edge2, cylinder_edge};
    BuildTankWire(edges, wire);
}

void CCPACSHull::BuildTankWireIsotensoid(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double polarOpeningRadius = m_domeType_choice2->GetIsotensoid_choice3().get_ptr()->GetPolarOpeningRadius();
    if (polarOpeningRadius <= 0 || polarOpeningRadius >= cylinderRadius) {
        throw CTiglError("The polar opening radius (" + std::to_string(polarOpeningRadius) + ") of hull \"" +
                         GetName() + "\" (uID=\"" + GetUID() +
                         "\") must be larger than 0 and smaller than the cylinder radius (" +
                         std::to_string(cylinderRadius) + ")!");
    }

    std::vector<double> x, r;
    IsotensoidContour(cylinderRadius, polarOpeningRadius, 50, x, r);
    double h = x.back() - x.front();

    TColgp_Array1OfPnt array(1, x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        array.SetValue(i + 1, gp_Pnt(-x[i], 0.0, r[i]));
    }
    Handle(Geom_BSplineCurve) bspline = GeomAPI_PointsToBSpline(array).Curve();
    TopoDS_Edge dome_edge             = BRepBuilderAPI_MakeEdge(bspline);

    TopoDS_Vertex v1 = GetFirstVertex(dome_edge);
    TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

    TopoDS_Edge cylinder_edge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();

    std::vector<TopoDS_Edge> edges = {dome_edge, cylinder_edge};
    BuildTankWire(edges, wire);
}

void CCPACSHull::BuildShapeFromSimpleParameters(TopoDS_Shape& loftShape) const
{
    BRepBuilderAPI_MakeWire wire;

    if (m_cylinderRadius_choice2.get() <= 0) {
        throw CTiglError("The cylinder radius (" + std::to_string(m_cylinderRadius_choice2.get()) + ") of hull \"" +
                         GetName() + "\" (uID=\"" + GetUID() + "\") must be larger than 0!");
    }
    if (m_cylinderLength_choice2.get() <= 0) {
        throw CTiglError("The cylinder length (" + std::to_string(m_cylinderLength_choice2.get()) + ") of hull \"" +
                         GetName() + "\" (uID=\"" + GetUID() + "\") must be larger than 0!");
    }

    if (m_domeType_choice2->GetEllipsoid_choice1()) {
        BuildTankWireEllipsoid(wire);
    }
    else if (m_domeType_choice2->GetTorispherical_choice2()) {
        BuildTankWireTorispherical(wire);
    }
    else if (m_domeType_choice2->GetIsotensoid_choice3()) {
        BuildTankWireIsotensoid(wire);
    }

    gp_Ax1 ax(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
    loftShape = BRepPrimAPI_MakeRevol(wire, ax).Shape();

    tigl::CTiglTransformation transform = this->GetTransformationMatrix();
    TopoDS_Shape TransformedShape       = transform.Transform(loftShape);

    loftShape = TransformedShape;
}

PNamedShape CCPACSHull::BuildLoft() const
{

    TopoDS_Shape loftShape;
    std::string loftName      = GetUID();
    std::string loftShortName = GetShortShapeName();

    if (m_segments_choice1 && m_sections_choice1) {
        BuildShapeFromSegments(loftShape);
        PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
        SetFaceTraits(loft);
        return loft;
    }
    else if (m_cylinderRadius_choice2 && m_cylinderLength_choice2 && m_domeType_choice2) {

        const tigl::generated::CPACSEllipsoidDome* ellipsoid = m_domeType_choice2->GetEllipsoid_choice1().get_ptr();
        const tigl::generated::CPACSTorisphericalDome* torispherical =
            m_domeType_choice2->GetTorispherical_choice2().get_ptr();
        const tigl::generated::CPACSIsotensoidDome* isotensoid = m_domeType_choice2->GetIsotensoid_choice3().get_ptr();

        BuildShapeFromSimpleParameters(loftShape);
        PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
        return loft;
    }
    else {
        throw CTiglError("No valid combination of segments and sections or parametric specification for lofting of "
                         "tank hull available.",
                         TIGL_ERROR);
    }
}

gp_Pnt CCPACSHull::GetPoint(int segmentIndex, double eta, double zeta)
{
    return ((CCPACSFuselageSegment&)GetSegment(segmentIndex)).GetPoint(eta, zeta, getPointBehavior);
}

// Sets the GetPoint behavior to asParameterOnSurface or onLinearLoft
void CCPACSHull::SetGetPointBehavior(TiglGetPointBehavior behavior)
{
    getPointBehavior = behavior;
}

TiglGetPointBehavior CCPACSHull::GetGetPointBehavior() const
{
    return getPointBehavior;
}

CCPACSGuideCurve& CCPACSHull::GetGuideCurveSegment(std::string uid)
{
    return const_cast<CCPACSGuideCurve&>(static_cast<const CCPACSHull&>(*this).GetGuideCurveSegment(uid));
}

const CCPACSGuideCurve& CCPACSHull::GetGuideCurveSegment(std::string uid) const
{
    for (int i = 1; i <= m_segments_choice1.get().GetSegmentCount(); i++) {
        const CCPACSFuselageSegment& segment = m_segments_choice1.get().GetSegment(i);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        if (segment.GetGuideCurves()->GuideCurveExists(uid)) {
            return segment.GetGuideCurves()->GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

std::vector<gp_Pnt> CCPACSHull::GetGuideCurvePoints() const
{
    std::vector<gp_Pnt> points;

    // connect the belonging guide curve segments
    for (int isegment = 1; isegment <= GetSegmentCount(); ++isegment) {
        const CCPACSFuselageSegment& segment = m_segments_choice1.get().GetSegment(isegment);

        if (!segment.GetGuideCurves()) {
            continue;
        }

        const CCPACSGuideCurves& segmentCurves = *segment.GetGuideCurves();
        for (int iguide = 1; iguide <= segmentCurves.GetGuideCurveCount(); ++iguide) {
            const CCPACSGuideCurve& curve = segmentCurves.GetGuideCurve(iguide);
            std::vector<gp_Pnt> curPoints = curve.GetCurvePoints();
            points.insert(points.end(), curPoints.begin(), curPoints.end());
        }
    }
    return points;
}

// get short name for loft
std::string CCPACSHull::GetShortShapeName() const
{
    unsigned int findex = 0;
    unsigned int i      = 0;

    for (auto& h : GetParent()->GetHulls()) {
        ++i;
        if (GetUID() == h->GetUID()) {
            findex = i;
            std::stringstream shortName;
            shortName << "H" << findex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

void CCPACSHull::SetFaceTraits(PNamedShape loft) const
{
    int nFacesTotal       = GetNumberOfFaces(loft->Shape());
    int nFacesAero        = nFacesTotal;
    bool hasSymmetryPlane = GetNumberOfEdges(m_segments_choice1.get().GetSegment(1).GetEndWire()) > 1;

    std::vector<std::string> names;
    names.push_back(loft->Name());
    names.push_back("symmetry");
    names.push_back("Front");
    names.push_back("Rear");

    if (!CTiglTopoAlgorithms::IsDegenerated(m_segments_choice1.get().GetSegment(1).GetStartWire())) {
        nFacesAero -= 1;
    }
    if (!CTiglTopoAlgorithms::IsDegenerated(
            m_segments_choice1.get().GetSegment(m_segments_choice1.get().GetSegmentCount()).GetEndWire())) {
        nFacesAero -= 1;
    }

    // if we have a smooth surface, the whole fuslage is treatet as one segment
    int nSegments = m_segments_choice1.get().GetSegmentCount();

    int facesPerSegment = nFacesAero / nSegments;

    int iFaceTotal     = 0;
    int nSymmetryFaces = (int)hasSymmetryPlane;
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
    for (; iFaceTotal < nFacesTotal; ++iFaceTotal) {
        loft->FaceTraits(iFaceTotal).SetName(names[iFace++].c_str());
    }
}

} //namespace tigl
