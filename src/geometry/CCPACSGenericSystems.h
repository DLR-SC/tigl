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

#include "generated/CPACSGenericSystems.h"

namespace tigl
{

class CCPACSConfiguration;
class CCPACSGenericSystem;

class CCPACSGenericSystems : public generated::CPACSGenericSystems
{
public:
    // Constructor
    TIGL_EXPORT CCPACSGenericSystems(CCPACSACSystems* parent, CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSGenericSystems();

    // Returns the total count of wings in a configuration
    TIGL_EXPORT int GetGenericSystemCount() const;

    // Returns the wing for a given index.
    TIGL_EXPORT CCPACSGenericSystem& GetGenericSystem(int index) const;

    // Returns the wing for a given UID.
    TIGL_EXPORT CCPACSGenericSystem& GetGenericSystem(const std::string& UID) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

private:
    // Copy constructor
    CCPACSGenericSystems(const CCPACSGenericSystems& );

    // Assignment operator
    void operator=(const CCPACSGenericSystem& );

};

} // end namespace tigl

#endif // CCPACSGENERICSYSTEMS_H
