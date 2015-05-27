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
* @brief  Implementation of CPACS wing section handling routines.
*/

#include "CCPACSWingSection.h"
#include "CTiglError.h"
#include <iostream>

namespace tigl
{

// Constructor
CCPACSWingSection::CCPACSWingSection()
{
    Cleanup();
}

// Destructor
CCPACSWingSection::~CCPACSWingSection(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingSection::Cleanup(void)
{
    name = "";
    uID  = "";
    transformation.reset();
}

// Read CPACS section elements
void CCPACSWingSection::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = sectionXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get attribute "uID"
    char* ptrUID = NULL;
    tempString    = sectionXPath;
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, elementPath, "uID", &ptrUID) == SUCCESS) {
        uID = ptrUID;
    }

    // Get Transformation
    transformation.ReadCPACS(tixiHandle, sectionXPath);

    // Get subelement "elements", which means the section elements
    elements.ReadCPACS(tixiHandle, sectionXPath);
}

// Get profile count for this section
int CCPACSWingSection::GetSectionElementCount(void) const
{
    return elements.GetSectionElementCount();
}

// Get the UID of this WingSection
const std::string& CCPACSWingSection::GetUID(void) const
{
    return uID;
}

// Get element for a given index
CCPACSWingSectionElement& CCPACSWingSection::GetSectionElement(int index) const
{
    return elements.GetSectionElement(index);
}

// Gets the section transformation
CTiglTransformation CCPACSWingSection::GetSectionTransformation(void) const
{
    return transformation.getTransformationMatrix();
}

} // end namespace tigl
