/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
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

#pragma once

#include "generated/CPACSEnginePositions.h"
#include "generated/CPACSEnginePosition.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSEnginePositions : public generated::CPACSEnginePositions
{
public:
    CCPACSEnginePositions(CTiglUIDManager* uidMgr);

    TIGL_EXPORT size_t GetEnginePositionCount() const;
    TIGL_EXPORT CCPACSEnginePosition& GetEnginePosition(size_t index) const;
    TIGL_EXPORT CCPACSEnginePosition& GetEnginePosition(std::string const uid) const;
};

} //namespace tigl
