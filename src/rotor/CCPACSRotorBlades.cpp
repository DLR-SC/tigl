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

namespace tigl
{

// Constructor
CCPACSRotorBlades::CCPACSRotorBlades(CCPACSRotorBladeAttachment* rotorBladeAttachment)
    : rotorBladeAttachment(rotorBladeAttachment)
{
    Cleanup();
}

// Destructor
CCPACSRotorBlades::~CCPACSRotorBlades(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSRotorBlades::Invalidate(void)
{
    for (int i = 1; i <= GetRotorBladeCount(); i++) {
        GetRotorBlade(i).Invalidate();
    }
}

// Cleanup routine
void CCPACSRotorBlades::Cleanup(void)
{
    for (CCPACSRotorBladeContainer::size_type i = 0; i < rotorBlades.size(); i++) {
        delete rotorBlades[i];
    }
    rotorBlades.clear();
}

// Adds a rotor blade to the  rotor blade container
void CCPACSRotorBlades::AddRotorBlade(CCPACSRotorBlade* rotorBlade)
{
    rotorBlades.push_back(rotorBlade);
}

// Returns the total count of rotor blades of a rotor blade attachment
int CCPACSRotorBlades::GetRotorBladeCount(void) const
{
    return (static_cast<int>(rotorBlades.size()));
}

// Returns the rotor blade for a given index
CCPACSRotorBlade& CCPACSRotorBlades::GetRotorBlade(int index) const
{
    index --;
    if (index < 0 || index >= GetRotorBladeCount()) {
        throw CTiglError("Error: Invalid index in CCPACSRotorBlades::GetRotorBlade", TIGL_INDEX_ERROR);
    }
    return (*rotorBlades[index]);
}

// Returns the parent rotor blade attachment
CCPACSRotorBladeAttachment& CCPACSRotorBlades::GetRotorBladeAttachment(void) const
{
    return *rotorBladeAttachment;
}

// Returns the parent rotor
CCPACSRotor& CCPACSRotorBlades::GetRotor(void) const
{
    return rotorBladeAttachment->GetRotor();
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorBlades::GetConfiguration(void) const
{
    return rotorBladeAttachment->GetConfiguration();
}

} // end namespace tigl
