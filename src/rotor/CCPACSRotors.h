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

#include "tixi.h"
#include "CCPACSRotor.h"
#include <vector>

namespace tigl
{

class CCPACSConfiguration;

class CCPACSRotors
{
private:
    // Typedef for a CCPACSRotor container to store the rotors of a configuration.
    typedef std::vector<CCPACSRotor*> CCPACSRotorContainer;

public:
    // Constructor
    CCPACSRotors(CCPACSConfiguration* config);

    // Virtual Destructor
    virtual ~CCPACSRotors(void);

    // Invalidates internal state
    void Invalidate(void);

    // Read CPACS rotors elements
    void ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID, const std::string rotorsLibraryName="rotors", const std::string rotorElementName="rotor", const std::string rotorProfilesLibraryPath="/cpacs/vehicles/profiles/rotorAirfoils", const std::string rotorProfileElementName="rotorAirfoil");

    // Returns the total count of rotors in a configuration
    int GetRotorCount(void) const;

    // Returns the rotor for a given index.
    CCPACSRotor& GetRotor(int index) const;

    // Returns the rotor for a given UID.
    CCPACSRotor& GetRotor(const std::string& UID) const;

    // Returns the parent configuration
    CCPACSConfiguration& GetConfiguration(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSRotors(const CCPACSRotors&);

    // Assignment operator
    void operator=(const CCPACSRotors&);

private:
    //TODO: *CCPACSRotorBlades rotorBlades;     /**< pointer to the collection of rotor blades */
    CCPACSRotorContainer rotors;        /**< Rotor elements */
    CCPACSConfiguration* configuration; /**< Pointer to parent configuration */

};

} // end namespace tigl

#endif // CCPACSROTORS_H
