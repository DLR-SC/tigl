/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-10-31 Martin Siggel <martin.siggel@dlr.de>
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


#include "CCPACSEnginePylons.h"

#include "CTiglError.h"
#include "CCPACSEnginePylon.h"

namespace tigl
{

CCPACSEnginePylons::CCPACSEnginePylons(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEnginePylons(parent, uidMgr)
{
}

// Invalidates internal state
void CCPACSEnginePylons::Invalidate()
{
    for (std::size_t i = 0; i < m_enginePylons.size(); i++) {
        m_enginePylons[i]->Invalidate();
    }
}

// Gets a pylon by index. 
CCPACSEnginePylon& CCPACSEnginePylons::GetEnginePylon(int index)
{
    index--;
    if (index < 0 || index >= GetPylonCount()) {
        throw CTiglError("Invalid index value in CCPACSEnginePylons::GetEnginePylon", TIGL_INDEX_ERROR);
    }
    return *m_enginePylons[index];
}

const CCPACSEnginePylon& CCPACSEnginePylons::GetEnginePylon(int index) const
{
    index--;
    if (index < 0 || index >= GetPylonCount()) {
        throw CTiglError("Invalid index value in CCPACSEnginePylons::GetEnginePylon", TIGL_INDEX_ERROR);
    }
    return *m_enginePylons[index];
}

// Gets a pylon by uid. 
CCPACSEnginePylon& CCPACSEnginePylons::GetEnginePylon(const std::string& segmentUID)
{
    for (std::size_t i = 0; i < m_enginePylons.size(); i++) {
        if (m_enginePylons[i]->GetUID() == segmentUID) {
            return *m_enginePylons[i];
        }
    }
    throw CTiglError("Invalid uid in CCPACSEnginePylons::GetEnginePylon", TIGL_UID_ERROR);
}

const CCPACSEnginePylon& CCPACSEnginePylons::GetEnginePylon(const std::string& segmentUID) const
{
    for (std::size_t i = 0; i < m_enginePylons.size(); i++) {
        if (m_enginePylons[i]->GetUID() == segmentUID) {
            return *m_enginePylons[i];
        }
    }
    throw CTiglError("Invalid uid in CCPACSEnginePylons::GetEnginePylon", TIGL_UID_ERROR);
}

// Gets total pylon count
int CCPACSEnginePylons::GetPylonCount() const
{
    return static_cast<int>(m_enginePylons.size());
}

} // namespace tigl
