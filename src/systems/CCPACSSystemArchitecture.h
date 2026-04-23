/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-13 Marko Alder <marko.alder@dlr.de>
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

#include "generated/CPACSSystemArchitecture.h"
#include "CCPACSComponent.h"

namespace tigl
{

class CCPACSSystemArchitecture : public generated::CPACSSystemArchitecture
{
public:
    TIGL_EXPORT CCPACSSystemArchitecture(CCPACSSystemArchitectures* parent, CTiglUIDManager* uidMgr);

    /**
     * @brief Returns all unique referenced components used in the system architecture connections.
     *
     * This convenience getter iterates over all connections of the system architecture
     * and collects all referenced source and target objects that resolve to
     * @ref CCPACSComponent. References to other CPACS object types, such as fuselages
     * or external elements, are ignored.
     *
     * The returned list is unique with respect to the component UID and preserves
     * the order of first occurrence in the connection list.
     *
     * @return Unique list of referenced @ref CCPACSComponent objects.
     */
    TIGL_EXPORT std::vector<const CCPACSComponent*> GetGenericSystemComponents() const;
};

} // namespace tigl
