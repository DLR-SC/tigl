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

#include "CCPACSRotorHinges.h"
#include "CTiglError.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSRotorHinges::CCPACSRotorHinges(CCPACSConfiguration* config)
    : configuration(config)
{
    Cleanup();
}

// Virtual Destructor
CCPACSRotorHinges::~CCPACSRotorHinges(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSRotorHinges::Cleanup(void)
{
    for (CCPACSRotorHingeContainer::size_type i = 0; i < rotorHinges.size(); i++) {
        delete rotorHinges[i];
    }
    rotorHinges.clear();
}

// Invalidates internal state
void CCPACSRotorHinges::Invalidate(void)
{
    for (int i = 1; i <= GetRotorHingeCount(); i++) {
        GetRotorHinge(i).Invalidate();
    }
}

// Read CPACS rotorHinges elements
void CCPACSRotorHinges::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string rotorHingesXPath, const std::string rotorHingeElementName)
{
    Cleanup();

    /* Get rotorHinge element count */
    int elementCount;
    if (tixiGetNamedChildrenCount(tixiHandle, rotorHingesXPath.c_str(), rotorHingeElementName.c_str(), &elementCount) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSRotorHinges::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all rotorHinge elements
    for (int i = 1; i <= elementCount; i++) {
        CCPACSRotorHinge* rotorHinge = new CCPACSRotorHinge(configuration);
        rotorHinges.push_back(rotorHinge);

        std::ostringstream xpath;
        xpath << rotorHingesXPath << "/" << rotorHingeElementName << "[" << i << "]";
        rotorHinge->ReadCPACS(tixiHandle, xpath.str());
    }
}

// Returns the total count of rotor hinges in a rotor blade attachment
int CCPACSRotorHinges::GetRotorHingeCount(void) const
{
    return static_cast<int>(rotorHinges.size());
}

// Returns the rotor hinge for a given index.
CCPACSRotorHinge& CCPACSRotorHinges::GetRotorHinge(int index) const
{
    index--;
    if (index < 0 || index >= GetRotorHingeCount()) {
        throw CTiglError("Error: Invalid index in CCPACSRotorHinges::GetRotorHinge", TIGL_INDEX_ERROR);
    }
    return (*rotorHinges[index]);
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorHinges::GetConfiguration(void) const
{
    return *configuration;
}

} // end namespace tigl
