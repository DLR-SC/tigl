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
* @brief  Implementation of CPACS wing ComponentSegments handling routines.
*/

#include <iostream>
#include <sstream>
#include <exception>

#include "CCPACSWingComponentSegments.h"

namespace tigl {

	// Constructor
	CCPACSWingComponentSegments::CCPACSWingComponentSegments(CCPACSWing* aWing)
		: componentSegments()
		, wing(aWing)
	{
	}

	// Destructor
	CCPACSWingComponentSegments::~CCPACSWingComponentSegments(void)
	{
		Cleanup();
	}

	// Invalidates internal state
	void CCPACSWingComponentSegments::Invalidate(void)
	{
		for (int i = 1; i <= GetComponentSegmentCount(); i++)
		{
			GetComponentSegment(i).Invalidate();
		}
	}

	// Cleanup routine
	void CCPACSWingComponentSegments::Cleanup(void)
	{
		for (CCPACSWingComponentSegmentContainer::size_type i = 0; i < componentSegments.size(); i++) {
			delete componentSegments[i];
		}
		componentSegments.clear();
	}


	// Gets a componentSegment by index.
	CCPACSWingComponentSegment & CCPACSWingComponentSegments::GetComponentSegment(const int index)
	{
		const int idx = index - 1;
		if (idx < 0 || idx >= GetComponentSegmentCount())
			throw CTiglError("Error: Invalid index value in CCPACSWingComponentSegments::GetComponentSegment", TIGL_INDEX_ERROR);
		return (CCPACSWingComponentSegment &) (*(componentSegments[idx]));
	}

	// Gets a componentSegment by uid.
	CCPACSWingComponentSegment & CCPACSWingComponentSegments::GetComponentSegment(const std::string& componentSegmentUID)
	{
		for (CCPACSWingComponentSegmentContainer::size_type i = 0; i < componentSegments.size(); i++) {
			if (componentSegments[i]->GetUID() == componentSegmentUID) {
				return (CCPACSWingComponentSegment &) (*(componentSegments[i]));
			}
		}
		throw CTiglError("Error: Invalid uid in CCPACSWingComponentSegments::GetComponentSegment", TIGL_INDEX_ERROR);
	}

	// Gets total segment count
	int CCPACSWingComponentSegments::GetComponentSegmentCount(void)
	{
		return static_cast<int>(componentSegments.size());
	}

	// Read CPACS segments element
	void CCPACSWingComponentSegments::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
	{
		Cleanup();

		ReturnCode    tixiRet;
		int           componentSegmentCount;
		std::string   tempString;
		char*         elementPath;

		/* Get segment element count */
		tempString  = wingXPath + "/componentSegments";
		elementPath = const_cast<char*>(tempString.c_str());
		tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath, "componentSegment", &componentSegmentCount);
		if (tixiRet != SUCCESS)
			throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWingComponentSegments::ReadCPACS", TIGL_XML_ERROR);

		// Loop over all segments
		for (int i = 1; i <= componentSegmentCount; i++) {
			CCPACSWingComponentSegment* componentSegment = new CCPACSWingComponentSegment(wing, i);
			componentSegments.push_back(componentSegment);

			tempString = wingXPath + "/componentSegments/componentSegment[";
			std::ostringstream xpath;
			xpath << tempString << i << "]";
			componentSegment->ReadCPACS(tixiHandle, xpath.str());
		}

	}

} // end namespace tigl
