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
* @brief  Implementation of CPACS fuselages handling routines.
*/

#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>
#include <sstream>

namespace tigl
{
//CCPACSFuselages::CCPACSFuselages() {}

CCPACSFuselages::CCPACSFuselages(CCPACSModel* parent)
    : generated::CPACSFuselages(parent) {}

CCPACSFuselages::CCPACSFuselages(generated::CPACSRotorcraftModel* parent)
    : generated::CPACSFuselages(parent) {}

// Invalidates internal state
void CCPACSFuselages::Invalidate()
{
    profiles.Invalidate();
    for (int i = 1; i <= GetFuselageCount(); i++) {
        GetFuselage(i).Invalidate();
    }
}

namespace {
    const std::string profilesXPath = "/cpacs/vehicles/profiles/fuselageProfiles";
}

// Read CPACS fuselages element
void CCPACSFuselages::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    generated::CPACSFuselages::ReadCPACS(tixiHandle, xpath);
    profiles.ReadCPACS(tixiHandle, profilesXPath);
}

// Write CPACS fuselage elements
void CCPACSFuselages::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    profiles.WriteCPACS(tixiHandle, profilesXPath);
    generated::CPACSFuselages::WriteCPACS(tixiHandle, xpath);
}

bool CCPACSFuselages::HasProfile(std::string uid) const
{
    return profiles.HasProfile(uid);
}

// Returns the total count of fuselage profiles in this configuration
int CCPACSFuselages::GetProfileCount() const
{
    return profiles.GetProfileCount();
}

CCPACSFuselageProfiles& CCPACSFuselages::GetProfiles() 
{
    return profiles;
}

// Returns the fuselage profile for a given index.
CCPACSFuselageProfile& CCPACSFuselages::GetProfile(int index) const
{
    return profiles.GetProfile(index);
}

// Returns the fuselage profile for a given uid.
CCPACSFuselageProfile& CCPACSFuselages::GetProfile(std::string uid) const
{
    return profiles.GetProfile(uid);
}

// Returns the total count of fuselages in a configuration
int CCPACSFuselages::GetFuselageCount() const
{
    return static_cast<int>(m_fuselage.size());
}

// Returns the fuselage for a given index.
CCPACSFuselage& CCPACSFuselages::GetFuselage(int index) const
{
    index--;
    if (index < 0 || index >= GetFuselageCount()) {
        throw CTiglError("Error: Invalid index in CCPACSFuselages::GetFuselage", TIGL_INDEX_ERROR);
    }
    return *m_fuselage[index];
}

// Returns the fuselage for a given index.
CCPACSFuselage& CCPACSFuselages::GetFuselage(const std::string& UID) const
{
    for (int i=0; i < GetFuselageCount(); i++) {
        const std::string tmpUID(m_fuselage[i]->GetUID());
        if (tmpUID == UID) {
            return *m_fuselage[i];
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid index in CCPACSFuselages::GetFuselage", TIGL_INDEX_ERROR);
}


void CCPACSFuselages::AddFuselage(CCPACSFuselage* fuselage)
{
    // Check whether the same fuselage already exists if yes remove it before adding the new one
    decltype(m_fuselage)::iterator it;
    for (it = m_fuselage.begin(); it != m_fuselage.end(); ++it) {
        if ((*it)->GetUID() == fuselage->GetUID()) {
            delete (*it);
            m_fuselage.erase(it);
            break;
        }
    }

    // Add the new fuselage to the fuselage list
    m_fuselage.push_back(fuselage);
}



} // end namespace tigl
