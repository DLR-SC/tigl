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

#ifndef CCPACSGENERICSYSTEMS_H
#define CCPACSGENERICSYSTEMS_H

#include "tigl_internal.h"
#include "tixi.h"

#include <vector>
#include <string>

namespace tigl
{

class CCPACSConfiguration;
class CCPACSGenericSystem;

class CCPACSGenericSystems
{
public:
    // Constructor
    TIGL_EXPORT CCPACSGenericSystems(CCPACSConfiguration* config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSGenericSystems(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS generic system elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID);

    // Returns the total count of wings in a configuration
    TIGL_EXPORT int GetGenericSystemCount(void) const;

    // Returns the wing for a given index.
    TIGL_EXPORT CCPACSGenericSystem& GetGenericSystem(int index) const;

    // Returns the wing for a given UID.
    TIGL_EXPORT CCPACSGenericSystem& GetGenericSystem(const std::string& UID) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSGenericSystems(const CCPACSGenericSystems& );

    // Assignment operator
    void operator=(const CCPACSGenericSystem& );

private:
    typedef std::vector<CCPACSGenericSystem*> CCPACSGenericSystemContainer;

    CCPACSGenericSystemContainer    systems;            /**< Generic system elements */
    CCPACSConfiguration*            configuration;      /**< Pointer to parent configuration */
};

} // end namespace tigl

#endif // CCPACSGENERICSYSTEMS_H
