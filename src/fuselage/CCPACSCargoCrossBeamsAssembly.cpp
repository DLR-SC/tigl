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

#include "CCPACSCargoCrossBeamsAssembly.h"

#include "CCPACSCrossBeamAssemblyPosition.h"

namespace tigl
{
CCPACSCargoCrossBeamsAssembly::CCPACSCargoCrossBeamsAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSCargoCrossBeamsAssembly(parent, uidMgr)
{
}

void CCPACSCargoCrossBeamsAssembly::Invalidate()
{
    for (int i = 0; i < m_cargoCrossBeams.size(); i++) {
        m_cargoCrossBeams[i]->Invalidate();
    }
}

} // namespace tigl
