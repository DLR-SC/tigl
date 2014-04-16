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

#include "CCPACSRotorHub.h"
#include "CCPACSRotor.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSRotorHub::CCPACSRotorHub(CCPACSRotor* rotor)
    : rotor(rotor)
    , rotorBladeAttachments(rotor)
{
    Cleanup();
}

// Virtual destructor
CCPACSRotorHub::~CCPACSRotorHub(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSRotorHub::Cleanup(void)
{
    uID = "";
    name = "";
    description = "";
    type = TIGLROTORHUB_UNDEFINED;
    //rotorBladeAttachments.Cleanup();

    Invalidate();
}

// Update internal rotor hub data
void CCPACSRotorHub::Update(void)
{
    if (!invalidated) {
        return;
    }

    invalidated = false;
}

// Invalidates internal state
void CCPACSRotorHub::Invalidate(void)
{
    invalidated = true;
}

// Read CPACS rotorHub elements
void CCPACSRotorHub::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorHubXPath)
{
    Cleanup();

    std::string tempString;

    // Get attribute "uID"
    char* ptrUID  = NULL;
    if (tixiGetTextAttribute(tixiHandle, rotorHubXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        uID = ptrUID;
    }

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = rotorHubXPath + "/name";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get subelement "description"
    char* ptrDescription = NULL;
    tempString    = rotorHubXPath + "/description";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrDescription) == SUCCESS) {
        description = ptrDescription;
    }

    // Get subelement "type"
    char* ptrType = NULL;
    tempString    = rotorHubXPath + "/type";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrType) == SUCCESS) {
        std::string strType(ptrType);
        if (strType == "semiRigid") {
            type = TIGLROTORHUB_SEMI_RIGID;
        }
        else if (strType == "rigid") {
            type = TIGLROTORHUB_RIGID;
        }
        else if (strType == "articulated") {
            type = TIGLROTORHUB_ARTICULATED;
        }
        else if (strType == "hingeless") {
            type = TIGLROTORHUB_HINGELESS;
        }
        else {
            throw CTiglError("Error: XML error while reading <type> in CCPACSRotorHub::ReadCPACS: illegal value", TIGL_XML_ERROR);
        }
    }
    else {
        type = TIGLROTORHUB_UNDEFINED;
    }

    // Get subelement "rotorBladeAttachments"
    rotorBladeAttachments.ReadCPACS(tixiHandle, rotorHubXPath + "/rotorBladeAttachments", "rotorBladeAttachment");

    Update();
}

// Returns the UID of the rotor hub
std::string CCPACSRotorHub::GetUID(void) const
{
    return uID;
}

// Returns the name of the rotor hub
const std::string& CCPACSRotorHub::GetName(void) const
{
    return name;
}

// Returns the description of the rotor hub
const std::string& CCPACSRotorHub::GetDescription(void) const
{
    return description;
}

// Returns the type of the rotor hub
const TiglRotorHubType& CCPACSRotorHub::GetType(void) const
{
    return type;
}

// Returns the rotor blade attachment count
int CCPACSRotorHub::GetRotorBladeAttachmentCount(void) const
{
    return rotorBladeAttachments.GetRotorBladeAttachmentCount();
}

// Returns the rotor blade attachment for a given index
CCPACSRotorBladeAttachment& CCPACSRotorHub::GetRotorBladeAttachment(int index) const
{
    return rotorBladeAttachments.GetRotorBladeAttachment(index);
}

// Returns the rotor blade count
int CCPACSRotorHub::GetRotorBladeCount(void) const
{
    int rotorBladeCount = 0;
    for (int i=1; i<=GetRotorBladeAttachmentCount(); i++) {
        rotorBladeCount += GetRotorBladeAttachment(i).GetRotorBladeCount();
    }
    return rotorBladeCount;
}

// Returns the rotor blade for a given index
CCPACSRotorBlade& CCPACSRotorHub::GetRotorBlade(int index) const
{
    int rotorBladeIndex = index;
    int rotorBladeAttachmentIndex = 1;
    while (rotorBladeIndex > GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetRotorBladeCount()) {
        rotorBladeIndex -= GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetRotorBladeCount();
        rotorBladeAttachmentIndex++;
    }
    return GetRotorBladeAttachment(rotorBladeAttachmentIndex).GetRotorBlade(index);
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorHub::GetConfiguration(void) const
{
    return rotor->GetConfiguration();
}

// Returns the parent rotor
CCPACSRotor& CCPACSRotorHub::GetRotor(void) const
{
    return *rotor;
}

} // end namespace tigl
