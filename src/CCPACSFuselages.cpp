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
* @brief  Implementation of CPACS fuselages handling routines.
*/

#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CTiglError.h"
#include <iostream>
#include <sstream>

namespace tigl {

	// Constructor
	CCPACSFuselages::CCPACSFuselages(CCPACSConfiguration* config)
		: configuration(config)
	{
		Cleanup();
	}

	// Destructor
	CCPACSFuselages::~CCPACSFuselages(void)
	{
		Cleanup();
	}

	// Invalidates internal state
	void CCPACSFuselages::Invalidate(void)
	{
		profiles.Invalidate();
		for (int i = 1; i <= GetFuselageCount(); i++)
		{
			GetFuselage(i).Invalidate();
		}
	}

	// Cleanup routine
	void CCPACSFuselages::Cleanup(void)
	{
		for (CCPACSFuselageContainer::size_type i = 0; i < fuselages.size(); i++)
        {
			delete fuselages[i];
		}
		fuselages.clear();
	}

	// Read CPACS fuselages element
	void CCPACSFuselages::ReadCPACS(TixiDocumentHandle tixiHandle, char* configurationUID)
	{
		Cleanup();
		char *fuselagesXPathPrt = NULL;
		char *tmpString = NULL;

		if (tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString) != SUCCESS)
            throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSFuselages::ReadCPACS", TIGL_XML_ERROR);

		fuselagesXPathPrt = (char *) malloc(sizeof(char) * (strlen(tmpString) + 50));
		strcpy(fuselagesXPathPrt, tmpString);
		strcat(fuselagesXPathPrt, "[@uID=\"");
		strcat(fuselagesXPathPrt, configurationUID);
		strcat(fuselagesXPathPrt, "\"]/fuselages");
	
		// Read fuselage profiles
		profiles.ReadCPACS(tixiHandle);

		if (tixiCheckElement(tixiHandle, fuselagesXPathPrt) != SUCCESS){
			free(fuselagesXPathPrt);
			return;
		}
		/* Get fuselage element count */
		int fuselageCount;
		if (tixiGetNamedChildrenCount(tixiHandle, fuselagesXPathPrt, "fuselage", &fuselageCount) != SUCCESS)
			throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselages::ReadCPACS", TIGL_XML_ERROR);

		// Loop over all fuselages
		for (int i = 1; i <= fuselageCount; i++) {
			CCPACSFuselage* fuselage = new CCPACSFuselage(configuration);
			fuselages.push_back(fuselage);

			std::ostringstream xpath;
			xpath << fuselagesXPathPrt << "/fuselage[" << i << "]";
			fuselage->ReadCPACS(tixiHandle, xpath.str());
		}
		free(fuselagesXPathPrt);
	}

	// Returns the total count of fuselage profiles in this configuration
	int CCPACSFuselages::GetProfileCount(void) const
	{
		return profiles.GetProfileCount();
	}

	// Returns the fuselage profile for a given index.
	CCPACSFuselageProfile& CCPACSFuselages::GetProfile(int index) const
	{
		return profiles.GetProfile(index);
	}

	// Returns the fuselage profile for a given uid.
	CCPACSFuselageProfile& CCPACSFuselages::GetProfile(std::string uid) const
	{
		return profiles.GetProfile(uid);
	}

	// Returns the total count of fuselages in a configuration
	int CCPACSFuselages::GetFuselageCount(void) const
	{
		return (static_cast<int>(fuselages.size()));
	}

	// Returns the fuselage for a given index.
	CCPACSFuselage& CCPACSFuselages::GetFuselage(int index) const
	{
        index--;
		if (index < 0 || index >= GetFuselageCount())
			throw CTiglError("Error: Invalid index in CCPACSFuselages::GetFuselage", TIGL_INDEX_ERROR);
		return (*fuselages[index]);
	}

	// Returns the fuselage for a given index.
    CCPACSFuselage& CCPACSFuselages::GetFuselage(std::string UID) const
    {
        for(int i=0; i < GetFuselageCount(); i++)
        {
            const std::string tmpUID(fuselages[i]->GetUID());
            if(tmpUID == UID) {
                return (*fuselages[i]);
            }
        }

        // UID not there
            throw CTiglError("Error: Invalid index in CCPACSFuselages::GetFuselage", TIGL_INDEX_ERROR);
    }


} // end namespace tigl
