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

#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopExp.hxx"


namespace tigl {

CCPACSHull::CCPACSHull(CCPACSHulls* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSHull(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(GetParent()->GetParent(), &m_transformation)
{}

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
    return m_sections.GetSectionCount();
}

CCPACSFuselageSection& CCPACSHull::GetSection(int index) const
{
    return m_sections.GetSection(index);
}

TopoDS_Shape CCPACSHull::GetSectionFace(const std::string section_uid) const
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
    throw CTiglError("GetSectionFace: Could not find a section for the given UID");
    return TopoDS_Shape();
}

int CCPACSHull::GetSegmentCount() const
{
    return m_segments.GetSegmentCount();
}

CCPACSFuselageSegment& CCPACSHull::GetSegment(const int index)
{
    return m_segments.GetSegment(index);
}

const CCPACSFuselageSegment& CCPACSHull::GetSegment(const int index) const
{
    return m_segments.GetSegment(index);
}

CCPACSFuselageSegment& CCPACSHull::GetSegment(std::string uid)
{
    return m_segments.GetSegment(uid);
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

double CCPACSHull::GetCircumference(const int segmentIndex, const double eta)
{
    return static_cast<CCPACSFuselageSegment&>(GetSegment(segmentIndex)).GetCircumference(eta);
}

PNamedShape CCPACSHull::BuildLoft() const
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
    lofter.addGuides(m_segments.GetGuideCurveWires());

    lofter.setMakeSolid(true);
    lofter.setMakeSmooth(smooth);

    TopoDS_Shape loftShape =  lofter.Shape();

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(loft);

    return loft;
}

gp_Pnt CCPACSHull::GetPoint(int segmentIndex, double eta, double zeta)
{
    return ((CCPACSFuselageSegment &) GetSegment(segmentIndex)).GetPoint(eta, zeta, getPointBehavior);
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

std::vector<gp_Pnt> CCPACSHull::GetGuideCurvePoints() const
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

// get short name for loft
std::string CCPACSHull::GetShortShapeName() const
{
    unsigned int findex = 0;
    unsigned int i = 0;

    for (auto& h: GetParent()->GetHulls()) {
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

void CCPACSHull::SetFaceTraits (PNamedShape loft) const
{
    int nFacesTotal = GetNumberOfFaces(loft->Shape());
    int nFacesAero = nFacesTotal;
    bool hasSymmetryPlane = GetNumberOfEdges(m_segments.GetSegment(1).GetEndWire()) > 1;

    std::vector<std::string> names;
    names.push_back(loft->Name());
    names.push_back("symmetry");
    names.push_back("Front");
    names.push_back("Rear");

    if (!CTiglTopoAlgorithms::IsDegenerated(m_segments.GetSegment(1).GetStartWire())) {
          nFacesAero-=1;
    }
    if (!CTiglTopoAlgorithms::IsDegenerated(m_segments.GetSegment(m_segments.GetSegmentCount()).GetEndWire())) {
          nFacesAero-=1;
    }

    // if we have a smooth surface, the whole fuslage is treatet as one segment
    int nSegments = m_segments.GetSegmentCount();

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

} //namespace tigl
