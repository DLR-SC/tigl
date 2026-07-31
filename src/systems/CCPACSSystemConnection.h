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

#include "generated/CPACSSystemConnection.h"
#include "CCPACSComponent.h"

namespace tigl
{

class CCPACSSystemConnection : public generated::CPACSSystemConnection
{
public:
    TIGL_EXPORT CCPACSSystemConnection(CCPACSSystemConnections* parent, CTiglUIDManager* uidMgr);

    /**
     * @brief Returns the source component of this system connection, if available as CCPACSComponent.
     *
     * This convenience getter resolves the source @c componentUID and returns the
     * referenced object only if it exists and is of type @ref CCPACSComponent.
     * If the source does not define a @c componentUID, or if the referenced UID
     * resolves to a different CPACS object type, @c boost::none is returned.
     *
     * @return Referenced source component as @ref CCPACSComponent, or @c boost::none.
     */
    TIGL_EXPORT boost::optional<const CCPACSComponent&> GetSourceComponent() const;

    /**
     * @brief Returns the target component of this system connection, if available as CCPACSComponent.
     *
     * This convenience getter resolves the target @c componentUID and returns the
     * referenced object only if it exists and is of type @ref CCPACSComponent.
     * If the target does not define a @c componentUID, or if the referenced UID
     * resolves to a different CPACS object type, @c boost::none is returned.
     *
     * @return Referenced target component as @ref CCPACSComponent, or @c boost::none.
     */
    TIGL_EXPORT boost::optional<const CCPACSComponent&> GetTargetComponent() const;
};

} // namespace tigl
