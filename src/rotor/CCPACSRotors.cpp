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

#include "CCPACSRotors.h"
#include "CTiglError.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSRotors::CCPACSRotors(CCPACSConfiguration* config)
    : configuration(config)
{
    Cleanup();
}

// Destructor
CCPACSRotors::~CCPACSRotors(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSRotors::Invalidate(void)
{
    for (int i = 1; i <= GetRotorCount(); i++) {
        GetRotor(i).Invalidate();
    }
}

// Cleanup routine
void CCPACSRotors::Cleanup(void)
{
    for (CCPACSRotorContainer::size_type i = 0; i < rotors.size(); i++) {
        delete rotors[i];
    }
    rotors.clear();
}

// Read CPACS rotors element
void CCPACSRotors::ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID, const std::string rotorsLibraryName, const std::string rotorElementName, const std::string rotorProfilesLibraryPath, const std::string rotorProfileElementName)
{
    Cleanup();
    std::string rotorXPathPrt;
    char* tmpString = NULL;

    if (tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSRotors::ReadCPACS", TIGL_XML_ERROR);
    }

    rotorXPathPrt = std::string(tmpString) + "[@uID=\"" + std::string(configurationUID) + "\"]/" + rotorsLibraryName;

    if (tixiCheckElement(tixiHandle, rotorXPathPrt.c_str()) != SUCCESS) {
        return;
    }

    /* Get rotor element count */
    int rotorCount;
    if (tixiGetNamedChildrenCount(tixiHandle, rotorXPathPrt.c_str(), rotorElementName.c_str(), &rotorCount) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSRotors::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all rotors
    for (int i = 1; i <= rotorCount; i++) {
        CCPACSRotor* rotor = new CCPACSRotor(configuration);
        rotors.push_back(rotor);

        std::ostringstream xpath;
        xpath << rotorXPathPrt << "/" << rotorElementName << "[" << i << "]";
        rotor->ReadCPACS(tixiHandle, xpath.str());
    }
}

// Returns the total count of rotors in a configuration
int CCPACSRotors::GetRotorCount(void) const
{
    return (static_cast<int>(rotors.size()));
}

// Returns the rotor for a given index.
CCPACSRotor& CCPACSRotors::GetRotor(int index) const
{
    index --;
    if (index < 0 || index >= GetRotorCount()) {
        throw CTiglError("Error: Invalid index in CCPACSRotors::GetRotor", TIGL_INDEX_ERROR);
    }
    return (*rotors[index]);
}

// Returns the rotor for a given UID.
CCPACSRotor& CCPACSRotors::GetRotor(const std::string& UID) const
{
    return (*rotors[GetRotorIndex(UID)-1]);
}

// Returns the rotor index for a given UID.
int CCPACSRotors::GetRotorIndex(const std::string& UID) const
{
    for (int i=0; i < GetRotorCount(); i++) {
        const std::string tmpUID(rotors[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSRotors::GetRotorIndex", TIGL_UID_ERROR);
    return -1;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotors::GetConfiguration(void) const
{
    return *configuration;
}

} // end namespace tigl
