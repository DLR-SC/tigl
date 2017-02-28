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

#ifndef CCPACSACSYSTEMS_H
#define CCPACSACSYSTEMS_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSGenericSystems.h"

namespace tigl
{

class CCPACSConfiguration;

class CCPACSACSystems
{

public:
    // Constructor
    TIGL_EXPORT CCPACSACSystems(CCPACSConfiguration* config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSACSystems();

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS aircraft systems elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& configurationUID);

    // Returns the generic systems object.
    TIGL_EXPORT CCPACSGenericSystems& GetGenericSystems();

protected:
    // Cleanup routine
    void Cleanup();

private:
    // Copy constructor
    CCPACSACSystems(const CCPACSACSystems& );

    // Assignment operator
    void operator=(const CCPACSACSystems& );

private:
    CCPACSGenericSystems    genericSystems;    /**< Generic system elements */
    CCPACSConfiguration*    configuration;     /**< Pointer to parent configuration */

};

} // end namespace tigl

#endif // CCPACSACSYSTEMS_H
