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
* @brief  Implementation of CPACS fuselage segments handling routines.
*/

#include "CCPACSFuselageSegments.h"
#include <iostream>
#include <sstream>


namespace tigl {

	// Constructor
	CCPACSFuselageSegments::CCPACSFuselageSegments(CCPACSFuselage* aFuselage)
		: segments()
		, fuselage(aFuselage)
	{
	}

	// Destructor
	CCPACSFuselageSegments::~CCPACSFuselageSegments(void)
	{
		Cleanup();
	}

	// Invalidates internal state
	void CCPACSFuselageSegments::Invalidate(void)
	{
		for (int i = 1; i <= GetSegmentCount(); i++)
		{
			GetSegment(i).Invalidate();
		}
	}

	// Cleanup routine
	void CCPACSFuselageSegments::Cleanup(void)
	{
		for (CCPACSFuselageSegmentContainer::size_type i = 0; i < segments.size(); i++) {
			delete segments[i];
		}
		segments.clear();
	}

	// Gets a segment by index. 
	CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(const int index)
	{
        const int idx = index - 1;
        if (idx < 0 || idx >= GetSegmentCount()) 
        {
			throw CTiglError("Error: Invalid index value in CCPACSFuselageSegments::GetSegment", TIGL_INDEX_ERROR);
        }
        return (CCPACSFuselageSegment &) (*(segments[idx]));
	}

	// Gets total segment count
	int CCPACSFuselageSegments::GetSegmentCount(void)
	{
		return static_cast<int>(segments.size());
	}

	// Read CPACS segments element
	void CCPACSFuselageSegments::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath)
	{
		Cleanup();

		ReturnCode    tixiRet;
		int           segmentCount;
		std::string   tempString;
		char*         elementPath;

		/* Get segment element count */
		tempString  = fuselageXPath + "/segments";
		elementPath = const_cast<char*>(tempString.c_str());
		tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath, "segment", &segmentCount);
		if (tixiRet != SUCCESS)
			throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselageSegments::ReadCPACS", TIGL_XML_ERROR);

		// Loop over all segments
        for (int i = 1; i <= segmentCount; i++) {
			CCPACSFuselageSegment* segment = new CCPACSFuselageSegment(fuselage, i);
			segments.push_back(segment);

			tempString = fuselageXPath + "/segments/segment[";
			std::ostringstream xpath;
			xpath << tempString << i << "]"; 
			segment->ReadCPACS(tixiHandle, xpath.str());
        }

	}

} // end namespace tigl
