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
* @brief  Implementation of CPACS wing profiles handling routines.
*/

#include <iostream>
#include <algorithm>

#include "generated/CPACSWingAirfoils.h"
#include "CCPACSWingProfile.h"
#include "tigl_internal.h"
#include "CCPACSWingProfiles.h"
#include "CTiglError.h"
#include "generated/IOHelper.h"

namespace tigl
{
// Invalidates internal state
void CCPACSWingProfiles::Invalidate(void)
{
    for (decltype(m_wingAirfoil)::iterator p = m_wingAirfoil.begin(); p != m_wingAirfoil.end(); ++p) {
        static_cast<CCPACSWingProfile*>(*p)->Invalidate();
    }
}

// Read CPACS wing profiles
void CCPACSWingProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // call separate import method for reading (allows for importing
    // additional profiles from other files)
    m_wingAirfoil.clear();
    ImportCPACS(tixiHandle, xpath);
}

void CCPACSWingProfiles::ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // we replace generated::CPACSWingAirfoils::ReadCPACS and not call it to allow instantiation of CCPACSWingProfile instead of generated::CPACSProfileGeometry

    // read base
    CPACSComplexBase::ReadCPACS(tixiHandle, xpath);

    // read element wingAirfoil
    if (TixiCheckElement(tixiHandle, xpath, "wingAirfoil")) {
        TixiReadElements(tixiHandle, xpath, "wingAirfoil", m_wingAirfoil, [&](const std::string& childXPath) {
            CCPACSWingProfile* child = new CCPACSWingProfile;
            child->ReadCPACS(tixiHandle, childXPath);
            return child;
        });
    }
}

void CCPACSWingProfiles::AddProfile(CCPACSWingProfile* profile)
{
    // free memory for existing profiles
    DeleteProfile(profile->GetUID());
    m_wingAirfoil.push_back(profile);
}


void CCPACSWingProfiles::DeleteProfile(std::string uid)
{
    const auto it = std::find_if(std::begin(m_wingAirfoil), std::end(m_wingAirfoil), [&](const generated::CPACSProfileGeometry* pg) {
        return pg->GetUID() == uid;
    });
    if (it != std::end(m_wingAirfoil))
        m_wingAirfoil.erase(it);
}

// Returns the total count of wing profiles in this configuration
int CCPACSWingProfiles::GetProfileCount(void) const
{
    return static_cast<int>(m_wingAirfoil.size());
}

bool CCPACSWingProfiles::HasProfile(std::string uid) const
{
    for (const auto& p : m_wingAirfoil)
        if (p->GetUID() == uid)
            return true;

    return false;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(std::string uid) const
{
    for (const auto& p : m_wingAirfoil)
        if (p->GetUID() == uid)
            return *static_cast<CCPACSWingProfile*>(p);

    throw CTiglError("Wing profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

// Returns the wing profile for a given index - TODO: depricated function!
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(int index) const
{
    return *static_cast<CCPACSWingProfile*>(m_wingAirfoil[index]);
}

} // end namespace tigl
