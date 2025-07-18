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
* @brief  Implementation of CPACS rotor hub handling routines.
*/

#include "CCPACSRotorHub.h"
#include "CCPACSRotor.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSRotorHub::CCPACSRotorHub(CCPACSRotor* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSRotorHub(parent, uidMgr) {}

// Returns the type of the rotor hub
TiglRotorHubType CCPACSRotorHub::GetDefaultedType() const
{
    if (!m_type)
        return TIGLROTORHUB_UNDEFINED;
    return *m_type;
}

// Returns the rotor blade attachment count
size_t CCPACSRotorHub::GetRotorBladeAttachmentCount() const
{
    return m_rotorBladeAttachments.GetRotorBladeAttachmentCount();
}

// Returns the rotor blade attachment for a given index
const CCPACSRotorBladeAttachment& CCPACSRotorHub::GetRotorBladeAttachment(size_t index) const
{
    return m_rotorBladeAttachments.GetRotorBladeAttachment(index);
}

CCPACSRotorBladeAttachment& CCPACSRotorHub::GetRotorBladeAttachment(size_t index)
{
    return m_rotorBladeAttachments.GetRotorBladeAttachment(index);
}

// Returns the rotor blade count
size_t CCPACSRotorHub::GetRotorBladeCount() const
{
    int rotorBladeCount = 0;
    for (int i=1; i<=GetRotorBladeAttachmentCount(); i++) {
        rotorBladeCount += GetRotorBladeAttachment(i).GetNumberOfBlades();
    }
    return rotorBladeCount;
}

// Returns the rotor blade for a given index
const CTiglAttachedRotorBlade& CCPACSRotorHub::GetRotorBlade(size_t index) const
{
    int rotorBladeIndex = index;
    int rotorBladeAttachmentIndex = 1;
    while (rotorBladeIndex > GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetNumberOfBlades()) {
        rotorBladeIndex -= GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetNumberOfBlades();
        rotorBladeAttachmentIndex++;
    }
    return GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetAttachedRotorBlade(rotorBladeIndex);
}

CTiglAttachedRotorBlade& CCPACSRotorHub::GetRotorBlade(size_t index)
{
    int rotorBladeIndex = index;
    int rotorBladeAttachmentIndex = 1;
    while (rotorBladeIndex > GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetNumberOfBlades()) {
        rotorBladeIndex -= GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetNumberOfBlades();
        rotorBladeAttachmentIndex++;
    }
    return GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetAttachedRotorBlade(rotorBladeIndex);
}

// Returns the parent rotor
const CCPACSRotor& CCPACSRotorHub::GetRotor() const
{
    return *m_parent;
}

CCPACSRotor& CCPACSRotorHub::GetRotor()
{
    return *m_parent;
}

// Returns the parent configuration
const CCPACSConfiguration& CCPACSRotorHub::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

CCPACSConfiguration& CCPACSRotorHub::GetConfiguration()
{
    return m_parent->GetConfiguration();
}

} // end namespace tigl
