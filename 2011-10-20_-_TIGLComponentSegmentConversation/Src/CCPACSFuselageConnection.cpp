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
* @brief  Implementation of CPACS fuselage connection handling routines.
*/

#include "CCPACSFuselageConnection.h"
#include "CTiglError.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSConfiguration.h"
#include <iostream>

namespace tigl {

	// Constructor
	CCPACSFuselageConnection::CCPACSFuselageConnection(CCPACSFuselageSegment* aSegment)
		: segment(aSegment)
	{
		Cleanup();
	}

	// Destructor
	CCPACSFuselageConnection::~CCPACSFuselageConnection(void)
	{
		Cleanup();
	}

	// Cleanup routine
	void CCPACSFuselageConnection::Cleanup(void)
	{
		sectionUID = "";
		elementUID = "";
		sectionIndex = -1;
		elementIndex = -1;
	}

    // Returns the section index of this connection
	std::string CCPACSFuselageConnection::GetSectionUID(void) const
    {
        return sectionUID;
    }

    // Returns the section element index of this connection
	std::string CCPACSFuselageConnection::GetSectionElementUID(void) const
    {
        return elementUID;
    }


    // Returns the section index of this connection
	int CCPACSFuselageConnection::GetSectionIndex(void) const
	{
		return sectionIndex;
	}

    // Returns the section element index of this connection
    int CCPACSFuselageConnection::GetSectionElementIndex(void) const
    {
    	return elementIndex;
    }


	// Returns the fuselage profile referenced by this connection
	CCPACSFuselageProfile& CCPACSFuselageConnection::GetProfile(void) const
	{
		CCPACSFuselage& fuselage              = segment->GetFuselage();
		std::string profileUID;

		for(int i=1; i <= fuselage.GetSectionCount(); i++) {
			CCPACSFuselageSection& section        = fuselage.GetSection(i);
			for(int j=1; j <= section.GetSectionElementCount(); j++) {
				if(section.GetSectionElement(j).GetUID() == elementUID ) {
					CCPACSFuselageSectionElement& element = section.GetSectionElement(j);
					profileUID            = element.GetProfileIndex();
				}
			}
		}
		CCPACSConfiguration& config  	  = fuselage.GetConfiguration();

		return (config.GetFuselageProfile(profileUID));
	}

	// Returns the positioning transformation for the referenced section
	CTiglTransformation CCPACSFuselageConnection::GetPositioningTransformation(void) const
	{
		return (segment->GetFuselage().GetPositioningTransformation(sectionUID));
	}

	// Returns the section matrix referenced by this connection
	CTiglTransformation CCPACSFuselageConnection::GetSectionTransformation(void) const
	{
		CCPACSFuselage& fuselage       = segment->GetFuselage();
		CTiglTransformation	transformation;

		for (int i = 1; i <= fuselage.GetSectionCount(); i++) {
			CCPACSFuselageSection& section = fuselage.GetSection(i);
			for (int j = 1; j <= section.GetSectionElementCount(); j++) {
				if (section.GetSectionElement(j).GetUID() == elementUID) {
					transformation = section.GetSectionTransformation();
				}
			}
		}
		return transformation;
	}

	// Returns the section element matrix referenced by this connection
	CTiglTransformation CCPACSFuselageConnection::GetSectionElementTransformation(void) const
	{
			CCPACSFuselage& fuselage = segment->GetFuselage();
			CTiglTransformation transformation;

			for (int i = 1; i <= fuselage.GetSectionCount(); i++) {
				CCPACSFuselageSection& section = fuselage.GetSection(i);
				for (int j = 1; j <= section.GetSectionElementCount(); j++) {
					if (section.GetSectionElement(j).GetUID() == elementUID) {
						CCPACSFuselageSectionElement& element = section.GetSectionElement(j);
						transformation = element.GetSectionElementTransformation();
					}
				}  
			}
			return transformation;
	}

	// Read CPACS section elements
	void CCPACSFuselageConnection::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& connectionXPath)
	{
		Cleanup();

		char*       elementPath;
		std::string tempString;

		// Get subelement "element"
		char*			ptrElementUID = "";
		tempString	=	connectionXPath;
		elementPath   = const_cast<char*>(tempString.c_str());
		if (tixiGetTextElement(tixiHandle, elementPath, &ptrElementUID) != SUCCESS)
			throw CTiglError("Error: Can't read element <element/> in CCPACSFuselageConnection::ReadCPACS", TIGL_XML_ERROR);
		elementUID = ptrElementUID;

		// find the corresponding section to this segment
		CCPACSFuselage& fuselage = segment->GetFuselage();
		for(int i=1; i <= fuselage.GetSectionCount(); i++) {
			CCPACSFuselageSection& section        = fuselage.GetSection(i);
			for(int j=1; j <= section.GetSectionElementCount(); j++) {
				if(section.GetSectionElement(j).GetUID() == elementUID ) {
					sectionUID = section.GetUID();
					sectionIndex = i;
					elementIndex = j;
				}
			}
		}
	}

} // end namespace tigl
