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

#include "CCPACSSystemConnection.h"
#include "CTiglUIDManager.h"

namespace tigl
{

CCPACSSystemConnection::CCPACSSystemConnection(CCPACSSystemConnections* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSystemConnection(parent, uidMgr)
{
}

boost::optional<const CCPACSComponent&> CCPACSSystemConnection::GetSourceComponent() const
{
    const auto& componentUID = GetSource().GetComponentUID_choice4();
    if (!componentUID) {
        return boost::none;
    }

    if (!GetUIDManager().IsType<CCPACSComponent>(*componentUID)) {
        return boost::none;
    }

    return GetUIDManager().ResolveObject<CCPACSComponent>(*componentUID);
}

boost::optional<const CCPACSComponent&> CCPACSSystemConnection::GetTargetComponent() const
{
    const auto& componentUID = GetTarget().GetComponentUID_choice4();
    if (!componentUID) {
        return boost::none;
    }

    if (!GetUIDManager().IsType<CCPACSComponent>(*componentUID)) {
        return boost::none;
    }

    return GetUIDManager().ResolveObject<CCPACSComponent>(*componentUID);
}

} //namespace tigl