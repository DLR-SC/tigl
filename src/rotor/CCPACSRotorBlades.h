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

#ifndef CCPACSROTORBLADES_H
#define CCPACSROTORBLADES_H

#include <vector>

#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSRotorBlade.h"

namespace tigl
{

class CCPACSConfiguration;
class CCPACSRotor;
class CCPACSRotorBladeAttachment;

class CCPACSRotorBlades
{
private:
    // Typedef for a CCPACSRotorBlade container to store the rotor blades of a rotor blade attachment.
    typedef std::vector<CCPACSRotorBlade*> CCPACSRotorBladeContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSRotorBlades(CCPACSRotorBladeAttachment* rotorBladeAttachment);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSRotorBlades(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Adds a rotor blade to the  rotor blade container
    TIGL_EXPORT void AddRotorBlade(CCPACSRotorBlade* rotorBlade);

    // Returns the total count of rotor blades of a rotor blade attachment
    TIGL_EXPORT int GetRotorBladeCount(void) const;

    // Returns the rotor blade for a given index.
    TIGL_EXPORT CCPACSRotorBlade& GetRotorBlade(int index) const;

    // Returns the parent rotor blade attachment
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(void) const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor(void) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSRotorBlades(const CCPACSRotorBlades&);

    // Assignment operator
    void operator=(const CCPACSRotorBlades&);

private:
    CCPACSRotorBladeContainer   rotorBlades;            /**< RotorBlade elements */
    CCPACSRotorBladeAttachment* rotorBladeAttachment;   /**< Pointer to parent rotor blade attachment */

};

} // end namespace tigl

#endif // CCPACSROTORBLADES_H
