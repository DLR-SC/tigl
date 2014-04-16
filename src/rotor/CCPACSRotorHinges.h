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

#include "tixi.h"
#include "CCPACSRotorHinge.h"
#include <vector>

namespace tigl
{

class CCPACSConfiguration;
class CCPACSRotor;
class CCPACSRotorBladeAttachment;

class CCPACSRotorHinges
{
private:
    // Typedef for a CCPACSRotorHinge container to store the rotor hinges of a configuration.
    typedef std::vector<CCPACSRotorHinge*> CCPACSRotorHingeContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSRotorHinges(CCPACSRotorBladeAttachment* rotorBladeAttachment);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSRotorHinges(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS rotorHinges elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string rotorHingesLibraryXPath, const std::string rotorHingeElementName="hinge");

    // Returns the total count of rotor hinges in a rotor blade attachment
    TIGL_EXPORT int GetRotorHingeCount(void) const;

    // Returns the rotor hinge for a given index.
    TIGL_EXPORT CCPACSRotorHinge& GetRotorHinge(int index) const;

    // Returns the rotor hinge for a given UID.
    TIGL_EXPORT CCPACSRotorHinge& GetRotorHinge(const std::string& UID) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor(void) const;

    // Returns the parent rotor blade attachment
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSRotorHinges(const CCPACSRotorHinges&);

    // Assignment operator
    void operator=(const CCPACSRotorHinges&);

private:
    CCPACSRotorHingeContainer   rotorHinges;            /**< RotorHinge elements */
    CCPACSRotorBladeAttachment* rotorBladeAttachment;   /**< Pointer to parent rotor blade attachment */

};

} // end namespace tigl

#endif // CCPACSROTORHINGES_H
