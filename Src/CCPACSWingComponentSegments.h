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

#ifndef CCPACSWINGCOMPONENTSEGMENTS_H
#define CCPACSWINGCOMPONENTSEGMENTS_H

#include <string>
#include <vector>

#include "tixi.h"
#include "CCPACSWingComponentSegment.h"
#include "CTiglError.h"

namespace tigl {

	class CCPACSWing;

	class CCPACSWingComponentSegments
	{
	private:
		// Typedef for a CCPACSWingSegment container to store the segments of a wing.
		typedef std::vector<CCPACSWingComponentSegment*> CCPACSWingComponentSegmentContainer;

	public:
		// Constructor
		CCPACSWingComponentSegments(CCPACSWing* aWing);

		// Virtual Destructor
		virtual ~CCPACSWingComponentSegments(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS segments element
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath);

		// Gets a segment by index. 
		CCPACSWingComponentSegment & GetComponentSegment(const int index);
		CCPACSWingComponentSegment & GetComponentSegment(const std::string& componentSegmentUID);

		// Gets total segment count
		int GetComponentSegmentCount(void);

	protected:
		// Cleanup routine
		void Cleanup(void);

	private:
		// Copy constructor
		CCPACSWingComponentSegments(const CCPACSWingComponentSegments& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWingComponentSegments& ) { /* Do nothing */ }

	private:
		CCPACSWingComponentSegmentContainer componentSegments;       /**< ComponentSegment elements    	*/
		CCPACSWing*        			        wing;      			     /**< Parent wing         			*/

	};

} // end namespace tigl

#endif // CCPACSWINGCOMPONENTSEGMENTS_H
