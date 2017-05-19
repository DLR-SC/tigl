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
CCPACSFuselages::CCPACSFuselages(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselages(parent, uidMgr) {}

CCPACSFuselages::CCPACSFuselages(CCPACSRotorcraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselages(parent, uidMgr) {}

// Invalidates internal state
void CCPACSFuselages::Invalidate()
{
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

// Returns the total count of fuselages in a configuration
int CCPACSFuselages::GetFuselageCount() const
{
    return static_cast<int>(m_fuselages.size());
}

// Returns the fuselage for a given index.
CCPACSFuselage& CCPACSFuselages::GetFuselage(int index) const
{
    index--;
    if (index < 0 || index >= GetFuselageCount()) {
        throw CTiglError("Error: Invalid index in CCPACSFuselages::GetFuselage", TIGL_INDEX_ERROR);
    }
    return *m_fuselages[index];
}

// Returns the fuselage for a given UID.
CCPACSFuselage& CCPACSFuselages::GetFuselage(const std::string& UID) const
{
    return *m_fuselages[GetFuselageIndex(UID) - 1];
}

// Returns the fuselage index for a given UID.
int CCPACSFuselages::GetFuselageIndex(const std::string& UID) const
{
    for (int i=0; i < GetFuselageCount(); i++) {
        const std::string tmpUID(m_fuselages[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSFuselages::GetFuselageIndex", TIGL_UID_ERROR);
}

void CCPACSFuselages::Add(CCPACSFuselage* fuselage)
{
    m_fuselages.push_back(unique_ptr<CCPACSFuselage>(fuselage));
}

void CCPACSFuselages::Remove(CCPACSFuselage* fuselage)
{
    for (std::size_t i = 0; i < m_fuselages.size(); i++) {
        if (m_fuselages[i].get() == fuselage) {
            m_fuselages.erase(m_fuselages.begin() + i);
            return;
        }
    }
    throw std::runtime_error("Fuselage not found");
}

} // end namespace tigl
