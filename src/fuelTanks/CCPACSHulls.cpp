/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-04-06 Anton Reiswich <Anton.Reiswich@dlr.de>
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
/**
* @file
* @brief  Implementation of CPACS hulls handling routines.
*/

#include "CCPACSHulls.h"
#include "CCPACSHull.h"
#include "CTiglError.h"

// #include <algorithm>

namespace tigl {

CCPACSHulls::CCPACSHulls(CCPACSGenericFuelTank* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSHulls(parent, uidMgr)
    , enabled(false)
{}

// void CCPACSHulls::RegisterInvalidationCallback(std::function<void()> const& fn){
//     invalidationCallbacks.push_back(fn);
// }

CCPACSHull const& CCPACSHulls::GetHull(std::string const& uid) const
{
    return const_cast<CCPACSHulls&>(*this).GetHull(uid);
}

CCPACSHull& CCPACSHulls::GetHull(std::string const& uid)
{
    auto it = std::find_if(m_hulls.begin(), m_hulls.end(), [&](std::unique_ptr<CCPACSHull> const& v){ return v->GetUID() == uid; });

    if ( it != std::end(m_hulls)) {
        return **it;
    }
    throw CTiglError("Could not find hull with uid" + uid);
}


bool CCPACSHulls::IsEnabled() const
{
    return enabled;
}

void CCPACSHulls::SetEnabled(bool val)
{
    if (enabled != val) {
        for (auto const& invalidator: invalidationCallbacks) {
            invalidator();
        }
        enabled = val;
    }
}

} //namespace tigl
