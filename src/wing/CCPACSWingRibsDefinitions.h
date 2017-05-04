/* 
* Copyright (C) 2016 Airbus Defence and Space
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
#ifndef CCPACSWINGRIBSDEFINITIONS_H
#define CCPACSWINGRIBSDEFINITIONS_H

#include "generated/CPACSWingRibsDefinitions.h"

namespace tigl
{
// forward declarations
class CCPACSWingRibsDefinition;
class CCPACSWingCSStructure;

class CCPACSWingRibsDefinitions : public generated::CPACSWingRibsDefinitions
{
public:
    TIGL_EXPORT CCPACSWingRibsDefinitions(CCPACSWingCSStructure* structure, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT int GetRibsDefinitionCount() const;

    TIGL_EXPORT const CCPACSWingRibsDefinition& GetRibsDefinition(int index) const;
    TIGL_EXPORT CCPACSWingRibsDefinition& GetRibsDefinition(int index);

    TIGL_EXPORT const CCPACSWingRibsDefinition& GetRibsDefinition(const std::string& uid) const;
    TIGL_EXPORT CCPACSWingRibsDefinition& GetRibsDefinition(const std::string& uid);
};

} // end namespace tigl

#endif // CCPACSWINGRIBSDEFINITIONS_H
