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

#include "generated/CPACSWingAirfoils.h"
#include "CCPACSFuselageProfile.h"
#include "tigl_internal.h"
#include "CCPACSWingProfiles.h"
#include "CTiglError.h"

namespace tigl
{
// Invalidates internal state
void CCPACSWingProfiles::Invalidate(void)
{
    for (CCPACSWingProfileContainer::iterator p = profiles.begin(); p!=profiles.end(); ++p) {
        p->second->Invalidate();
    }
}

// Read CPACS wing profiles
void CCPACSWingProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // TODO: cleanup

    // call separate import method for reading (allows for importing
    // additional profiles from other files)
    profiles.clear();
    ImportCPACS(tixiHandle, xpath);
}

void CCPACSWingProfiles::ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSWingAirfoils::ReadCPACS(tixiHandle, xpath);
    for (const auto& p : GetWingAirfoil())
        profiles[p->GetUID()] = p; // TODO: we have to transfer ownership here
    GetWingAirfoil().clear();
}

// Write CPACS wing profiles
void CCPACSWingProfiles::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
{
    const auto self = const_cast<CCPACSWingProfiles*>(this);
    for (const auto& p : profiles)
        self->GetWingAirfoil().push_back(p.second); // TODO: we have to transfer ownership here
    generated::CPACSWingAirfoils::WriteCPACS(tixiHandle, xpath);
    self->GetWingAirfoil().clear();
}

void CCPACSWingProfiles::AddProfile(CCPACSWingProfile* profile)
{
    // free memory for existing profiles
    if (profiles.find(profile->GetUID()) != profiles.end()) {
        delete profiles[profile->GetUID()];
    }
    profiles[profile->GetUID()] = profile;
}


void CCPACSWingProfiles::DeleteProfile( std::string uid )
{
    // free memory for existing profiles
    if (profiles.find( uid ) != profiles.end()) {
        profiles[ uid ]->Invalidate();
        delete profiles[ uid ];
        profiles.erase( uid );
    }
}

// Returns the total count of wing profiles in this configuration
int CCPACSWingProfiles::GetProfileCount(void) const
{
    return (static_cast<int>(profiles.size()));
}

bool CCPACSWingProfiles::HasProfile(std::string uid) const
{
    CCPACSWingProfileContainer::const_iterator it = profiles.find(uid);
    if (it != profiles.end() && it->second) {
        return true;
    }
    else {
        return false;
    }
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(std::string uid) const
{
    CCPACSWingProfileContainer::const_iterator it = profiles.find(uid);
    if (it != profiles.end() && it->second) {
        return *(it->second);
    }
    else {
        throw CTiglError("Wing profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
    }
}

// Returns the wing profile for a given index - TODO: depricated function!
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(int index) const
{
    int i = 0;
    CCPACSWingProfileContainer::const_iterator p;
    for (p = profiles.begin(); p!=profiles.end(); ++p) {
        if (i == index-1) {
            break;
        }
        i++;
    }
    return (*p->second);
}


} // end namespace tigl
