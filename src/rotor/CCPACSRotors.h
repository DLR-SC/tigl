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

#ifndef CCPACSROTORS_H
#define CCPACSROTORS_H

#include "generated/CPACSRotors.h"
#include "CCPACSRotor.h"

namespace tigl
{

class CCPACSConfiguration;

class CCPACSRotors : public generated::CPACSRotors
{
public:
    // Constructor
    TIGL_EXPORT CCPACSRotors(CCPACSRotorcraftModel* parent);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Returns the total count of rotors in a configuration
    TIGL_EXPORT int GetRotorCount(void) const;

    // Returns the rotor for a given index.
    TIGL_EXPORT CCPACSRotor& GetRotor(int index) const;

    // Returns the rotor for a given UID.
    TIGL_EXPORT CCPACSRotor& GetRotor(const std::string& UID) const;

    // Returns the rotor index for a given UID.
    TIGL_EXPORT int GetRotorIndex(const std::string& UID) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;
};
} // end namespace tigl

#endif // CCPACSROTORS_H
