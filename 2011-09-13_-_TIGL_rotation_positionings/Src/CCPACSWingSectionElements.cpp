/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
* @brief  Implementation of CPACS wing section elements handling routines.
*/

#include "CCPACSWingSectionElements.h"
#include "CTiglError.h"
#include <iostream>
#include <sstream>

namespace tigl {

	// Constructor
	CCPACSWingSectionElements::CCPACSWingSectionElements(void)
	{
		Cleanup();
	}

	// Destructor
	CCPACSWingSectionElements::~CCPACSWingSectionElements(void)
	{
		Cleanup();
	}

	// Cleanup routine
	void CCPACSWingSectionElements::Cleanup(void)
	{
		for (CCPACSWingSectionElementContainer::size_type i = 0; i < elements.size(); i++)
		{
			delete elements[i];
		}
		elements.clear();
	}

	// Read CPACS wing section elements
	void CCPACSWingSectionElements::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
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
			throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWingSectionElements::ReadCPACS", TIGL_XML_ERROR);
		}

		// Loop over all section elements
		for (int i = 1; i <= elementCount; i++) {
			CCPACSWingSectionElement* element = new CCPACSWingSectionElement();
			elements.push_back(element);

			tempString = sectionXPath + "/elements/element[";
			std::ostringstream xpath;
			xpath << tempString << i << "]";
			element->ReadCPACS(tixiHandle, xpath.str());
		}
	}

	// Get element count for this section
	int CCPACSWingSectionElements::GetSectionElementCount(void) const
	{
		return static_cast<int>(elements.size());
	}

	// Get element for a given index
	CCPACSWingSectionElement& CCPACSWingSectionElements::GetSectionElement(int index) const
	{
        index--;
		if (index < 0 || index >= GetSectionElementCount())
			throw CTiglError("Error: Invalid index in CCPACSWingSectionElements::GetSectionElement", TIGL_INDEX_ERROR);
		return (*elements[index]);
	}


} // end namespace tigl
