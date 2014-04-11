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
* @brief  Implementation of CPACS rotor blade attachment handling routines.
*/

#include "CCPACSRotorBladeAttachment.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSRotorBladeAttachment::CCPACSRotorBladeAttachment(CCPACSConfiguration* config)
    : configuration(config)
    , hinges(config)
    , azimuthAngles()
//TODO:    , rotorBlades(config)
{
    Cleanup();
}

// Destructor
CCPACSRotorBladeAttachment::~CCPACSRotorBladeAttachment(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSRotorBladeAttachment::Cleanup(void)
{
    uID  = "";
    azimuthAngles.clear();
    rotorBladeUID = "";
    //TODO?: hinges.Cleanup();
}

// Update internal rotor blade attachment data
void CCPACSRotorBladeAttachment::Update(void)
{
    if (!invalidated) {
        return;
    }

    invalidated = false;
}

// Invalidates internal state
void CCPACSRotorBladeAttachment::Invalidate(void)
{
    invalidated = true;
    for (int i = 1; i <= GetHingeCount(); i++) {
        GetHinge(i).Invalidate();
    }
//TODO:        for (int i = 1; i <= GetRotorBladeCount(); i++)
//TODO:        {
//TODO:            GetRotorBlade(i).Invalidate();
//TODO:        }
}

// Read CPACS rotor elements
void CCPACSRotorBladeAttachment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorBladeAttachmentXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get attribute "uID"
    char* ptrUID = NULL;
    tempString   = rotorBladeAttachmentXPath;
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, elementPath, "uID", &ptrUID) == SUCCESS) {
        uID = ptrUID;
    }

    // Get subelement "numberOfBlades"
    int numberOfBlades = 0;
    tempString  = rotorBladeAttachmentXPath + "/numberOfBlades";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS &&
        tixiGetIntegerElement(tixiHandle, elementPath, &numberOfBlades) == SUCCESS) {
        for (int i=0; i<numberOfBlades; ++i) {
            azimuthAngles.push_back(static_cast<double>(i)*360./static_cast<double>(numberOfBlades));
        }
    }

    // Get subelement "azimuthAngles"
    tempString = rotorBladeAttachmentXPath + "/azimuthAngles";
    elementPath = const_cast<char*>(tempString.c_str());
    int countAzimuthAngles;
    double* ptrAzimuthAngles = NULL;
    if (tixiGetVectorSize(tixiHandle, elementPath, &countAzimuthAngles) == SUCCESS) {
        if (tixiGetFloatVector(tixiHandle, elementPath, &ptrAzimuthAngles, countAzimuthAngles) == SUCCESS) {
            numberOfBlades = countAzimuthAngles;
            azimuthAngles.clear();
            for (int i=0; i<numberOfBlades; ++i) {
                azimuthAngles.push_back(ptrAzimuthAngles[i]);
            }
        }
        else {
            throw CTiglError("Error: XML error while reading azimuth angles vector <azimuthAngles> in CCPACSRotorBladeAttachment::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "rotorBladeUID"
    char* ptrRotorBladeUID = NULL;
    tempString    = rotorBladeAttachmentXPath + "/rotorBladeUID";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrRotorBladeUID) == SUCCESS) {
        rotorBladeUID = ptrRotorBladeUID;
    }


    // Get subelement "hinges"
    hinges.ReadCPACS(tixiHandle, rotorBladeAttachmentXPath + "/hinges", "hinge");

    Update();
}

// Returns the UID of the rotor blade attachment
const std::string& CCPACSRotorBladeAttachment::GetUID(void) const
{
    return uID;
}

// Returns the number of attached rotor blades
int CCPACSRotorBladeAttachment::GetNumberOfBlades(void) const
{
    return static_cast<int>(azimuthAngles.size());
}

// Returns the azimuth angle of the attached rotor blade with the given index
const double& CCPACSRotorBladeAttachment::GetAzimuthAngle(int index) const
{
    index --;
    if (index < 0 || index >= azimuthAngles.size()) {
        throw CTiglError("Error: Invalid index in CCPACSRotorBladeAttachment::GetAzimuthAngle", TIGL_INDEX_ERROR);
    }
    return (azimuthAngles[index]);
}

// Returns the UID of the referenced rotor blade
const std::string& CCPACSRotorBladeAttachment::GetRotorBladeUID(void) const
{
    return rotorBladeUID;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorBladeAttachment::GetConfiguration(void) const
{
    return *configuration;
}

// Get hinge count
int CCPACSRotorBladeAttachment::GetHingeCount(void) const
{
    return hinges.GetRotorHingeCount();
}

// Returns the hinge for a given index
CCPACSRotorHinge& CCPACSRotorBladeAttachment::GetHinge(const int index) const
{
    return hinges.GetRotorHinge(index);
}

//TODO:	// Get rotor blade count
//TODO:	int CCPACSRotorBladeAttachment::GetRotorBladeCount(void) const
//TODO:	{
//TODO:		return (static_cast<int>(rotorBlades.size()));
//TODO:	}

//TODO:    // Returns the rotor blade for a given index
//TODO:	CCPACSRotorBlade& CCPACSRotorBladeAttachment::GetRotorBlade(const int index) const
//TODO:	{
//TODO:        return rotorBlades.GetRotorBlade(index);
//TODO:	}

} // end namespace tigl
