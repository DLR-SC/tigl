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
#include "TiglRotorHubType.h"

namespace tigl
{
class CCPACSConfiguration;
class CCPACSRotor;
class CTiglAttachedRotorBlade;

class CCPACSRotorHub : public generated::CPACSRotorHub
{
public:
    // Constructor
    TIGL_EXPORT CCPACSRotorHub(CCPACSRotor* parent, CTiglUIDManager* uidMgr);

    // Returns the type of the rotor hub
    TIGL_EXPORT TiglRotorHubType GetDefaultedType() const;

    // Returns the rotor blade attachment count
    TIGL_EXPORT size_t GetRotorBladeAttachmentCount() const;

    // Returns the rotor blade attachment for a given index
    TIGL_EXPORT const CCPACSRotorBladeAttachment& GetRotorBladeAttachment(size_t index) const;
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(size_t index);

    // Returns the rotor blade count
    TIGL_EXPORT size_t GetRotorBladeCount() const;

    // Returns the rotor blade for a given index
    TIGL_EXPORT const CTiglAttachedRotorBlade& GetRotorBlade(size_t index) const;
    TIGL_EXPORT CTiglAttachedRotorBlade& GetRotorBlade(size_t index);

    // Returns the parent rotor
    TIGL_EXPORT const CCPACSRotor& GetRotor() const;
    TIGL_EXPORT CCPACSRotor& GetRotor();

    // Returns the parent configuration
    TIGL_EXPORT const CCPACSConfiguration& GetConfiguration() const;
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration();
};

} // end namespace tigl

#endif // CCPACSROTORHUB_H
