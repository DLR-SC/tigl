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

void CCPACSDuct::SetFaceTraits (PNamedShape loft) const
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

void CCPACSDuct::RegisterInvalidationCallback(std::function<void()> const& fn){
    invalidationCallbacks.push_back(fn);
}

void CCPACSDuct::InvalidateImpl(const boost::optional<std::string>&) const
{
    CTiglAbstractGeometricComponent::Reset();
    for (auto const& invalidator: invalidationCallbacks) {
        invalidator();
    }
}

} //namespace tigl
