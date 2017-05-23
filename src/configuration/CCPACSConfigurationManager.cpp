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
* @brief  Implementation of routines for managing CPACS configurations by handle.
*/

#include "CCPACSConfigurationManager.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSConfigurationManager::CCPACSConfigurationManager()
    : configurations()
    , handleCounter(0)
{
}

// Destructor
CCPACSConfigurationManager::~CCPACSConfigurationManager()
{
    // Delete all remaining configurations in the configuration container
    for (CCPACSConfigConstIterator iter = configurations.begin(); iter != configurations.end(); iter++) {
        CCPACSConfiguration* config = iter->second;
        delete config;
    }
    configurations.clear();
}

// Returns a reference to the only instance of this class
CCPACSConfigurationManager& CCPACSConfigurationManager::GetInstance()
{
    static CCPACSConfigurationManager instance;
    return instance;
}

// Adds a configuration to the configuration container and returns a handle for it.
TiglCPACSConfigurationHandle CCPACSConfigurationManager::AddConfiguration(CCPACSConfiguration* config)
{
    if (config == 0) {
        throw CTiglError("Null pointer argument for CCPACSConfiguration in CCPACSConfigurationManager::AddConfiguration", TIGL_NULL_POINTER);
    }

    handleCounter++;

    if (handleCounter < 1) {
        // handleCounter could be less than 1 because of a range overflow. Very unlikely, but who knows ;-)
        throw CTiglError("Can't generate a valid handle in CPACSConfigurationManager::AddConfiguration", TIGL_ERROR);
    }
 
    CCPACSConfigConstIterator iter = configurations.find(handleCounter);
    if (iter != configurations.end()) {
        // handle could already exist because of a range overflow for handleCounter. Very unlikely too ;-)
        throw CTiglError("Generated handle already exists in CPACSConfigurationManager::AddConfiguration", TIGL_ERROR);
    }

    configurations[handleCounter] = config;
    return handleCounter;
}

// Removes and deletes a configuration from the configuration container
void CCPACSConfigurationManager::DeleteConfiguration(TiglCPACSConfigurationHandle handle)
{
    CCPACSConfigIterator iter = configurations.find(handle);
    if (iter == configurations.end()) {
        throw CTiglError("Invalid CPACS configuration handle in CCPACSConfigurationManager::DeleteConfiguration", TIGL_NOT_FOUND);
    }

    CCPACSConfiguration* config = iter->second;
    delete config;
    configurations.erase(iter);
}

// Returns the configuration for a given handle
CCPACSConfiguration& CCPACSConfigurationManager::GetConfiguration(TiglCPACSConfigurationHandle handle) const
{
    CCPACSConfigConstIterator iter = configurations.find(handle);
    if (iter == configurations.end()) {
        throw CTiglError("Invalid CPACS configuration handle in CCPACSConfigurationManager::GetConfiguration", TIGL_NOT_FOUND);
    }

    CCPACSConfiguration* config = iter->second;
    return *config;
}

// Tests if a given configuration handle is valid
bool CCPACSConfigurationManager::IsValid(TiglCPACSConfigurationHandle handle) const
{
    CCPACSConfigConstIterator iter = configurations.find(handle);
    return (iter != configurations.end());
}

// Invalidates all configurations and forces recalculation of wires/points etc.
void CCPACSConfigurationManager::Invalidate()
{
    for (CCPACSConfigConstIterator iter = configurations.begin(); iter != configurations.end(); ++iter) {
        CCPACSConfiguration* config = iter->second;
        config->Invalidate();
    }
}

} // end namespace tigl

