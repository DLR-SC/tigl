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

#ifndef CCPACSROTORHUB_H
#define CCPACSROTORHUB_H

#include <string>

#include "generated/CPACSRotorHub.h"
#include "CCPACSRotorBladeAttachments.h"


namespace tigl
{

enum TiglRotorHubType
{
    TIGLROTORHUB_UNDEFINED,
    TIGLROTORHUB_SEMI_RIGID,
    TIGLROTORHUB_RIGID,
    TIGLROTORHUB_ARTICULATED,
    TIGLROTORHUB_HINGELESS
};

class CCPACSConfiguration;
class CCPACSRotor;
class CTiglAttachedRotorBlade;

class CCPACSRotorHub : public generated::CPACSRotorHub
{

public:
    // Constructor
    TIGL_EXPORT CCPACSRotorHub(CCPACSRotor* parent);

    // Returns the type of the rotor hub
    TIGL_EXPORT TiglRotorHubType GetType(void) const;

    // Returns the rotor blade attachment count
    TIGL_EXPORT int GetRotorBladeAttachmentCount(void) const;

    // Returns the rotor blade attachment for a given index
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(int index) const;

    // Returns the rotor blade count
    TIGL_EXPORT int GetRotorBladeCount(void) const;

    // Returns the rotor blade for a given index
    TIGL_EXPORT CTiglAttachedRotorBlade& GetRotorBlade(int index) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor(void) const;
};

} // end namespace tigl

#endif // CCPACSROTORHUB_H
