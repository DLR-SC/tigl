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

namespace tigl
{

CCPACSHulls::CCPACSHulls(CCPACSGenericFuelTank* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSHulls(parent, uidMgr)
{
}

int CCPACSHulls::GetHullsCount() const
{
    return static_cast<int>(m_hulls.size());
}

CCPACSHull const& CCPACSHulls::GetHull(std::string const& uID) const
{
    return const_cast<CCPACSHulls&>(*this).GetHull(uID);
}

CCPACSHull& CCPACSHulls::GetHull(std::string const& uID)
{
    auto it = std::find_if(m_hulls.begin(), m_hulls.end(),
                           [&](std::unique_ptr<CCPACSHull> const& v) { return v->GetUID() == uID; });

    if (it != std::end(m_hulls)) {
        return **it;
    }
    throw CTiglError("Could not find hull with uid" + uID);
}

CCPACSHull& CCPACSHulls::GetHull(int index) const
{
    index--;
    if (index < 0 || index >= GetHullsCount()) {
        throw CTiglError("Invalid index in CCPACSHulls::GetHull", TIGL_INDEX_ERROR);
    }
    return *m_hulls[index];
}

int CCPACSHulls::GetHullIndex(const std::string& uID) const
{
    for (int i = 0; i < GetHullsCount(); i++) {
        const std::string tmpUID(m_hulls[i]->GetUID());
        if (tmpUID == uID) {
            return i + 1;
        }
    }
    throw CTiglError("Invalid UID in CCPACSHulls::GetHullIndex", TIGL_UID_ERROR);
}

} //namespace tigl
