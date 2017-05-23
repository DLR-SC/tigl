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
* @brief  Implementation of CPACS hinges handling routines.
*/

#include "CCPACSRotorHinges.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CTiglError.h"
#include <iostream>
#include <sstream>

namespace tigl
{
CCPACSRotorHinges::CCPACSRotorHinges(CCPACSRotorBladeAttachment* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSRotorHubHinges(parent, uidMgr) {}

// Returns the total count of rotor hinges in a rotor blade attachment
int CCPACSRotorHinges::GetRotorHingeCount() const
{
    return static_cast<int>(m_hinges.size());
}

// Returns the rotor hinge for a given index.
CCPACSRotorHinge& CCPACSRotorHinges::GetRotorHinge(int index) const
{
    index--;
    if (index < 0 || index >= GetRotorHingeCount()) {
        throw CTiglError("Invalid index in CCPACSRotorHinges::GetRotorHinge", TIGL_INDEX_ERROR);
    }
    return *m_hinges[index];
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorHinges::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

// Returns the parent rotor
CCPACSRotor& CCPACSRotorHinges::GetRotor() const
{
    return m_parent->GetRotor();
}

// Returns the parent rotor blade attachment
CCPACSRotorBladeAttachment& CCPACSRotorHinges::GetRotorBladeAttachment() const
{
    return *m_parent;
}

} // end namespace tigl
