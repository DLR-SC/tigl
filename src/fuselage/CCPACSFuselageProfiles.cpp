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
CCPACSFuselageProfiles::CCPACSFuselageProfiles(CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageProfiles(uidMgr) {}

// Invalidates internal state
void CCPACSFuselageProfiles::Invalidate()
{
    for (int i = 1; i < GetProfileCount(); i++) {
        GetProfile(i).Invalidate();
    }
}

// Read CPACS fuselage profiles
void CCPACSFuselageProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // we replace generated::CPACSFuselageProfiles::ReadCPACS and not call it to allow instantiation of CCPACSFuselageProfile instead of generated::CPACSProfileGeometry

    // read element fuselageProfile
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/fuselageProfile")) {
        tixi::TixiReadElements(tixiHandle, xpath + "/fuselageProfile", m_fuselageProfiles, tixi::ChildWithArgsReader1<CCPACSFuselageProfile, CTiglUIDManager>(m_uidMgr));
    }
}

bool CCPACSFuselageProfiles::HasProfile(std::string uid) const
{
    for (std::vector<unique_ptr<CCPACSProfileGeometry> >::const_iterator it = m_fuselageProfiles.begin(); it != m_fuselageProfiles.end(); ++it)
        if ((*it)->GetUID() == uid)
            return true;

    return false;
}

void CCPACSFuselageProfiles::AddProfile(CCPACSFuselageProfile* profile)
{
    // free memory for existing profiles
    DeleteProfile(profile->GetUID());
    m_fuselageProfiles.push_back(unique_ptr<CCPACSFuselageProfile>(profile));
}

void CCPACSFuselageProfiles::DeleteProfile( std::string uid )
{
    for (std::vector<unique_ptr<CCPACSProfileGeometry> >::iterator it = m_fuselageProfiles.begin(); it != m_fuselageProfiles.end(); ++it) {
        if ((*it)->GetUID() == uid) {
            m_fuselageProfiles.erase(it);
            return;
        }
    }
}

// Returns the total count of fuselage profiles in this configuration
int CCPACSFuselageProfiles::GetProfileCount() const
{
    return static_cast<int>(m_fuselageProfiles.size());
}

// Returns the fuselage profile for a given uid.
CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(std::string uid) const
{
    for (std::vector<unique_ptr<CCPACSProfileGeometry> >::const_iterator it = m_fuselageProfiles.begin(); it != m_fuselageProfiles.end(); ++it)
        if ((*it)->GetUID() == uid)
            return static_cast<CCPACSFuselageProfile&>(**it);
    throw CTiglError("Fuselage profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

// Returns the fuselage profile for a given index - TODO: depricated function!
CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(int index) const
{
    index--;
    if (index < 0 || index >= m_fuselageProfiles.size()) {
        throw CTiglError("Invalid index in CCPACSFuselageProfiles::GetProfile", TIGL_INDEX_ERROR);
    }
    return static_cast<CCPACSFuselageProfile&>(*m_fuselageProfiles[index]);
}


} // end namespace tigl
