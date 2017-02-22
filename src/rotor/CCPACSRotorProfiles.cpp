/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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

#include "CCPACSRotorProfiles.h"
#include "CCPACSWingProfile.h"
#include "CTiglError.h"
#include "generated/TixiHelper.h"
#include "generated/UniquePtr.h"

namespace tigl
{
// Invalidates internal state
void CCPACSRotorProfiles::Invalidate()
{
    for (auto& p : m_rotorAirfoil) {
        static_cast<CCPACSWingProfile&>(*p).Invalidate();
    }
}

// Read CPACS wing profiles
void CCPACSRotorProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // call separate import method for reading (allows for importing
    // additional profiles from other files)
    m_rotorAirfoil.clear();
    ImportCPACS(tixiHandle, xpath);
}

void CCPACSRotorProfiles::ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // we replace generated::CPACSRotorAirfoils::ReadCPACS and not call it to allow instantiation of CCPACSWingProfile instead of generated::CPACSProfileGeometry
    // read element wingAirfoil
    if (tixihelper::TixiCheckElement(tixiHandle, xpath + "/rotorAirfoil")) {
        tixihelper::TixiReadElements(tixiHandle, xpath + "/rotorAirfoil", m_rotorAirfoil, tixihelper::ChildReader<CCPACSWingProfile>());
    }
}

void CCPACSRotorProfiles::AddProfile(CCPACSWingProfile* profile)
{
    // free memory for existing profiles
    DeleteProfile(profile->GetUID());
    m_rotorAirfoil.emplace_back(profile);
}


void CCPACSRotorProfiles::DeleteProfile(std::string uid)
{
    const auto it = std::find_if(std::begin(m_rotorAirfoil), std::end(m_rotorAirfoil), [&](const unique_ptr<generated::CPACSProfileGeometry>& pg) {
        return pg->GetUID() == uid;
    });
    if (it != std::end(m_rotorAirfoil))
        m_rotorAirfoil.erase(it);
}

// Returns the total count of wing profiles in this configuration
int CCPACSRotorProfiles::GetProfileCount() const {
	return static_cast<int>(m_rotorAirfoil.size());
}

bool CCPACSRotorProfiles::HasProfile(std::string uid) const
{
    for (const auto& p : m_rotorAirfoil)
        if (p->GetUID() == uid)
            return true;

    return false;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSRotorProfiles::GetProfile(std::string uid) const
{
    for (const auto& p : m_rotorAirfoil)
        if (p->GetUID() == uid)
            return static_cast<CCPACSWingProfile&>(*p);

    throw CTiglError("Rotor profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

// Returns the wing profile for a given index - TODO: depricated function!
CCPACSWingProfile& CCPACSRotorProfiles::GetProfile(int index) const {
	index--;
	if (index < 0 || index >= GetProfileCount()) {
		throw CTiglError("Illegal index in CCPACSRotorProfiles::GetProfile", TIGL_INDEX_ERROR);
	}
	return static_cast<CCPACSWingProfile&>(*m_rotorAirfoil[index]);
}

} // end namespace tigl
