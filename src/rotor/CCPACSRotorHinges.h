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

#ifndef CCPACSROTORHINGES_H
#define CCPACSROTORHINGES_H

#include "generated/CPACSRotorHubHinges.h"
#include "CCPACSRotorHinge.h"

namespace tigl
{

class CCPACSConfiguration;
class CCPACSRotor;
class CCPACSRotorBladeAttachment;

class CCPACSRotorHinges : public generated::CPACSRotorHubHinges
{
public:
    TIGL_EXPORT CCPACSRotorHinges(CCPACSRotorBladeAttachment* parent, CTiglUIDManager* uidMgr);

    // Returns the total count of rotor hinges in a rotor blade attachment
    TIGL_EXPORT int GetRotorHingeCount() const;

    // Returns the rotor hinge for a given index.
    TIGL_EXPORT CCPACSRotorHinge& GetRotorHinge(int index) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor() const;

    // Returns the parent rotor blade attachment
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment() const;
};

} // end namespace tigl

#endif // CCPACSROTORHINGES_H
