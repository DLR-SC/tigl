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
* @brief  Implementation of CPACS blade attachments handling routines.
*/

#ifndef CCPACSROTORBLADEATTACHMENTS_H
#define CCPACSROTORBLADEATTACHMENTS_H

#include "generated/CPACSRotorBladeAttachments.h"

namespace tigl
{

class CCPACSConfiguration;
class CCPACSRotor;

class CCPACSRotorBladeAttachments : public generated::CPACSRotorBladeAttachments
{
public:
    // Constructor
    TIGL_EXPORT CCPACSRotorBladeAttachments(CCPACSRotorHub* parent);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Returns the total count of rotor blade attachments in a rotor hub
    TIGL_EXPORT int GetRotorBladeAttachmentCount(void) const;

    // Returns the rotor blade attachment for a given index.
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(int index) const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor(void) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;
};

} // end namespace tigl

#endif // CCPACSROTORBLADEATTACHMENTS_H
