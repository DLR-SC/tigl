/*
* Copyright (c) 2018 RISC Software GmbH
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

#include "CCPACSFuselageStructure.h"
#include "CCPACSFuselage.h"

namespace tigl
{
CCPACSFuselageStructure::CCPACSFuselageStructure(CCPACSFuselage* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageStructure(parent, uidMgr)
    , ITiglFuselageDuctStructure(parent)
{
}

void CCPACSFuselageStructure::Invalidate(const boost::optional<std::string>& source) const
{
    ITiglFuselageDuctStructure::Invalidate();

    if (m_frames) {
        m_frames->Invalidate(source);
    }
    if (m_stringers) {
        m_stringers->Invalidate(source);
    }
    if (m_pressureBulkheads) {
        m_pressureBulkheads->Invalidate(source);
    }
    if (m_cargoCrossBeamStruts) {
        m_cargoCrossBeamStruts->Invalidate(source);
    }
    if (m_longFloorBeams) {
        m_longFloorBeams->Invalidate(source);
    }
    if (m_walls) {
        m_walls->Invalidate(source);
    }
}

} // namespace tigl
