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
* @brief  Implementation of rotor blades handling routines.
*/

#include "CCPACSRotorBlades.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CTiglError.h"
#include "CCPACSRotorcraftModel.h"

namespace tigl
{
// Constructor
CCPACSRotorBlades::CCPACSRotorBlades(CCPACSRotorcraftModel* parent)
    : generated::CPACSRotorBlades(parent) {}

// Invalidates internal state
void CCPACSRotorBlades::Invalidate(void)
{
    for (int i = 1; i <= GetRotorBladeCount(); i++) {
        GetRotorBlade(i).Invalidate();
    }
}

// Adds a rotor blade to the  rotor blade container
void CCPACSRotorBlades::AddRotorBlade(CCPACSWing* rotorBlade)
{
    m_rotorBlade.push_back(tigl::unique_ptr<CCPACSWing>(rotorBlade));
}

// Returns the total count of rotor blades of a rotor blade attachment
int CCPACSRotorBlades::GetRotorBladeCount(void) const
{
    return (static_cast<int>(m_rotorBlade.size()));
}

// Returns the rotor blade for a given index
CCPACSWing& CCPACSRotorBlades::GetRotorBlade(int index) const
{
    index --;
    if (index < 0 || index >= GetRotorBladeCount()) {
        throw CTiglError("Error: Invalid index in CCPACSRotorBlades::GetRotorBlade", TIGL_INDEX_ERROR);
    }
    return *m_rotorBlade[index];
}

CCPACSWing& CCPACSRotorBlades::GetRotorBlade(const std::string& uid) const
{
    return *m_rotorBlade[GetRotorBladeIndex(uid) - 1];
}

// Returns the wing index for a given UID.
int CCPACSRotorBlades::GetRotorBladeIndex(const std::string& UID) const
{
    for (int i = 0; i < GetRotorBladeCount(); i++) {
        const std::string tmpUID(m_rotorBlade[i]->GetUID());
        if (tmpUID == UID) {
            return i + 1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSRotorBlades::GetRotorBladeIndex", TIGL_UID_ERROR);
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorBlades::GetConfiguration(void) const
{
    return m_parent->GetConfiguration();
}

} // end namespace tigl
