/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-11 Philipp Kunze <Philipp.Kunze@dlr.de>
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
* @brief  Implementation of CPACS rotors handling routines.
*/

#include "CCPACSRotors.h"

#include <iostream>
#include <sstream>

#include "CCPACSRotorcraftModel.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSRotors::CCPACSRotors(CCPACSRotorcraftModel* parent)
    : generated::CPACSRotors(parent) {}

// Invalidates internal state
void CCPACSRotors::Invalidate(void)
{
    for (int i = 1; i <= GetRotorCount(); i++) {
        GetRotor(i).Invalidate();
    }
}

// Returns the total count of rotors in a configuration
int CCPACSRotors::GetRotorCount(void) const
{
    return (static_cast<int>(m_rotor.size()));
}

// Returns the rotor for a given index.
CCPACSRotor& CCPACSRotors::GetRotor(int index) const
{
    index --;
    if (index < 0 || index >= GetRotorCount()) {
        throw CTiglError("Error: Invalid index in CCPACSRotors::GetRotor", TIGL_INDEX_ERROR);
    }
    return (*m_rotor[index]);
}

// Returns the rotor for a given UID.
CCPACSRotor& CCPACSRotors::GetRotor(const std::string& UID) const
{
    return (*m_rotor[GetRotorIndex(UID)-1]);
}

// Returns the rotor index for a given UID.
int CCPACSRotors::GetRotorIndex(const std::string& UID) const
{
    for (int i=0; i < GetRotorCount(); i++) {
        const std::string tmpUID(m_rotor[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSRotors::GetRotorIndex", TIGL_UID_ERROR);
    return -1;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotors::GetConfiguration(void) const
{
    return m_parent->GetConfiguration();
}

} // end namespace tigl
