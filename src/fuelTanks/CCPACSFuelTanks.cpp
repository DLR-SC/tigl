/*
* Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
*
* Created: 2025-05-09 Marko Alder <Marko.Alder@dlr.de>
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
* @brief  Implementation of CPACS fuelTanks handling routines.
*/

#include "CCPACSFuelTanks.h"
#include "CCPACSFuelTank.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSFuelTanks::CCPACSFuelTanks(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuelTanks(parent, uidMgr)
{
}

int CCPACSFuelTanks::GetFuelTanksCount() const
{
    return static_cast<int>(m_fuelTanks.size());
}

CCPACSFuelTank const& CCPACSFuelTanks::GetFuelTank(std::string const& uID) const
{
    return const_cast<CCPACSFuelTanks&>(*this).GetFuelTank(uID);
}

CCPACSFuelTank& CCPACSFuelTanks::GetFuelTank(std::string const& uID)
{
    auto it = std::find_if(m_fuelTanks.begin(), m_fuelTanks.end(),
                           [&](std::unique_ptr<CCPACSFuelTank> const& t) { return t->GetUID() == uID; });

    if (it != std::end(m_fuelTanks)) {
        return **it;
    }
    throw CTiglError("Could not find fuelTank with uid " + uID, TIGL_UID_ERROR);
}

CCPACSFuelTank const& CCPACSFuelTanks::GetFuelTank(int index) const
{
    return const_cast<CCPACSFuelTanks&>(*this).GetFuelTank(index);
}

CCPACSFuelTank& CCPACSFuelTanks::GetFuelTank(int index)
{
    index--;
    if (index < 0 || index >= GetFuelTanksCount()) {
        throw CTiglError("Invalid index in CCPACSFuelTanks::GetFuelTank", TIGL_INDEX_ERROR);
    }
    return *m_fuelTanks[index];
}

int CCPACSFuelTanks::GetFuelTankIndex(const std::string& uID) const
{
    for (int i = 0; i < GetFuelTanksCount(); i++) {
        const std::string tmpUID(m_fuelTanks[i]->GetUID());
        if (tmpUID == uID) {
            return i + 1;
        }
    }
    throw CTiglError("Could not find fuelTank with uid " + uID, TIGL_UID_ERROR);
}

} //namespace tigl
