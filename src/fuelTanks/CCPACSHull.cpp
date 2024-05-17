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
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_Transform.hxx>

namespace tigl
{

CCPACSHull::CCPACSHull(CCPACSHulls* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSHull(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(GetParent()->GetParent(), &m_transformation)
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

// ToDo: Add parametric build algorithms
PNamedShape CCPACSHull::BuildLoft() const
{
    if (m_segments_choice1 && m_sections_choice1) {
        TiglContinuity cont     = m_segments_choice1.get().GetSegment(1).GetContinuity();
        Standard_Boolean smooth = (cont == ::C0 ? false : true);

        CTiglMakeLoft lofter;
        // add profiles
        for (int i = 1; i <= m_segments_choice1.get().GetSegmentCount(); i++) {
            lofter.addProfiles(m_segments_choice1.get().GetSegment(i).GetStartWire());
        }
        lofter.addProfiles(
            m_segments_choice1.get().GetSegment(m_segments_choice1.get().GetSegmentCount()).GetEndWire());

        // add guides
        lofter.addGuides(m_segments_choice1.get().GetGuideCurveWires());

        lofter.setMakeSolid(true);
        lofter.setMakeSmooth(smooth);

        TopoDS_Shape loftShape = lofter.Shape();

        std::string loftName      = GetUID();
        std::string loftShortName = GetShortShapeName();
        PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
        SetFaceTraits(loft);

        return loft;
    }
    else if (m_cylinderRadius_choice2 && m_cylinderLength_choice2 && m_domeType_choice2) {
        double cylinderRadius = m_cylinderRadius_choice2.get();
        double cylinderLength = m_cylinderLength_choice2.get();

        double R, r, a, c, axRatio, h, alpha;

        if (m_domeType_choice2.get().GetSpherical_choice1()) {
            R       = cylinderRadius;
            a       = 0.0;
            axRatio = 1.0;
        }
        else if (m_domeType_choice2.get().GetEllipsoid_choice2()) {
            R       = cylinderRadius;
            a       = 0.0;
            axRatio = m_domeType_choice2.get().GetEllipsoid_choice2().get().GetHalfAxisFraction();
        }
        else if (m_domeType_choice2.get().GetTorispherical_choice3()) {
            R       = m_domeType_choice2.get().GetTorispherical_choice3().get().GetDishRadius();
            a       = m_domeType_choice2.get().GetTorispherical_choice3().get().GetKnuckleRadius();
            axRatio = 1.0;
        }

        if (a > 0.0) {
            c     = cylinderRadius - a;
            h     = R - sqrt((a + c - R) * (a - c - R));
            r     = c * (1.0 + pow((R / a - 1), -1.0));
            alpha = acos((r - c) / a);
        }
        else {
            alpha = 0.0;
            h     = R;
        }

        gp_Dir dir(0.0, 1.0, 0.0);

        gp_Pnt p1_l(R - h, 0.0, 0.0);
        gp_Pnt p1_r(cylinderLength - (R - h), 0.0, 0.0);

        gp_Elips ellips_l_gp(gp_Ax2(p1_l, dir), R, R * axRatio);
        gp_Elips ellips_r_gp(gp_Ax2(p1_r, dir), R, R * axRatio);

        GC_MakeArcOfEllipse arc1_l(ellips_l_gp, -M_PI / 2, -alpha, Standard_True);
        GC_MakeArcOfEllipse arc1_r(ellips_r_gp, alpha, M_PI / 2, Standard_True);

        TopoDS_Edge edge1_l = BRepBuilderAPI_MakeEdge(arc1_l.Value()).Edge();
        TopoDS_Edge edge1_r = BRepBuilderAPI_MakeEdge(arc1_r.Value()).Edge();

        TopoDS_Edge edge2_l;
        TopoDS_Edge edge2_r;

        TopoDS_Vertex v1;
        TopoDS_Vertex v2;
        if (a > 0) {
            gp_Pnt p2_l(0, 0.0, cylinderRadius - a);
            gp_Pnt p2_r(cylinderLength, 0.0, cylinderRadius - a);

            gp_Circ circ2_l_gp(gp_Ax2(p2_l, dir), a);
            gp_Circ circ2_r_gp(gp_Ax2(p2_r, dir), a);

            GC_MakeArcOfCircle arc2_l(circ2_l_gp, -alpha, 0, Standard_True);
            GC_MakeArcOfCircle arc2_r(circ2_r_gp, 0, alpha, Standard_True);

            edge2_l = BRepBuilderAPI_MakeEdge(arc2_l.Value()).Edge();
            edge2_r = BRepBuilderAPI_MakeEdge(arc2_r.Value()).Edge();

            v1 = GetLastVertex(edge2_l);
            v2 = GetFirstVertex(edge2_r);
        }
        else {
            v1 = GetLastVertex(edge1_l);
            v2 = GetFirstVertex(edge1_r);
        }

        TopoDS_Edge cylinder_edge = BRepBuilderAPI_MakeEdge(v1, v2).Edge();

        BRepBuilderAPI_MakeWire wire;
        std::vector<TopoDS_Edge> edges;
        if (a > 0) {
            edges = {edge1_l, edge2_l, cylinder_edge, edge2_r, edge1_r};
        }
        else {
            edges = {edge1_l, cylinder_edge, edge1_r};
        }

        for (const auto& edge : edges) {
            wire.Add(edge);
        }

        gp_Ax1 ax(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
        TopoDS_Shape loftShape = BRepPrimAPI_MakeRevol(wire.Shape(), ax).Shape();

        // gp_Trsf translationTrsf;
        // translationTrsf.SetTranslation(gp_Vec(h, 0, 0));
        // BRepBuilderAPI_Transform shapeTranslation(loftShape, translationTrsf);
        // shapeTranslation.Build();
        // TopoDS_Shape loftShape_translated = shapeTranslation.Shape();

        std::string loftName      = GetUID();
        std::string loftShortName = GetShortShapeName();
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
