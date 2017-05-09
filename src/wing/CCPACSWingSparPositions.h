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
#ifndef CCPACSWINGSPARPOSITIONS_H
#define CCPACSWINGSPARPOSITIONS_H

#include "generated/CPACSSparPositions.h"

namespace tigl
{

// forward declarations
class CCPACSWingSparPosition;
class CCPACSWingSpars;

class CCPACSWingSparPositions : public generated::CPACSSparPositions
{
private:
    typedef std::vector<CCPACSWingSparPosition*> CCPACSWingSparPositionContainer;

public:
    TIGL_EXPORT CCPACSWingSparPositions(CCPACSWingSpars* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT const CCPACSWingSparPosition& GetSparPosition(const std::string uid) const;
    TIGL_EXPORT CCPACSWingSparPosition& GetSparPosition(const std::string uid);
};

} // end namespace tigl

#endif // CCPACSWINGSPARPOSITIONS_H
