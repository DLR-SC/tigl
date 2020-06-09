/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-21 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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

#include "CCPACSGenericSystems.h"

#include "CCPACSGenericSystem.h"
#include "CCPACSACSystems.h"
#include "CTiglError.h"


namespace tigl
{

// Constructor
CCPACSGenericSystems::CCPACSGenericSystems(CCPACSACSystems* parent, CTiglUIDManager* uidMgr)
: generated::CPACSGenericSystems(parent, uidMgr)
{
}

// Destructor
CCPACSGenericSystems::~CCPACSGenericSystems()
{
}

// Returns the total count of generic systems in a configuration
int CCPACSGenericSystems::GetGenericSystemCount() const
{
    return static_cast<int>(m_genericSystems.size());
}

// Returns the generic system for a given index.
CCPACSGenericSystem& CCPACSGenericSystems::GetGenericSystem(int index) const
{
    index --;
    if (index < 0 || index >= GetGenericSystemCount()) {
        throw CTiglError("Invalid index in CCPACSGenericSystems::GetGenericSystem", TIGL_INDEX_ERROR);
    }
    return *m_genericSystems[index];
}

// Returns the generic system for a given UID.
CCPACSGenericSystem& CCPACSGenericSystems::GetGenericSystem(const std::string& UID) const
{
    for (int i=0; i < GetGenericSystemCount(); i++) {
        const std::string tmpUID(m_genericSystems[i]->GetUID());
        if (tmpUID == UID) {
            return *m_genericSystems[i];
        }
    }

    // UID not there
    throw CTiglError("Invalid UID in CCPACSGenericSystems::GetGenericSystem", TIGL_INDEX_ERROR);
}

CCPACSConfiguration& CCPACSGenericSystems::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

} // end namespace tigl
