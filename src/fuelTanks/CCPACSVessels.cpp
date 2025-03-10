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
* @brief  Implementation of CPACS vessels handling routines.
*/

#include "CCPACSVessels.h"
#include "CCPACSVessel.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSVessels::CCPACSVessels(CCPACSFuelTank* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSVessels(parent, uidMgr)
{
}

int CCPACSVessels::GetVesselsCount() const
{
    return static_cast<int>(m_vessels.size());
}

CCPACSVessel const& CCPACSVessels::GetVessel(std::string const& uID) const
{
    return const_cast<CCPACSVessels&>(*this).GetVessel(uID);
}

CCPACSVessel& CCPACSVessels::GetVessel(std::string const& uID)
{
    auto it = std::find_if(m_vessels.begin(), m_vessels.end(),
                           [&](std::unique_ptr<CCPACSVessel> const& v) { return v->GetUID() == uID; });

    if (it != std::end(m_vessels)) {
        return **it;
    }
    throw CTiglError("Could not find vessel with uid" + uID);
}

CCPACSVessel& CCPACSVessels::GetVessel(int index) const
{
    index--;
    if (index < 0 || index >= GetVesselsCount()) {
        throw CTiglError("Invalid index in CCPACSVessels::GetVessel", TIGL_INDEX_ERROR);
    }
    return *m_vessels[index];
}

int CCPACSVessels::GetVesselIndex(const std::string& uID) const
{
    for (int i = 0; i < GetVesselsCount(); i++) {
        const std::string tmpUID(m_vessels[i]->GetUID());
        if (tmpUID == uID) {
            return i + 1;
        }
    }
    throw CTiglError("Invalid UID in CCPACSVessels::GetVesselIndex", TIGL_UID_ERROR);
}

} //namespace tigl
