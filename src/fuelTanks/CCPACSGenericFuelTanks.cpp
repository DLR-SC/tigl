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
* @brief  Implementation of CPACS genericFuelTanks handling routines.
*/

#include "CCPACSGenericFuelTanks.h"
#include "CCPACSGenericFuelTank.h"
#include "CTiglError.h"

namespace tigl {

CCPACSGenericFuelTanks::CCPACSGenericFuelTanks(generated::CPACSFuselageFuelTanks* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGenericFuelTanks(parent, uidMgr)
{}

int CCPACSGenericFuelTanks::GetGenericFuelTanksCount() const
{
    return static_cast<int>(m_genericFuelTanks.size());
}

CCPACSGenericFuelTank const& CCPACSGenericFuelTanks::GetGenericFuelTank(std::string const& uID) const
{
    return const_cast<CCPACSGenericFuelTanks&>(*this).GetGenericFuelTank(uID);
}

CCPACSGenericFuelTank& CCPACSGenericFuelTanks::GetGenericFuelTank(std::string const& uID)
{
    auto it = std::find_if(m_genericFuelTanks.begin(), m_genericFuelTanks.end(), [&](std::unique_ptr<CCPACSGenericFuelTank> const& v){ return v->GetUID() == uID; });

    if ( it != std::end(m_genericFuelTanks)) {
        return **it;
    }
    throw CTiglError("Could not find genericFuelTank with uid" + uID);
}

CCPACSGenericFuelTank& CCPACSGenericFuelTanks::GetGenericFuelTank(int index) const
{
    index--;
    if (index < 0 || index >= GetGenericFuelTanksCount()) {
        throw CTiglError("Invalid index in CCPACSGenericFuelTanks::GetGenericFuelTank", TIGL_INDEX_ERROR);
    }
    return *m_genericFuelTanks[index];
}

int CCPACSGenericFuelTanks::GetGenericFuelTankIndex(const std::string& uID) const
{
    for (int i=0; i < GetGenericFuelTanksCount(); i++) {
        const std::string tmpUID(m_genericFuelTanks[i]->GetUID());
        if (tmpUID == uID) {
            return i+1;
        }
    }
    throw CTiglError("Invalid UID in CCPACSGenericFuelTanks::GetGenericFuelTankIndex", TIGL_UID_ERROR);
}

} //namespace tigl
