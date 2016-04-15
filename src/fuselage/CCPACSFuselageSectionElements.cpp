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
* @brief  Implementation of CPACS fuselage section elements handling routines.
*/

#include "CCPACSFuselageSectionElements.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSFuselageSectionElements::CCPACSFuselageSectionElements(void)
{
    Cleanup();
}

// Destructor
CCPACSFuselageSectionElements::~CCPACSFuselageSectionElements(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSFuselageSectionElements::Cleanup(void)
{
    for (CCPACSFuselageSectionElementContainer::size_type i = 0; i < elements.size(); i++) {
        delete elements[i];
    }
    elements.clear();
}

// Read CPACS fuselage section elements
void CCPACSFuselageSectionElements::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
{
    Cleanup();

    ReturnCode    tixiRet;
    int           elementCount;
    std::string   tempString;
    char*         elementPath;

    /* Get section element count */
    tempString  = sectionXPath + "/elements";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath, "element", &elementCount);
    if (tixiRet != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselageSectionElements::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all section elements
    for (int i = 1; i <= elementCount; i++) {
        CCPACSFuselageSectionElement* element = new CCPACSFuselageSectionElement();
        elements.push_back(element);

        tempString = sectionXPath + "/elements/element[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";
        element->ReadCPACS(tixiHandle, xpath.str());
    }
}

// Write CPACS fuselage section elements
void CCPACSFuselageSectionElements::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
{
    std::string elementPath;
    std::string xpath;
    ReturnCode    tixiRet;
    int           sectionElementCount, test;

    TixiSaveExt::TixiSaveElement(tixiHandle, sectionXPath.c_str(), "elements");
    elementPath = sectionXPath + "/elements";
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "element", &test);
    sectionElementCount = GetSectionElementCount();

    for (int i = 1; i <= sectionElementCount; i++) {
        CCPACSFuselageSectionElement& sectionElement = GetSectionElement(i);
        std::stringstream ss;
        ss << elementPath << "/element[" << i << "]";
        xpath = ss.str();
        if ((tixiRet = tixiCheckElement(tixiHandle, xpath.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, elementPath.c_str(), "element")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSFuselageSectionElements::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        sectionElement.WriteCPACS(tixiHandle, xpath);
    }
    for (int i = sectionElementCount + 1; i <= test; i++) {
        std::stringstream ss;
        ss << elementPath << "/element[" << sectionElementCount + 1 << "]";
        xpath = ss.str();
        tixiRemoveElement(tixiHandle, xpath.c_str());
    }
}

// Get element count for this section
int CCPACSFuselageSectionElements::GetSectionElementCount(void) const
{
    return static_cast<int>(elements.size());
}

// Get element for a given index
CCPACSFuselageSectionElement& CCPACSFuselageSectionElements::GetSectionElement(int index) const
{
    index--;
    if (index < 0 || index >= GetSectionElementCount()) {
        throw CTiglError("Error: Invalid index in CCPACSFuselageSectionElements::GetSectionElement", TIGL_INDEX_ERROR);
    }
    return (*elements[index]);
}


} // end namespace tigl
