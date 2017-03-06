/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file 
* @brief  Implementation of CPACS wings handling routines.
*/

#include "CCPACSWings.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include "CCPACSAircraftModel.h"

namespace tigl
{

// Invalidates internal state
void CCPACSWings::Invalidate()
{
    GetProfiles().Invalidate();
    for (int i = 1; i <= GetWingCount(); i++) {
        GetWing(i).Invalidate();
    }
}

CCPACSWings::CCPACSWings(CCPACSRotorcraftModel* parent)
    : generated::CPACSWings(parent) {
    throw std::logic_error("Instantiating CCPACSWings with CPACSRotorcraftModel as parent is not implemented");
}

CCPACSWings::CCPACSWings(CCPACSAircraftModel* parent)
    : generated::CPACSWings(parent) {}

bool CCPACSWings::HasProfile(std::string uid) const
{
    return GetProfiles().HasProfile(uid);
}

// Returns the total count of wing profiles in this configuration
int CCPACSWings::GetProfileCount() const
{
    return GetProfiles().GetProfileCount();
}

CCPACSWingProfiles& CCPACSWings::GetProfiles()
{
    return static_cast<CCPACSAircraftModel*>(m_parent)->GetConfiguration().GetWingProfiles();
}

const CCPACSWingProfiles& CCPACSWings::GetProfiles() const {
    return static_cast<CCPACSAircraftModel*>(m_parent)->GetConfiguration().GetWingProfiles();
}


// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWings::GetProfile(std::string uid) const
{
    return GetProfiles().GetProfile(uid);
}

// Returns the wing profile for a given index.
CCPACSWingProfile& CCPACSWings::GetProfile(int index) const
{
    return GetProfiles().GetProfile(index);
}

// Returns the total count of wings in a configuration
int CCPACSWings::GetWingCount() const
{
    return static_cast<int>(m_wing.size());
}

// Returns the count of wings in a configuration with the property isRotorBlade set to true
int CCPACSWings::GetRotorBladeCount() const
{
    int nRotorBlades = 0;
    for (int i = 1; i <= GetWingCount(); i++) {
        if (GetWing(i).IsRotorBlade()) {
            nRotorBlades++;
        }
    }
    return nRotorBlades;
}

// Returns the wing for a given index.
CCPACSWing& CCPACSWings::GetWing(int index) const
{
    index --;
    if (index < 0 || index >= GetWingCount()) {
        throw CTiglError("Error: Invalid index in CCPACSWings::GetWing", TIGL_INDEX_ERROR);
    }
    return *m_wing[index];
}

// Returns the wing for a given UID.
CCPACSWing& CCPACSWings::GetWing(const std::string& UID) const
{
    return *m_wing[GetWingIndex(UID)-1];
}

// Returns the wing index for a given UID.
int CCPACSWings::GetWingIndex(const std::string& UID) const
{
    for (int i=0; i < GetWingCount(); i++) {
        const std::string tmpUID(m_wing[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSWings::GetWingIndex", TIGL_UID_ERROR);
}

bool CCPACSWings::HasWing(const std::string & uid) const
{
    for (std::size_t i = 0; i < m_wing.size(); i++)
        if (m_wing[i]->GetUID() == uid)
            return true;
    return false;
}

void CCPACSWings::AddWing(CCPACSWing* wing)
{
    // Check whether the same wing already exists if yes remove it before adding the new one
    for (std::vector<unique_ptr<CCPACSWing>>::const_iterator it = m_wing.begin(); it != m_wing.end(); ++it) {
        if ((*it)->GetUID() == wing->GetUID()) {
            m_wing.erase(it);
            break;
        }
    }

    // Add the new wing to the wing list
    m_wing.emplace_back(wing);
}

} // end namespace tigl
