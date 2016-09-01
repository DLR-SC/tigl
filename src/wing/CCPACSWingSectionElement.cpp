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
#include "TixiSaveExt.h"
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
    description  = "";
    profileUID   = "";
    uID          = "";
    transformation.reset();
}

// Read CPACS wing section elements
void CCPACSWingSectionElement::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    Cleanup();

    char* ptrName = NULL;
    if (tixiGetTextElement(tixiHandle, (elementXPath + "/name").c_str(), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    char* ptrdescription = NULL;
    if (tixiGetTextElement(tixiHandle, (elementXPath + "/description").c_str(), &ptrdescription) == SUCCESS) {
        description = ptrdescription;
    }

    char* ptrUID  = NULL;
    if (tixiGetTextElement(tixiHandle, (elementXPath + "/airfoilUID").c_str(), &ptrUID) == SUCCESS) {
        profileUID = ptrUID;
    }

    char* ptrMyUID  = NULL;
    if (tixiGetTextAttribute(tixiHandle, elementXPath.c_str(), "uID", &ptrMyUID) == SUCCESS) {
        uID = ptrMyUID;
    }

    // Get Transformation
    transformation.ReadCPACS(tixiHandle, elementXPath);
}

// Write CPACS wing section elements
void CCPACSWingSectionElement::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, elementXPath.c_str(), "uID", uID.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "name", name.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "description", description.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "airfoilUID", profileUID.c_str());

    transformation.WriteCPACS(tixiHandle, elementXPath);
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

// Getter for the member name
std::string CCPACSWingSectionElement::GetName(void) const
{
    return name;
}

// Getter for the member description
std::string CCPACSWingSectionElement::GetDescription(void) const
{
    return description;
}

// Gets the section element transformation
CTiglTransformation CCPACSWingSectionElement::GetSectionElementTransformation(void) const
{
    return transformation.getTransformationMatrix();
}

} // end namespace tigl
