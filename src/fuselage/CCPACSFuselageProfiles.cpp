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
* @brief  Implementation of CPACS fuselage profiles handling routines.
*/

#include "generated/UniquePtr.h"
#include "generated/TixiHelper.h"
#include "CCPACSFuselageProfile.h"
#include "CCPACSFuselageProfiles.h"
#include "CTiglError.h"

namespace tigl
{
CCPACSFuselageProfiles::CCPACSFuselageProfiles(CCPACSProfiles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageProfiles(parent, uidMgr) {}

// Invalidates internal state
void CCPACSFuselageProfiles::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 1; i < GetFuselageProfileCount(); i++) {
        GetFuselageProfile(i).Invalidate(source);
    }
}

// Read CPACS fuselage profiles
void CCPACSFuselageProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // we replace generated::CPACSFuselageProfiles::ReadCPACS and not call it to allow instantiation of CCPACSFuselageProfile instead of generated::CPACSProfileGeometry

    // read element fuselageProfile
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/fuselageProfile")) {
        tixi::TixiReadElementsInternal(tixiHandle, xpath + "/fuselageProfile", m_fuselageProfiles, 1, tixi::xsdUnbounded, [&](const std::string& childXPath) {
            auto child = tigl::make_unique<CCPACSFuselageProfile>(this, m_uidMgr);
            child->ReadCPACS(tixiHandle, childXPath);
            return child;
        });
    }
}

CCPACSFuselageProfile& CCPACSFuselageProfiles::AddFuselageProfile() {
    m_fuselageProfiles.push_back(make_unique<CCPACSFuselageProfile>(this, m_uidMgr));
    return static_cast<CCPACSFuselageProfile&>(*m_fuselageProfiles.back());
}

bool CCPACSFuselageProfiles::HasProfile(std::string uid) const
{
    for (const auto& p : m_fuselageProfiles)
        if (p->GetUID() == uid)
            return true;

    return false;
}

void CCPACSFuselageProfiles::AddProfile(CCPACSFuselageProfile* profile)
{
    // free memory for existing profiles
    DeleteProfile(profile->GetUID());
    m_fuselageProfiles.push_back(std::unique_ptr<CCPACSFuselageProfile>(profile));
}

void CCPACSFuselageProfiles::DeleteProfile( std::string uid )
{
    for (auto it = m_fuselageProfiles.begin(); it != m_fuselageProfiles.end(); ++it) {
        if ((*it)->GetUID() == uid) {
            m_fuselageProfiles.erase(it);
            return;
        }
    }
}

// Returns the total count of fuselage profiles in this configuration
size_t CCPACSFuselageProfiles::GetProfileCount() const
{
    return GetFuselageProfileCount();
}

// Returns the fuselage profile for a given uid.
const CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(std::string uid) const
{
    for (auto& p : m_fuselageProfiles)
        if (p->GetUID() == uid)
            return static_cast<CCPACSFuselageProfile&>(*p);
    throw CTiglError("Fuselage profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}
CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(std::string uid)
{
    for (auto& p : m_fuselageProfiles)
        if (p->GetUID() == uid)
            return static_cast<CCPACSFuselageProfile&>(*p);
    throw CTiglError("Fuselage profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

// Returns the fuselage profile for a given index
const CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(size_t index) const
{
    return static_cast<const CCPACSFuselageProfile&>(GetFuselageProfile(index));
}

CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(size_t index)
{
    return static_cast<CCPACSFuselageProfile&>(GetFuselageProfile(index));
}

} // end namespace tigl
