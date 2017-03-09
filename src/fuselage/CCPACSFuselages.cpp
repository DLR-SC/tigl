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
#include "CCPACSAircraftModel.h"
#include "CCPACSFuselageProfiles.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"

namespace tigl
{
CCPACSFuselages::CCPACSFuselages(CCPACSAircraftModel* parent)
    : generated::CPACSFuselages(parent) {}

CCPACSFuselages::CCPACSFuselages(CCPACSRotorcraftModel* parent)
    : generated::CPACSFuselages(parent) {
    throw CTiglError("Instantiating CCPACSFuselages with CPACSRotorcraftModel as parent is not implemented");
}

// Invalidates internal state
void CCPACSFuselages::Invalidate()
{
    GetProfiles().Invalidate();
    for (int i = 1; i <= GetFuselageCount(); i++) {
        GetFuselage(i).Invalidate();
    }
}

// Read CPACS fuselages element
void CCPACSFuselages::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    generated::CPACSFuselages::ReadCPACS(tixiHandle, xpath);
}

// Write CPACS fuselage elements
void CCPACSFuselages::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    generated::CPACSFuselages::WriteCPACS(tixiHandle, xpath);
}

bool CCPACSFuselages::HasProfile(std::string uid) const
{
    return GetProfiles().HasProfile(uid);
}

// Returns the total count of fuselage profiles in this configuration
int CCPACSFuselages::GetProfileCount() const
{
    return GetProfiles().GetProfileCount();
}

CCPACSFuselageProfiles& CCPACSFuselages::GetProfiles() 
{
    return static_cast<CCPACSAircraftModel*>(m_parent)->GetConfiguration().GetFuselageProfiles();
}

const CCPACSFuselageProfiles& CCPACSFuselages::GetProfiles() const {
    return static_cast<const CCPACSAircraftModel*>(m_parent)->GetConfiguration().GetFuselageProfiles();
}

// Returns the fuselage profile for a given index.
CCPACSFuselageProfile& CCPACSFuselages::GetProfile(int index) const
{
    return GetProfiles().GetProfile(index);
}

// Returns the fuselage profile for a given uid.
CCPACSFuselageProfile& CCPACSFuselages::GetProfile(std::string uid) const
{
    return GetProfiles().GetProfile(uid);
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

// Returns the fuselage for a given UID.
CCPACSFuselage& CCPACSFuselages::GetFuselage(const std::string& UID) const
{
    return *m_fuselage[GetFuselageIndex(UID) - 1];
}

// Returns the fuselage index for a given UID.
int CCPACSFuselages::GetFuselageIndex(const std::string& UID) const
{
    for (int i=0; i < GetFuselageCount(); i++) {
        const std::string tmpUID(m_fuselage[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSFuselages::GetFuselageIndex", TIGL_UID_ERROR);
}


void CCPACSFuselages::AddFuselage(CCPACSFuselage* fuselage)
{
    // Check whether the same fuselage already exists if yes remove it before adding the new one
    for (std::vector<unique_ptr<CCPACSFuselage> >::iterator it = m_fuselage.begin(); it != m_fuselage.end(); ++it) {
        if ((*it)->GetUID() == fuselage->GetUID()) {
            m_fuselage.erase(it);
            break;
        }
    }

    // Add the new fuselage to the fuselage list
    m_fuselage.push_back(unique_ptr<CCPACSFuselage>(fuselage));
}

} // end namespace tigl
