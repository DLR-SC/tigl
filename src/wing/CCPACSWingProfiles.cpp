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
* @brief  Implementation of CPACS wing profiles handling routines.
*/

#include <iostream>
#include <algorithm>

#include "generated/CPACSWingAirfoils.h"
#include "CCPACSWingProfile.h"
#include "tigl_internal.h"
#include "CCPACSWingProfiles.h"
#include "CTiglError.h"
#include "generated/TixiHelper.h"
#include "generated/UniquePtr.h"
#include "CTiglLogging.h"

namespace tigl
{
CCPACSWingProfiles::CCPACSWingProfiles(CTiglUIDManager* uidMgr)
    : generated::CPACSWingAirfoils(uidMgr) {}

// Invalidates internal state
void CCPACSWingProfiles::Invalidate()
{
    for (int i = 0; i < m_wingAirfoils.size(); i++) {
        static_cast<CCPACSWingProfile&>(*m_wingAirfoils[i]).Invalidate();
    }
}

// Read CPACS wing profiles
void CCPACSWingProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // call separate import method for reading (allows for importing
    // additional profiles from other files)
    m_wingAirfoils.clear();
    ImportCPACS(tixiHandle, xpath);
}

void CCPACSWingProfiles::ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // we replace generated::CPACSWingAirfoils::ReadCPACS and not call it to allow instantiation of CCPACSWingProfile instead of generated::CPACSProfileGeometry
    // read element wingAirfoil
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/wingAirfoil")) {
        tixi::TixiReadElements(tixiHandle, xpath + "/wingAirfoil", m_wingAirfoils, tixi::ChildWithArgsReader1<CCPACSWingProfile, CTiglUIDManager>(m_uidMgr));
    }
}

CCPACSWingProfile& CCPACSWingProfiles::AddWingAirfoil() {
    m_wingAirfoils.push_back(make_unique<CCPACSWingProfile>(m_uidMgr));
    return static_cast<CCPACSWingProfile&>(*m_wingAirfoils.back());
}

bool CCPACSWingProfiles::HasProfile(std::string uid) const
{
    for (std::vector<unique_ptr<CCPACSProfileGeometry> >::const_iterator it = m_wingAirfoils.begin(); it != m_wingAirfoils.end(); ++it)
        if ((*it)->GetUID() == uid)
            return true;

    return false;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(std::string uid) const
{
    for (std::vector<unique_ptr<CCPACSProfileGeometry> >::const_iterator it = m_wingAirfoils.begin(); it != m_wingAirfoils.end(); ++it)
        if ((*it)->GetUID() == uid)
            return static_cast<CCPACSWingProfile&>(**it);
    throw CTiglError("Fuselage profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}


int CCPACSWingProfiles::GetProfileCount() const
{
    return static_cast<int>(m_wingAirfoils.size());
}

// Returns the fuselage profile for a given uid.
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(int index) const
{
    index--;
    if (index < 0 || index >= m_wingAirfoils.size()) {
        throw CTiglError("Invalid index in CCPACSWingProfiles::GetProfile", TIGL_INDEX_ERROR);
    }
    return static_cast<CCPACSWingProfile&>(*m_wingAirfoils[index]);
}

} // end namespace tigl
