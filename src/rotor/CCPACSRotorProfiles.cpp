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

#include "CCPACSRotorProfiles.h"
#include "CCPACSWingProfile.h"
#include "CTiglError.h"
#include "generated/TixiHelper.h"
#include "generated/UniquePtr.h"

namespace tigl
{
CCPACSRotorProfiles::CCPACSRotorProfiles(CCPACSProfiles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSRotorAirfoils(parent, uidMgr) {}

// Invalidates internal state
void CCPACSRotorProfiles::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 0; i < m_rotorAirfoils.size(); i++) {
        static_cast<CCPACSWingProfile&>(*m_rotorAirfoils[i]).Invalidate(source);
    }
}

// Read CPACS wing profiles
void CCPACSRotorProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // call separate import method for reading (allows for importing
    // additional profiles from other files)
    m_rotorAirfoils.clear();
    ImportCPACS(tixiHandle, xpath);
}

void CCPACSRotorProfiles::ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // we replace generated::CPACSRotorAirfoils::ReadCPACS and not call it to allow instantiation of CCPACSWingProfile instead of generated::CPACSProfileGeometry
    // read element wingAirfoil
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/rotorAirfoil")) {
        tixi::TixiReadElementsInternal(tixiHandle, xpath + "/rotorAirfoil", m_rotorAirfoils, 1, tixi::xsdUnbounded, [&](const std::string& childXPath) {
            auto child = tigl::make_unique<CCPACSWingProfile>(this, m_uidMgr);
            child->ReadCPACS(tixiHandle, childXPath);
            return child;
        });
    }
}

void CCPACSRotorProfiles::AddProfile(CCPACSWingProfile* profile)
{
    // free memory for existing profiles
    DeleteProfile(profile->GetUID());
    m_rotorAirfoils.push_back(std::unique_ptr<CCPACSWingProfile>(profile));
}


void CCPACSRotorProfiles::DeleteProfile(std::string uid)
{
    for (auto it = m_rotorAirfoils.begin(); it != m_rotorAirfoils.end(); ++it) {
        if ((*it)->GetUID() == uid) {
            m_rotorAirfoils.erase(it);
            return;
        }
    }
}

bool CCPACSRotorProfiles::HasProfile(std::string uid) const
{
    for (const auto& p : m_rotorAirfoils)
        if (p->GetUID() == uid)
            return true;
    return false;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSRotorProfiles::GetProfile(std::string uid) const
{
    for (auto& p : m_rotorAirfoils)
        if (p->GetUID() == uid)
            return static_cast<CCPACSWingProfile&>(*p);

    throw CTiglError("Rotor profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

} // end namespace tigl
