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
* @brief  Implementation of CPACS wings handling routines.
*/

#include "CCPACSWings.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Invalidates internal state
void CCPACSWings::Invalidate(void)
{
    profiles.Invalidate();
    for (int i = 1; i <= GetWingCount(); i++) {
        GetWing(i).Invalidate();
    }
}

namespace {
    const std::string profilesXPath = "/cpacs/vehicles/profiles/wingAirfoils";
}

CCPACSWings::CCPACSWings() {}

CCPACSWings::CCPACSWings(generated::CPACSRotorcraftModel* parent)
    : generated::CPACSWings(parent) {}

CCPACSWings::CCPACSWings(CCPACSModel* parent)
    : generated::CPACSWings(parent) {}

// Read CPACS wings element
void CCPACSWings::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    profiles.ReadCPACS(tixiHandle, profilesXPath);
    generated::CPACSWings::ReadCPACS(tixiHandle, xpath);
}

// Write CPACS wings elements
void CCPACSWings::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    generated::CPACSWings::WriteCPACS(tixiHandle, xpath);
    profiles.WriteCPACS(tixiHandle, profilesXPath);
}

bool CCPACSWings::HasProfile(std::string uid) const
{
    return profiles.HasProfile(uid);
}

// Returns the total count of wing profiles in this configuration
int CCPACSWings::GetProfileCount(void) const
{
    return profiles.GetProfileCount();
}

CCPACSWingProfiles& CCPACSWings::GetProfiles(void)
{
    return profiles;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWings::GetProfile(std::string uid) const
{
    return profiles.GetProfile(uid);
}

// Returns the wing profile for a given index.
CCPACSWingProfile& CCPACSWings::GetProfile(int index) const
{
    return profiles.GetProfile(index);
}

// Returns the total count of wings in a configuration
int CCPACSWings::GetWingCount(void) const
{
    return (static_cast<int>(m_wing.size()));
}

// Returns the wing for a given index.
CCPACSWing& CCPACSWings::GetWing(int index) const
{
    index --;
    if (index < 0 || index >= GetWingCount()) {
        throw CTiglError("Error: Invalid index in CCPACSWings::GetWing", TIGL_INDEX_ERROR);
    }
    return *m_wing[index];
}

// Returns the wing for a given UID.
CCPACSWing& CCPACSWings::GetWing(const std::string& UID) const
{
    for (int i=0; i < GetWingCount(); i++) {
        const std::string tmpUID(m_wing[i]->GetUID());
        if (tmpUID == UID) {
            return *m_wing[i];
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSWings::GetWing", TIGL_INDEX_ERROR);
}

void CCPACSWings::AddWing(CCPACSWing* wing)
{
    // Check whether the same wing already exists if yes remove it before adding the new one
    for (auto it = m_wing.begin(); it != m_wing.end(); ++it) {
        if ((*it)->GetUID() == wing->GetUID()) {
            delete (*it);
            m_wing.erase(it);
            break;
        }
    }

    // Add the new wing to the wing list
    m_wing.push_back(wing);
}

} // end namespace tigl
