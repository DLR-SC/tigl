/* 
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-05-15 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSWALLS_H
#define CCPACSWALLS_H

#include "generated/CPACSWalls.h"
#include "CCPACSFuselageWallSegment.h"
#include "CCPACSWallPosition.h"

namespace tigl
{

class CCPACSWalls : public generated::CPACSWalls
{
public:
    CCPACSWalls(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);

    const CCPACSFuselageWallSegment& GetWallSegment(const std::string& uid) const;
    const CCPACSWallPosition& GetWallPosition(const std::string& uid) const;
};

} // namespace tigl

#endif // CCPACSWALLS_H
