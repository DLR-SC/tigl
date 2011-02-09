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
* @brief  Implementation of CPACS wing segments handling routines.
*/

#ifndef CCPACSWINGSEGMENTS_H
#define CCPACSWINGSEGMENTS_H

#include <string>
#include <vector>

#include "tixi.h"
#include "CCPACSWingSegment.h"
#include "CTiglError.h"

namespace tigl {

	class CCPACSWing;

	class CCPACSWingSegments
	{
	private:
		// Typedef for a CCPACSWingSegment container to store the segments of a wing.
		typedef std::vector<CCPACSWingSegment*> CCPACSWingSegmentContainer;

	public:
		// Constructor
		CCPACSWingSegments(CCPACSWing* aWing);

		// Virtual Destructor
		virtual ~CCPACSWingSegments(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segments element
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath);

		// Gets a segment by index. 
		CCPACSWingSegment & GetSegment(const int index);

		// Gets total segment count
		int GetSegmentCount(void);

	protected:
		// Cleanup routine
		void Cleanup(void);

	private:
		// Copy constructor
		CCPACSWingSegments(const CCPACSWingSegments& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWingSegments& ) { /* Do nothing */ }

	private:
		CCPACSWingSegmentContainer segments;       /**< Segment elements    */
		CCPACSWing*                wing;           /**< Parent wing         */

	};

} // end namespace tigl

#endif // CCPACSWINGSEGMENTS_H
