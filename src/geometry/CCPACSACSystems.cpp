/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-21 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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

#include "CCPACSACSystems.h"
#include "CTiglError.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSACSystems::CCPACSACSystems(CCPACSConfiguration* config)
    : configuration(config)
    , genericSystems(config)
{
    Cleanup();
}

// Destructor
CCPACSACSystems::~CCPACSACSystems(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSACSystems::Invalidate(void)
{
    genericSystems.Invalidate();
}

// Cleanup routine
void CCPACSACSystems::Cleanup(void)
{
    Invalidate();
}

// Read CPACS systems element
void CCPACSACSystems::ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID)
{
    Cleanup();
    char *tmpString = NULL;

    if (tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSACSystems::ReadCPACS", TIGL_XML_ERROR);
    }

    // Read genericSystems
    genericSystems.ReadCPACS(tixiHandle, configurationUID);

}

// Returns the genericSystems object.
CCPACSGenericSystems& CCPACSACSystems::GetGenericSystems(void)
{
    return genericSystems;
}
} // end namespace tigl
