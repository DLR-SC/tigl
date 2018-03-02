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

#include "generated/CPACSFramesAssembly.h"

namespace tigl
{
class CCPACSFramesAssembly : public generated::CPACSFramesAssembly
{
public:
    TIGL_EXPORT CCPACSFramesAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);
};

} // namespace tigl
