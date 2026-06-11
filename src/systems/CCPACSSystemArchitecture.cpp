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

#include "CCPACSSystemArchitecture.h"
#include "CCPACSSystemConnection.h"
#include "CTiglUIDManager.h"

#include <set>

namespace tigl
{

CCPACSSystemArchitecture::CCPACSSystemArchitecture(CCPACSSystemArchitectures* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSystemArchitecture(parent, uidMgr)
{
}

std::vector<const CCPACSComponent*> CCPACSSystemArchitecture::GetGenericSystemComponents() const
{
    std::vector<const CCPACSComponent*> result;
    std::set<std::string> seenUids;

    const auto& connections = GetConnections();
    if (!connections) {
        return result;
    }

    auto addIfNew = [&](const boost::optional<const CCPACSComponent&>& component) {
        if (!component) {
            return;
        }

        const std::string uid = component->GetDefaultedUID();
        if (seenUids.insert(uid).second) {
            result.push_back(&(*component));
        }
    };

    for (size_t i = 1; i <= connections->GetConnectionCount(); ++i) {
        const auto& connection = connections->GetConnection(i);
        addIfNew(connection.GetSourceComponent());
        addIfNew(connection.GetTargetComponent());
    }

    return result;
}

} //namespace tigl