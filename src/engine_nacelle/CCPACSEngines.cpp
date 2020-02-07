/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-01-04 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSEngines.h"
#include "CTiglUIDManager.h"
#include "cassert"

namespace tigl {

CCPACSEngines::CCPACSEngines(CCPACSVehicles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEngines(parent, uidMgr)
{}

TIGL_EXPORT size_t CCPACSEngines::GetEngineCount() const
{
    return GetEngines().size();
}

TIGL_EXPORT CCPACSEngine& CCPACSEngines::GetEngine(size_t index) const
{
    index--;
    if (index >= GetEngineCount()) {
        throw CTiglError("Invalid index in CCPACSEngines::GetEngine", TIGL_INDEX_ERROR);
    }
    return *m_engines[index];
}

TIGL_EXPORT CCPACSEngine& CCPACSEngines::GetEngine(std::string const uid) const
{
    for (std::size_t i = 0; i < m_engines.size(); ++i) {
        if (m_engines[i]->GetUID() == uid) {
            return *m_engines[i];
        }
    }
    throw CTiglError("CCPACSEngines::GetEngine: Engine \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

} //namepsace tigl
