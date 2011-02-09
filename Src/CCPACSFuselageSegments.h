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

#ifndef CCPACSFUSELAGESEGMENTS_H
#define CCPACSFUSELAGESEGMENTS_H

#include "tixi.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglError.h"
#include <string>
#include <vector>

namespace tigl {

	class CCPACSFuselage;

	class CCPACSFuselageSegments
	{

	private:
		// Typedef for a CCPACSFuselageSegment container to store the segments of a fuselage.
		typedef std::vector<CCPACSFuselageSegment*> CCPACSFuselageSegmentContainer;

	public:
		// Constructor
		CCPACSFuselageSegments(CCPACSFuselage* aFuselage);

		// Virtual Destructor
		virtual ~CCPACSFuselageSegments(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segments element
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath);

		// Gets a segment by index. 
		CCPACSFuselageSegment& GetSegment(int index);

		// Gets total segment count
		int GetSegmentCount(void);

	protected:
		// Cleanup routine
		void Cleanup(void);

	private:
		// Copy constructor
		CCPACSFuselageSegments(const CCPACSFuselageSegments& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSFuselageSegments& ) { /* Do nothing */ }

	private:
		CCPACSFuselageSegmentContainer segments; /**< Segment elements    */
		CCPACSFuselage*                fuselage; /**< Parent fuselage     */

	};

} // end namespace tigl

#endif // CCPACSFUSELAGESEGMENTS_H
