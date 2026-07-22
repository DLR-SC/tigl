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

#include "CCPACSDuct.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglMakeLoft.h"
#include "CNamedShape.h"
#include "CTiglTopoAlgorithms.h"
#include "tiglcommonfunctions.h"

namespace tigl {

CCPACSDuct::CCPACSDuct(CCPACSDucts* parent, CTiglUIDManager* uidMgr)
  : generated::CPACSDuct(parent, uidMgr)
  , CTiglRelativelyPositionedComponent(static_cast<std::string*>(nullptr), &m_transformation, &m_symmetry)
  , loftUntrimmed(*this, &CCPACSDuct::BuildLoftUntrimmed)
  , loftTrimmed(*this, &CCPACSDuct::BuildLoftTrimmed)
{}

CCPACSConfiguration& CCPACSDuct::GetConfiguration() const
{
    return GetParent()->GetParent()->GetConfiguration();
}

std::string CCPACSDuct::GetDefaultedUID() const
{
    return generated::CPACSDuct::GetUID();
}

TiglGeometricComponentType CCPACSDuct::GetComponentType() const
{
    return TIGL_COMPONENT_DUCT;
}

TiglGeometricComponentIntent CCPACSDuct::GetComponentIntent() const
{
    // needs to be physical, so that transformation relative to parent works
    return TIGL_INTENT_PHYSICAL;
}

PNamedShape CCPACSDuct::BuildLoft() const
{
    return *loftUntrimmed;
}

void CCPACSDuct::BuildLoftUntrimmed(PNamedShape& cache) const
{
    BuildLoftImpl(cache, false);
}

void CCPACSDuct::BuildLoftTrimmed(PNamedShape& cache) const
{
    BuildLoftImpl(cache, true);
}

void CCPACSDuct::BuildLoftImpl(PNamedShape& cache, bool trim) const
{
    TiglContinuity cont = m_segments.GetSegment(1).GetContinuity();
    Standard_Boolean smooth = (cont == ::C0? false : true);

    CTiglMakeLoft lofter;
    lofter.setMakeSolid(true);
    lofter.setMakeSmooth(smooth);
    // Only the trimmed loft is cut at the profiles; the untrimmed loft is a
    // single continuous surface.
    lofter.setEnableProfileCutting(trim);

    for (int i=1; i <= m_segments.GetSegmentCount(); i++) {
        lofter.addProfiles(m_segments.GetSegment(i).GetStartWire());
    }
    lofter.addProfiles(m_segments.GetSegment(m_segments.GetSegmentCount()).GetEndWire());

    lofter.addGuides(m_segments.GetGuideCurveWires());

    TopoDS_Shape loftShape = lofter.Shape();

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    cache = std::make_shared<CNamedShape>(loftShape, loftName.c_str(), loftShortName.c_str());

    // The trimmed loft has one face group per segment, whereas the untrimmed
    // loft's aerodynamic faces form a single continuous group.
    SetFaceTraits(cache, trim ? m_segments.GetSegmentCount() : 1);
}

// get short name for loft
std::string CCPACSDuct::GetShortShapeName() const
{
    unsigned int findex = 0;
    unsigned int i = 0;

    for (auto& d: GetParent()->GetDucts()) {
        ++i;
        if (GetUID() == d->GetUID()) {
            findex = i;
            std::stringstream shortName;
            shortName << "D" << findex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

void CCPACSDuct::SetFaceTraits(PNamedShape loft, int nSegments) const
{
    // Face layout: [aerodynamic faces][optional symmetry faces][front/rear caps].
    // For the trimmed loft the aerodynamic (and symmetry) faces are grouped per
    // segment (nSegments > 1); for the untrimmed loft they form a single group
    // (nSegments == 1).
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

    int facesPerSegment = FacesPerSegment(nFacesAero, nSegments);

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

    int iFace = 2;
    for (;iFaceTotal < nFacesTotal; ++iFaceTotal, ++iFace) {
        if (iFace < (int)names.size()) {
            loft->FaceTraits(iFaceTotal).SetName(names[iFace].c_str());
        }
    }
}

PNamedShape CCPACSDuct::GetUntrimmedLoft() const
{
    // GetLoft() returns the untrimmed loft, so this is the untrimmed loft by
    // definition.
    return GetLoft();
}

PNamedShape CCPACSDuct::GetTrimmedLoft() const
{
    return *loftTrimmed;
}

void CCPACSDuct::RegisterInvalidationCallback(std::function<void()> const& fn){
    invalidationCallbacks.push_back(fn);
}

void CCPACSDuct::InvalidateImpl(const boost::optional<std::string>&) const
{
    loftTrimmed.clear();
    loftUntrimmed.clear();
    CTiglAbstractGeometricComponent::Reset();
    for (auto const& invalidator: invalidationCallbacks) {
        invalidator();
    }
}

} //namespace tigl
