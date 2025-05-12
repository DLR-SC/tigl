/*
* Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
*
* Created: 2025-05-09 Marko Alder <Marko.Alder@dlr.de>
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
* @brief  Implementation of CPACS duct handling routines.
*/

#include "CCPACSVessel.h"
#include "CCPACSVessels.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglMakeLoft.h"
#include "CNamedShape.h"
#include "CTiglTopoAlgorithms.h"
#include "tiglcommonfunctions.h"
#include "CCPACSFuelTank.h"
#include "CCPACSFuelTanks.h"
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
#include <limits>

namespace tigl
{

CCPACSVessel::CCPACSVessel(CCPACSVessels* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSVessel(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(GetParent()->GetParent(), &m_transformation)
{
    m_transformation.setScalingType(ABS_LOCAL);
    m_transformation.setRotationType(ABS_LOCAL);
}

CCPACSConfiguration const& CCPACSVessel::GetConfiguration() const
{
    return GetParent()->GetParent()->GetConfiguration();
}

std::string CCPACSVessel::GetDefaultedUID() const
{
    return generated::CPACSVessel::GetUID();
}

TiglGeometricComponentType CCPACSVessel::GetComponentType() const
{
    return TIGL_COMPONENT_TANK_HULL;
}

TiglGeometricComponentIntent CCPACSVessel::GetComponentIntent() const
{
    // needs to be physical, so that transformation relative to parent works
    return TIGL_INTENT_PHYSICAL;
}

int CCPACSVessel::GetSectionCount() const
{
    if (m_sections_choice1) {
        return m_sections_choice1.get_ptr()->GetSectionCount();
    }
    else {
        return 0;
    }
}

CCPACSFuselageSection const& CCPACSVessel::GetSection(int index) const
{
    return const_cast<CCPACSVessel&>(*this).GetSection(index);
}

CCPACSFuselageSection& CCPACSVessel::GetSection(int index)
{
    if (m_sections_choice1) {
        return m_sections_choice1.get().GetSection(index);
    }
    else {
        throw CTiglError(_vesselTypeException);
    }
}

CCPACSFuselageSection const& CCPACSVessel::GetSection(const std::string& sectionUID) const
{
    return const_cast<CCPACSVessel&>(*this).GetSection(sectionUID);
}

CCPACSFuselageSection& CCPACSVessel::GetSection(const std::string& sectionUID)
{
    if (m_sections_choice1) {
        return m_sections_choice1.get().GetSection(sectionUID);
    }
    else {
        throw CTiglError(_vesselTypeException);
    }
}

TopoDS_Shape CCPACSVessel::GetSectionFace(const std::string sectionUID) const
{
    const int segmentCount = GetSegmentCount();

    // Search for the section in all segments
    for (int n = 0; n < segmentCount; ++n) {
        const CCPACSFuselageSegment& segment = GetSegment(n + 1);

        if (sectionUID == segment.GetStartSectionUID()) {
            return BuildFace(segment.GetStartWire());
        }
        else if (sectionUID == segment.GetEndSectionUID()) {
            return BuildFace(segment.GetEndWire());
        }
    }
    m_sections_choice1 ? throw CTiglError("GetSectionFace: Could not find a fuselage section for the given UID")
                       : throw CTiglError(_vesselTypeException);
    return TopoDS_Shape();
}

int CCPACSVessel::GetSegmentCount() const
{
    if (m_segments_choice1) {
        return m_segments_choice1.get_ptr()->GetSegmentCount();
    }
    else {
        return 0;
    }
}

CCPACSFuselageSegment& CCPACSVessel::GetSegment(const int index)
{
    if (m_segments_choice1) {
        return m_segments_choice1.get().GetSegment(index);
    }
    else {
        throw CTiglError(_vesselTypeException);
    }
}

const CCPACSFuselageSegment& CCPACSVessel::GetSegment(const int index) const
{
    if (m_segments_choice1) {
        return m_segments_choice1.get().GetSegment(index);
    }
    else {
        throw CTiglError(_vesselTypeException);
    }
}

CCPACSFuselageSegment& CCPACSVessel::GetSegment(std::string uid)
{
    if (m_segments_choice1) {
        return m_segments_choice1.get().GetSegment(uid);
    }
    else {
        throw CTiglError(_vesselTypeException);
    }
}

double CCPACSVessel::GetGeometricVolume()
{
    const TopoDS_Shape& shape = GetLoft()->Shape();
    GProp_GProps shapeProps;
    BRepGProp::VolumeProperties(shape, shapeProps);

    return shapeProps.Mass();
}

double CCPACSVessel::GetSurfaceArea()
{
    const PNamedShape& shape = GetLoft();
    double area              = 0.;

    TopTools_IndexedMapOfShape shapeMap;
    TopExp::MapShapes(shape->Shape(), TopAbs_FACE, shapeMap);
    for (int i = 1; i <= shapeMap.Extent(); ++i) {
        const std::string& faceName = shape->GetFaceTraits(i - 1).Name();
        if (faceName != "symmetry") {
            const TopoDS_Face& face = TopoDS::Face(shapeMap(i));
            GProp_GProps shapeProps;
            BRepGProp::SurfaceProperties(face, shapeProps);
            area += shapeProps.Mass();
        }
    }

    return area;
}

double CCPACSVessel::GetCircumference(const int segmentIndex, const double eta)
{
    if (m_segments_choice1) {
        return static_cast<CCPACSFuselageSegment&>(GetSegment(segmentIndex)).GetCircumference(eta);
    }
    else {
        throw CTiglError(_vesselTypeException);
    }
}

bool CCPACSVessel::IsVesselViaSegments() const
{
    if (m_segments_choice1) {
        return true;
    }
    else if (m_domeType_choice2) {
        return false;
    }
    else {
        throw CTiglError("Vessel type could not be determined.");
    }
}

bool CCPACSVessel::IsVesselViaDesignParameters() const
{
    if (m_segments_choice1) {
        return false;
    }
    else if (m_domeType_choice2) {
        return true;
    }
    else {
        throw CTiglError("Vessel type could not be determined.");
    }
}

bool CCPACSVessel::HasSphericalDome() const
{
    return GetEllipsoidDome() && GetEllipsoidDome()->GetHalfAxisFraction() == 1.0;
}

bool CCPACSVessel::HasEllipsoidDome() const
{
    return GetEllipsoidDome() != nullptr;
}

bool CCPACSVessel::HasIsotensoidDome() const
{
    return GetIsotensoidDome() != nullptr;
}

bool CCPACSVessel::HasTorisphericalDome() const
{
    return GetTorisphericalDome() != nullptr;
}

CCPACSEllipsoidDome const* CCPACSVessel::GetEllipsoidDome() const
{
    if (m_domeType_choice2) {
        return m_domeType_choice2->GetEllipsoid_choice1().get_ptr();
    }
    return nullptr;
}

CCPACSTorisphericalDome const* CCPACSVessel::GetTorisphericalDome() const
{
    if (m_domeType_choice2) {
        return m_domeType_choice2->GetTorispherical_choice2().get_ptr();
    }
    return nullptr;
}

CCPACSIsotensoidDome const* CCPACSVessel::GetIsotensoidDome() const
{
    if (m_domeType_choice2) {
        return m_domeType_choice2->GetIsotensoid_choice3().get_ptr();
    }
    return nullptr;
}

CCPACSVessel::IsotensoidContour::IsotensoidContour(double rCyl, double rPolarOpening, int nodeNumber)
{
    double dPhi = 1.0 / nodeNumber;

    radii.push_back(rCyl);
    axialPositions.push_back(0.0);

    double phi   = 0.0;
    double alpha = std::asin(rPolarOpening / rCyl);

    const double eps = std::numeric_limits<double>::epsilon();

    while (std::tan(alpha) * std::tan(alpha) < 2 && radii.back() > 1.22 * rPolarOpening &&
           (radii.size() < 2 || radii.back() < radii[radii.size() - 2] + eps)) {
        phi += dPhi;
        alpha = std::asin(rPolarOpening / radii.back());

        double rm = radii.back() / (std::cos(phi) * (2 - std::tan(alpha) * std::tan(alpha)));
        double dr = rm * dPhi * std::sin(phi);
        radii.push_back(radii.back() - dr);

        double dx = rm * dPhi * std::cos(phi);
        axialPositions.push_back(axialPositions.back() + dx);
    }

    // Add 5 points linear until polar opening
    double slope =
        (radii.back() - radii[radii.size() - 2]) / (axialPositions.back() - axialPositions[axialPositions.size() - 2]);
    for (int i = 0; i < 5; ++i) {
        axialPositions.push_back(axialPositions.back() + (rPolarOpening - radii.back()) / slope);
        radii.push_back(radii.back() + slope * (axialPositions.back() - axialPositions[axialPositions.size() - 2]));
    }

    // Shifting to x=0
    double totalDx = axialPositions.back() - axialPositions.front();
    for (double& v : axialPositions) {
        v -= totalDx;
    }
}

Handle(Geom_BSplineCurve) CCPACSVessel::IsotensoidContour::ToBSpline() const
{
    TColgp_Array1OfPnt pts(1, (int)axialPositions.size());
    for (size_t i = 0; i < axialPositions.size(); ++i) {
        pts.SetValue((int)i + 1, gp_Pnt(-axialPositions[i], 0.0, radii[i]));
    }
    return GeomAPI_PointsToBSpline(pts).Curve();
}

TopoDS_Edge CCPACSVessel::IsotensoidContour::ToEdge() const
{
    return BRepBuilderAPI_MakeEdge(ToBSpline());
}

void CCPACSVessel::BuildShapeFromSegments(TopoDS_Shape& loftShape) const
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

void CCPACSVessel::BuildVesselWire(std::vector<TopoDS_Edge>& edges, BRepBuilderAPI_MakeWire& wire) const
{
    for (const auto& edge : edges) {
        wire.Add(edge);
    }

    TopoDS_Vertex v2 = TopExp::LastVertex(edges.back());
    gp_Pnt mirror_point(BRep_Tool::Pnt(v2).X(), 0, 0);
    gp_Trsf mirror;
    mirror.SetMirror(gp_Ax2(mirror_point, gp_Dir(1, 0, 0)));

    for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
        TopoDS_Shape mirrored_edge = BRepBuilderAPI_Transform(*it, mirror).Shape();
        wire.Add(TopoDS::Edge(mirrored_edge));
    }
}

void CCPACSVessel::BuildVesselWireEllipsoid(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double axRatio = GetEllipsoidDome()->GetHalfAxisFraction();
    if (axRatio < 0.0) {
        throw CTiglError("Half axis fraction (" + std::to_string(axRatio) + ") of vessel \"" + GetName() +
                         "\" (uID=\"" + GetUID() + "\") must be a positive value!");
    }

    double R = cylinderRadius;
    double h = R * axRatio;

    gp_Dir dir(0.0, 1.0, 0.0);
    gp_Pnt p(h, 0.0, 0.0);

    double majorAxis  = R;
    double minorAxis  = R * axRatio;
    double rotAngle   = 0;
    double startAngle = -M_PI / 2;
    double endAngle   = 0.0;

    if (axRatio > 1) {
        majorAxis  = R * axRatio;
        minorAxis  = R;
        rotAngle   = 0.5 * M_PI;
        startAngle = -M_PI;
        endAngle   = -M_PI / 2;
    }

    gp_Elips ellips(gp_Ax2(p, dir), majorAxis, minorAxis);
    ellips.Rotate(gp_Ax1(p, dir), rotAngle);
    GC_MakeArcOfEllipse arc(ellips, startAngle, endAngle, Standard_True);

    TopoDS_Edge domeEdge = BRepBuilderAPI_MakeEdge(arc.Value()).Edge();

    std::vector<TopoDS_Edge> edges = {domeEdge};

    if (cylinderLength > 0.0) {
        TopoDS_Vertex v1 = TopExp::LastVertex(domeEdge);
        TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

        TopoDS_Edge cylinderEdge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();
        edges.push_back(cylinderEdge);
    }

    BuildVesselWire(edges, wire);
}

void CCPACSVessel::BuildVesselWireTorispherical(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double R = GetTorisphericalDome()->GetDishRadius(); // Radius of the sphere (crown radius)
    double r = GetTorisphericalDome()->GetKnuckleRadius(); // Radius of the torus (knuckle radius)

    if (R <= cylinderRadius) {
        throw CTiglError("The dish radius (" + std::to_string(R) + ") of vessel \"" + GetName() + "\" (uID=\"" +
                         GetUID() + "\") must be larger than the cylinder radius (" + std::to_string(cylinderRadius) +
                         ")!");
    }
    if (r <= 0. || r >= cylinderRadius) {
        throw CTiglError("The knuckle radius (" + std::to_string(r) + ") of vessel \"" + GetName() + "\" (uID=\"" +
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

    TopoDS_Edge domeEdge1 = BRepBuilderAPI_MakeEdge(arc1.Value()).Edge();
    TopoDS_Edge domeEdge2 = BRepBuilderAPI_MakeEdge(arc2.Value()).Edge();

    std::vector<TopoDS_Edge> edges = {domeEdge1, domeEdge2};

    if (cylinderLength > 0.0) {
        TopoDS_Vertex v1 = TopExp::LastVertex(domeEdge2);
        TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

        TopoDS_Edge cylinderEdge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();
        edges.push_back(cylinderEdge);
    }

    BuildVesselWire(edges, wire);
}

void CCPACSVessel::BuildVesselWireIsotensoid(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double polarOpeningRadius = GetIsotensoidDome()->GetPolarOpeningRadius();
    if (polarOpeningRadius <= 0 || polarOpeningRadius >= cylinderRadius) {
        throw CTiglError("The polar opening radius (" + std::to_string(polarOpeningRadius) + ") of vessel \"" +
                         GetName() + "\" (uID=\"" + GetUID() +
                         "\") must be larger than 0 and smaller than the cylinder radius (" +
                         std::to_string(cylinderRadius) + ")!");
    }

    // 50 Points is chosen as a compromise between performance and accuracy
    IsotensoidContour contour(cylinderRadius, polarOpeningRadius, 50);
    TopoDS_Edge domeEdge = contour.ToEdge();

    TopoDS_Vertex v1    = BRepBuilderAPI_MakeVertex(gp_Pnt(-contour.axialPositions.back(), 0.0, 0.0));
    TopoDS_Vertex v2    = TopExp::LastVertex(domeEdge);
    TopoDS_Edge lidEdge = BRepBuilderAPI_MakeEdge(v1, v2);

    std::vector<TopoDS_Edge> edges = {lidEdge, domeEdge};

    if (cylinderLength > 0.0) {
        double h         = contour.axialPositions.back() - contour.axialPositions.front();
        TopoDS_Vertex v1 = TopExp::FirstVertex(domeEdge);
        TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

        TopoDS_Edge cylinderEdge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();
        edges.push_back(cylinderEdge);
    }

    BuildVesselWire(edges, wire);
}

void CCPACSVessel::BuildShapeFromSimpleParameters(TopoDS_Shape& loftShape) const
{

    if (!m_domeType_choice2 || !m_cylinderRadius_choice2 || !m_cylinderLength_choice2) {
        throw CTiglError("Parametric vessel specification incomplete: "
                         "cylinderRadius, cylinderLength and domeType required.",
                         TIGL_ERROR);
    }

    if (m_cylinderRadius_choice2.get() <= 0) {
        throw CTiglError("The cylinder radius (" + std::to_string(m_cylinderRadius_choice2.get()) + ") of vessel \"" +
                         GetName() + "\" (uID=\"" + GetUID() + "\") must be larger than 0!");
    }
    if (m_cylinderLength_choice2.get() < 0) {
        throw CTiglError("The cylinder length (" + std::to_string(m_cylinderLength_choice2.get()) + ") of vessel \"" +
                         GetName() + "\" (uID=\"" + GetUID() + "\") must be larger than or equal to 0!");
    }

    BRepBuilderAPI_MakeWire wire;

    if (GetEllipsoidDome()) {
        BuildVesselWireEllipsoid(wire);
    }
    else if (GetTorisphericalDome()) {
        BuildVesselWireTorispherical(wire);
    }
    else if (GetIsotensoidDome()) {
        BuildVesselWireIsotensoid(wire);
    }

    gp_Ax1 ax(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
    loftShape = BRepPrimAPI_MakeRevol(wire, ax).Shape();

    tigl::CTiglTransformation transform = this->GetTransformationMatrix();
    TopoDS_Shape TransformedShape       = transform.Transform(loftShape);

    loftShape = TransformedShape;
}

PNamedShape CCPACSVessel::BuildLoft() const
{
    TopoDS_Shape loftShape;
    std::string loftName      = GetUID();
    std::string loftShortName = GetShortShapeName();

    if (m_sections_choice1) {
        BuildShapeFromSegments(loftShape);
        PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
        SetFaceTraitsFromSegments(loft);
        return loft;
    }
    else if (m_domeType_choice2) {
        BuildShapeFromSimpleParameters(loftShape);
        PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
        SetFaceTraitsFromParams(loft);
        return loft;
    }
    else {
        throw CTiglError("No valid combination of segments and sections or parametric specification for lofting of "
                         "tank vessel available.",
                         TIGL_ERROR);
    }
}

CCPACSGuideCurve& CCPACSVessel::GetGuideCurveSegment(std::string uid)
{
    return const_cast<CCPACSGuideCurve&>(static_cast<const CCPACSVessel&>(*this).GetGuideCurveSegment(uid));
}

const CCPACSGuideCurve& CCPACSVessel::GetGuideCurveSegment(std::string uid) const
{
    if (m_segments_choice1) {
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
    else {
        throw CTiglError(_vesselTypeException);
    }
}

std::vector<gp_Pnt> CCPACSVessel::GetGuideCurvePoints() const
{
    if (m_segments_choice1) {
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
    else {
        throw CTiglError(_vesselTypeException);
    }
}

// get short name for loft
std::string CCPACSVessel::GetShortShapeName() const
{
    unsigned int findex = 0;
    unsigned int i      = 0;

    for (auto& v : GetParent()->GetVessels()) {
        ++i;
        if (GetUID() == v->GetUID()) {
            findex = i;
            std::stringstream shortName;
            shortName << "V" << findex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

void CCPACSVessel::SetFaceTraitsFromSegments(PNamedShape loft) const
{
    int nFacesTotal = GetNumberOfFaces(loft->Shape());
    int nFacesAero  = nFacesTotal;

    auto& segments = m_segments_choice1.get();
    int nSegments  = segments.GetSegmentCount();

    bool hasSymmetryPlane = GetNumberOfEdges(segments.GetSegment(1).GetEndWire()) > 1;

    std::array<std::string, 4> names = {loft->Name(), "symmetry", "Front", "Rear"};

    if (!CTiglTopoAlgorithms::IsDegenerated(segments.GetSegment(1).GetStartWire())) {
        nFacesAero--;
    }
    if (!CTiglTopoAlgorithms::IsDegenerated(segments.GetSegment(nSegments).GetEndWire())) {
        nFacesAero--;
    }

    int facesPerSegment = nFacesAero / nSegments;
    int iFaceTotal      = 0;
    int nSymmetryFaces  = hasSymmetryPlane ? 1 : 0;

    for (int iSegment = 0; iSegment < nSegments; ++iSegment) {
        for (int iFace = 0; iFace < facesPerSegment - nSymmetryFaces; ++iFace) {
            loft->FaceTraits(iFaceTotal++).SetName(names[0].c_str());
        }
        for (int iFace = 0; iFace < nSymmetryFaces; ++iFace) {
            loft->FaceTraits(iFaceTotal++).SetName(names[1].c_str());
        }
    }

    // Front and rear caps
    int iFace = 2;
    for (; iFaceTotal < nFacesTotal; ++iFaceTotal) {
        loft->FaceTraits(iFaceTotal).SetName(names[iFace++].c_str());
    }
}

void CCPACSVessel::SetFaceTraitsFromParams(PNamedShape loft) const
{
    int nFaces = GetNumberOfFaces(loft->Shape());

    // Cylinders are always the two middel segments
    const int cylCount = 2;
    const int cylStart = (nFaces - cylCount) / 2;
    const int cylEnd   = cylStart + cylCount;

    for (int i = 0; i < nFaces; ++i) {
        const char* tag = (i >= cylStart && i < cylEnd) ? "Cylinder" : "Dome";
        loft->FaceTraits(i).SetName(tag);
    }
}

void CCPACSVessel::InvalidateImpl(const boost::optional<std::string>&) const
{
    loft.clear();
    if (m_segments_choice1) {
        m_segments_choice1.get().Invalidate();
    }
    if (m_structure) {
        m_structure->Invalidate();
    }
    GetParent()->GetParent()->Invalidate();
}

} //namespace tigl
