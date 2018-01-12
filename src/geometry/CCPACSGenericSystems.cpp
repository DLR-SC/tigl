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
#include "CTiglError.h"

#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSGenericSystems::CCPACSGenericSystems(CCPACSConfiguration* config)
    : configuration(config)
{
    Cleanup();
}

// Destructor
CCPACSGenericSystems::~CCPACSGenericSystems()
{
    Cleanup();
}

// Cleanup routine
void CCPACSGenericSystems::Cleanup()
{
    for (CCPACSGenericSystemContainer::size_type i = 0; i < systems.size(); i++) {
        delete systems[i];
    }
    systems.clear();
}

// Read CPACS generic system element
void CCPACSGenericSystems::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& configurationUID)
{
    Cleanup();
    char *tmpString = NULL;

    if (tixiUIDGetXPath(tixiHandle, configurationUID.c_str(), &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSGenericSystems::ReadCPACS", TIGL_XML_ERROR);
    }

    std::string genericSysXPath= tmpString;
    genericSysXPath += "[@uID=\"";
    genericSysXPath += configurationUID;
    genericSysXPath+= "\"]/systems/genericSystems";

    if (tixiCheckElement(tixiHandle, genericSysXPath.c_str()) != SUCCESS) {
        return;
    }

    /* Get generic system element count */
    int genericSysCount;
    if (tixiGetNamedChildrenCount(tixiHandle, genericSysXPath.c_str(), "genericSystem", &genericSysCount) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSGenericSystems::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all generic systems
    for (int i = 1; i <= genericSysCount; i++) {
        CCPACSGenericSystem* sys = new CCPACSGenericSystem(configuration);
        systems.push_back(sys);

        std::ostringstream xpath;
        xpath << genericSysXPath << "/genericSystem[" << i << "]";
        sys->ReadCPACS(tixiHandle, xpath.str());
    }
}

// Returns the total count of generic systems in a configuration
int CCPACSGenericSystems::GetGenericSystemCount() const
{
    return (static_cast<int>(systems.size()));
}

// Returns the generic system for a given index.
CCPACSGenericSystem& CCPACSGenericSystems::GetGenericSystem(int index) const
{
    index --;
    if (index < 0 || index >= GetGenericSystemCount()) {
        throw CTiglError("Invalid index in CCPACSGenericSystems::GetGenericSystem", TIGL_INDEX_ERROR);
    }
    return (*systems[index]);
}

// Returns the generic system for a given UID.
CCPACSGenericSystem& CCPACSGenericSystems::GetGenericSystem(const std::string& UID) const
{
    for (int i=0; i < GetGenericSystemCount(); i++) {
        const std::string tmpUID(systems[i]->GetUID());
        if (tmpUID == UID) {
            return (*systems[i]);
        }
    }

    // UID not there
    throw CTiglError("Invalid UID in CCPACSGenericSystems::GetGenericSystem", TIGL_INDEX_ERROR);
}

} // end namespace tigl
