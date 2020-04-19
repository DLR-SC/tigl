/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-02-26 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSEnginePositions.h"
#include "CCPACSEnginePosition.h"
#include "CTiglUIDManager.h"
#include "cassert"

namespace tigl {

CCPACSEnginePositions::CCPACSEnginePositions(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEnginePositions(parent, uidMgr)
{}

CCPACSEnginePositions::CCPACSEnginePositions(CCPACSRotorcraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEnginePositions(parent, uidMgr)
{}


TIGL_EXPORT size_t CCPACSEnginePositions::GetEnginePositionCount() const
{
    return GetEngines().size();
}

TIGL_EXPORT CCPACSEnginePosition& CCPACSEnginePositions::GetEnginePosition(size_t index) const
{
    index--;
    if (index >= GetEnginePositionCount()) {
        throw CTiglError("Invalid index in CCPACSEnginePositions::GetEnginePosition", TIGL_INDEX_ERROR);
    }
    return *m_engines[index];
}

TIGL_EXPORT CCPACSEnginePosition& CCPACSEnginePositions::GetEnginePosition(std::string const uid) const
{
    for (std::size_t i = 0; i < m_engines.size(); ++i) {
        if (m_engines[i]->GetUID() == uid) {
            return *m_engines[i];
        }
    }
    throw CTiglError("CCPACSEnginePositions::GetEnginePosition: EnginePosition \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

} //namepsace tigl
