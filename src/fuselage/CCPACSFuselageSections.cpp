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
* @brief  Implementation of CPACS fuselage sections handling routines.
*/

#include "CCPACSFuselageSections.h"

#include "CCPACSFuselageSection.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"

#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSFuselageSections::CCPACSFuselageSections(void)
{
    Cleanup();
}

// Destructor
CCPACSFuselageSections::~CCPACSFuselageSections(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSFuselageSections::Cleanup(void)
{
    for (CCPACSFuselageSectionContainer::size_type i = 0; i < sections.size(); i++) {
        delete sections[i];
    }
    sections.clear();
}

// Read CPACS fuselage sections element
void CCPACSFuselageSections::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath)
{
    Cleanup();

    ReturnCode    tixiRet;
    int           sectionCount;
    std::string   tempString;
    char*         elementPath;

    /* Get section element count */
    tempString  = fuselageXPath + "/sections";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath, "section", &sectionCount);
    if (tixiRet != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselageSections::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all sections
    for (int i = 1; i <= sectionCount; i++) {
        CCPACSFuselageSection* section = new CCPACSFuselageSection();
        sections.push_back(section);

        tempString = fuselageXPath + "/sections/section[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";
        section->ReadCPACS(tixiHandle, xpath.str());
    }
}

// Write CPACS fuselage sections element
void CCPACSFuselageSections::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath)
{
    std::string elementPath;
    std::string xpath;
    ReturnCode    tixiRet;
    int           sectionCount, test;

    TixiSaveExt::TixiSaveElement(tixiHandle, fuselageXPath.c_str(), "sections");
    elementPath = fuselageXPath + "/sections";
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "section", &test);
    sectionCount = GetSectionCount();

    for (int i = 1; i <= sectionCount; i++) {
        CCPACSFuselageSection& section = GetSection(i);
        std::stringstream ss;
        ss << elementPath << "/section[" << i << "]";
        xpath = ss.str();
        if ((tixiRet = tixiCheckElement(tixiHandle, xpath.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, elementPath.c_str(), "section")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSFuselageSections::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        section.WriteCPACS(tixiHandle, xpath);
    }
    for (int i = sectionCount + 1; i <= test; i++) {
        std::stringstream ss;
        ss << elementPath << "/section[" << sectionCount + 1 << "]";
        xpath = ss.str();
        tixiRemoveElement(tixiHandle, xpath.c_str());
    }
}

// Get section count
int CCPACSFuselageSections::GetSectionCount(void) const
{
    return static_cast<int>(sections.size());
}

// Returns the section for a given index
CCPACSFuselageSection& CCPACSFuselageSections::GetSection(int index) const
{
    index--;
    if (index < 0 || index >= GetSectionCount()) {
        throw CTiglError("Error: Invalid index in CCPACSFuselageSections::GetSection", TIGL_INDEX_ERROR);
    }
    return (*sections[index]);
}

} // end namespace tigl
