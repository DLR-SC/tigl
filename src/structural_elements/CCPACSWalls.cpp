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

#include "CCPACSWalls.h"

namespace tigl
{

CCPACSWalls::CCPACSWalls(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWalls (parent, uidMgr)
{
}

const CCPACSFuselageWallSegment& CCPACSWalls::GetWallSegment(const std::string &uid) const
{
    const auto& wallSegmentsVector = GetWallSegments().GetWallSegments();
    
    for (const std::unique_ptr<CCPACSFuselageWallSegment>& current : wallSegmentsVector) {
        if (current->GetUID().value_or("") == uid) {
            return *current.get();
        }
    }

    throw CTiglError("Wall Segment '" + uid + "' not found.", TIGL_UID_ERROR);
}

const CCPACSWallPosition &CCPACSWalls::GetWallPosition(const std::string &uid) const
{
    const auto& wallPositionsVector = GetWallPositions().GetWallPositions();
    
    for (const std::unique_ptr<CCPACSWallPosition>& current : wallPositionsVector ) {
        if (current->GetUID().value_or("") == uid) {
            return *current.get();
        }
    }

    throw CTiglError("Wall Position '" + uid + "' not found.", TIGL_UID_ERROR);

}

} // namespace tigl
