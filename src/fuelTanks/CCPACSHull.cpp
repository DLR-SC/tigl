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
    if (m_sections_choice1) {
        return m_sections_choice1.get_ptr()->GetSectionCount();
    }
    else {
        return 0;
    }
}

CCPACSFuselageSection& CCPACSHull::GetSection(int index) const
{
    if (m_sections_choice1) {
        return m_sections_choice1.get().GetSection(index);
    }
    else {
        throw CTiglError(_hullTypeException);
    }
}

TopoDS_Shape CCPACSHull::GetSectionFace(const std::string sectionUID) const
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
                       : throw CTiglError(_hullTypeException);
    return TopoDS_Shape();
}

int CCPACSHull::GetSegmentCount() const
{
    if (m_segments_choice1) {
        return m_segments_choice1.get_ptr()->GetSegmentCount();
    }
    else {
        return 0;
    }
}

CCPACSFuselageSegment& CCPACSHull::GetSegment(const int index)
{
    if (m_segments_choice1) {
        return m_segments_choice1.get().GetSegment(index);
    }
    else {
        throw CTiglError(_hullTypeException);
    }
}

const CCPACSFuselageSegment& CCPACSHull::GetSegment(const int index) const
{
    if (m_segments_choice1) {
        return m_segments_choice1.get().GetSegment(index);
    }
    else {
        throw CTiglError(_hullTypeException);
    }
}

CCPACSFuselageSegment& CCPACSHull::GetSegment(std::string uid)
{
    if (m_segments_choice1) {
        return m_segments_choice1.get().GetSegment(uid);
    }
    else {
        throw CTiglError(_hullTypeException);
    }
}

double CCPACSHull::GetVolume()
{
    const TopoDS_Shape& shape = GetLoft()->Shape();
    GProp_GProps shapeProps;
    BRepGProp::VolumeProperties(shape, shapeProps);

    return shapeProps.Mass();
}

double CCPACSHull::GetSurfaceArea()
{
    const PNamedShape& shape = GetLoft();

    // Loop over all faces that are not symmetry, front or rear
    double area = 0.;

    TopTools_IndexedMapOfShape shapeMap;
    TopExp::MapShapes(shape->Shape(), TopAbs_FACE, shapeMap);
    for (int i = 1; i <= shapeMap.Extent(); ++i) {
        const std::string& faceName = shape->GetFaceTraits(i - 1).Name();
        if (GetUID() == faceName) {
            const TopoDS_Face& face = TopoDS::Face(shapeMap(i));
            GProp_GProps shapeProps;
            BRepGProp::SurfaceProperties(face, shapeProps);
            area += shapeProps.Mass();
        }
    }

    return area;
}

double CCPACSHull::GetCircumference(const int segmentIndex, const double eta)
{
    if (m_segments_choice1) {
        return static_cast<CCPACSFuselageSegment&>(GetSegment(segmentIndex)).GetCircumference(eta);
    }
    else {
        throw CTiglError(_hullTypeException);
    }
}

bool CCPACSHull::IsHullViaSegments() const
{
    if (m_segments_choice1) {
        return true;
    }
    else if (m_domeType_choice2) {
        return false;
    }
    else {
        throw CTiglError("Hull type could not be determined.");
    }
}

bool CCPACSHull::IsHullViaDesignParameters() const
{
    if (m_segments_choice1) {
        return false;
    }
    else if (m_domeType_choice2) {
        return true;
    }
    else {
        throw CTiglError("Hull type could not be determined.");
    }
}

bool CCPACSHull::HasSphericalDome() const
{
    EvaluateDome();
    if (_ellipsoidPtr && _ellipsoidPtr->GetHalfAxisFraction() == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool CCPACSHull::HasEllipsoidDome() const
{
    EvaluateDome();
    if (_ellipsoidPtr) {
        return true;
    }
    else {
        return false;
    }
}

bool CCPACSHull::HasIsotensoidDome() const
{
    EvaluateDome();
    if (_isotensoidPtr) {
        return true;
    }
    else {
        return false;
    }
}

bool CCPACSHull::HasTorisphericalDome() const
{
    EvaluateDome();
    if (_torisphericalPtr) {
        return true;
    }
    else {
        return false;
    }
}

void CCPACSHull::EvaluateDome() const
{
    if (!_isEvaluated) {
        if (m_domeType_choice2) {
            _ellipsoidPtr     = m_domeType_choice2->GetEllipsoid_choice1().get_ptr();
            _torisphericalPtr = m_domeType_choice2->GetTorispherical_choice2().get_ptr();
            _isotensoidPtr    = m_domeType_choice2->GetIsotensoid_choice3().get_ptr();
        }
        _isEvaluated = true;
    }
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

void CCPACSHull::BuildHullWire(std::vector<TopoDS_Edge>& edges, BRepBuilderAPI_MakeWire& wire) const
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

void CCPACSHull::BuildHullWireEllipsoid(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double axRatio = _ellipsoidPtr->GetHalfAxisFraction();
    if (axRatio < 0.0) {
        throw CTiglError("Half axis fraction (" + std::to_string(axRatio) + ") of hull \"" + GetName() + "\" (uID=\"" +
                         GetUID() + "\") must be a positive value!");
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
        TopoDS_Vertex v1 = GetLastVertex(domeEdge);
        TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

        TopoDS_Edge cylinderEdge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();
        edges.push_back(cylinderEdge);
    }

    BuildHullWire(edges, wire);
}

void CCPACSHull::BuildHullWireTorispherical(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double R = _torisphericalPtr->GetDishRadius(); // Radius of the sphere (crown radius)
    double r = _torisphericalPtr->GetKnuckleRadius(); // Radius of the torus (knuckle radius)

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

    TopoDS_Edge domeEdge1 = BRepBuilderAPI_MakeEdge(arc1.Value()).Edge();
    TopoDS_Edge domeEdge2 = BRepBuilderAPI_MakeEdge(arc2.Value()).Edge();

    std::vector<TopoDS_Edge> edges = {domeEdge1, domeEdge2};

    if (cylinderLength > 0.0) {
        TopoDS_Vertex v1 = GetLastVertex(domeEdge2);
        TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

        TopoDS_Edge cylinderEdge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();
        edges.push_back(cylinderEdge);
    }

    BuildHullWire(edges, wire);
}

void CCPACSHull::BuildHullWireIsotensoid(BRepBuilderAPI_MakeWire& wire) const
{
    double cylinderRadius = m_cylinderRadius_choice2.get();
    double cylinderLength = m_cylinderLength_choice2.get();

    double polarOpeningRadius = _isotensoidPtr->GetPolarOpeningRadius();
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
    TopoDS_Edge domeEdge              = BRepBuilderAPI_MakeEdge(bspline);

    std::vector<TopoDS_Edge> edges = {domeEdge};

    if (cylinderLength > 0.0) {
        TopoDS_Vertex v1 = GetFirstVertex(domeEdge);
        TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(gp_Pnt(0.5 * cylinderLength + h, 0.0, cylinderRadius));

        TopoDS_Edge cylinderEdge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();
        edges.push_back(cylinderEdge);
    }

    BuildHullWire(edges, wire);
}

void CCPACSHull::BuildShapeFromSimpleParameters(TopoDS_Shape& loftShape) const
{
    BRepBuilderAPI_MakeWire wire;

    if (m_cylinderRadius_choice2.get() <= 0) {
        throw CTiglError("The cylinder radius (" + std::to_string(m_cylinderRadius_choice2.get()) + ") of hull \"" +
                         GetName() + "\" (uID=\"" + GetUID() + "\") must be larger than 0!");
    }
    if (m_cylinderLength_choice2.get() < 0) {
        throw CTiglError("The cylinder length (" + std::to_string(m_cylinderLength_choice2.get()) + ") of hull \"" +
                         GetName() + "\" (uID=\"" + GetUID() + "\") must be larger than or equal to 0!");
    }

    if (_ellipsoidPtr) {
        BuildHullWireEllipsoid(wire);
    }
    else if (_torisphericalPtr) {
        BuildHullWireTorispherical(wire);
    }
    else if (_isotensoidPtr) {
        BuildHullWireIsotensoid(wire);
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

    if (m_sections_choice1) {
        BuildShapeFromSegments(loftShape);
        PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
        SetFaceTraits(loft);
        return loft;
    }
    else if (m_domeType_choice2) {
        EvaluateDome();
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
    // ToDo: This functionality could also be implemented for parametric hulls, if needed
    if (m_segments_choice1) {
        return ((CCPACSFuselageSegment&)GetSegment(segmentIndex)).GetPoint(eta, zeta, _getPointBehavior);
    }
    else {
        throw CTiglError(_hullTypeException);
    }
}

// Sets the GetPoint behavior to asParameterOnSurface or onLinearLoft
void CCPACSHull::SetGetPointBehavior(TiglGetPointBehavior behavior)
{
    _getPointBehavior = behavior;
}

TiglGetPointBehavior CCPACSHull::GetGetPointBehavior() const
{
    return _getPointBehavior;
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
