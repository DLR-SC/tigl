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

#pragma once

#include "generated/CPACSCrossBeamAssemblyPosition.h"

namespace tigl
{
class CCPACSFrame;

class CCPACSCrossBeamAssemblyPosition : public generated::CPACSCrossBeamAssemblyPosition
{
public:
    TIGL_EXPORT CCPACSCrossBeamAssemblyPosition(CCPACSCargoCrossBeamsAssembly* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate();

private:
    bool invalidated = true;
};

} // namespace tigl
