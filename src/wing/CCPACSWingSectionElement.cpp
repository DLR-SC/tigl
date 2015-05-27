/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Implementation of CPACS wing section element handling routines.
*/

#include "CCPACSWingSectionElement.h"
#include "CTiglError.h"
#include <iostream>

namespace tigl
{

// Constructor
CCPACSWingSectionElement::CCPACSWingSectionElement()
{
    Cleanup();
}

// Destructor
CCPACSWingSectionElement::~CCPACSWingSectionElement(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingSectionElement::Cleanup(void)
{
    name         = "";
    profileUID     = "";
    uID          = "";
    transformation.reset();
}

// Read CPACS wing section elements
void CCPACSWingSectionElement::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = elementXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name          = ptrName;
    }

    // Get subelement "profileUID"
    char* ptrUID  = NULL;
    tempString    = elementXPath + "/airfoilUID";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrUID) == SUCCESS) {
        profileUID      = ptrUID;
    }

    // Get attribute "uID"
    char* ptrMyUID  = NULL;
    tempString    = elementXPath;
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, elementPath, "uID", &ptrMyUID) == SUCCESS) {
        uID      = ptrMyUID;
    }

    // Get Transformation
    transformation.ReadCPACS(tixiHandle, elementXPath);
}

// Returns the UID of the referenced wing profile
std::string CCPACSWingSectionElement::GetProfileIndex(void) const
{
    return profileUID;
}

// Returns the UID of this WingSectionElement
std::string CCPACSWingSectionElement::GetUID(void) const
{
    return uID;
}

// Returns the uid of the profile of this element
std::string CCPACSWingSectionElement::GetProfileUID(void) const
{
    return profileUID;
}

// Gets the section element transformation
CTiglTransformation CCPACSWingSectionElement::GetSectionElementTransformation(void) const
{
    return transformation.getTransformationMatrix();
}

} // end namespace tigl
