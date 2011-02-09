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
* @brief  Implementation of CPACS wing positionings handling routines.
*/

#ifndef CCPACSWINGPOSITIONINGS_H
#define CCPACSWINGPOSITIONINGS_H

#include "tixi.h"
#include "CCPACSWingPositioning.h"
#include "CTiglError.h"
#include <string>
#include <vector>
#include <map>

namespace tigl {

	class CCPACSWingPositionings
	{

	private:
		// Typedef for a CCPACSWingPositioning container to store the positionings.
		typedef std::vector<CCPACSWingPositioning*> CCPACSWingPositioningContainer;

		// Typedef for a CTiglTransformation map to store multiple transformations by a section-uid.
		typedef std::map<std::string, CTiglTransformation> CCPACSTransformationMap;
		typedef CCPACSTransformationMap::iterator  CCPACSTransformationMapIterator;

	public:
		// Constructor
		CCPACSWingPositionings(void);

		// Virtual Destructor
		virtual ~CCPACSWingPositionings(void);

		// Read CPACS positionings element
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath);

		// Invalidates internal state
		void Invalidate(void);

		// Gets a positioning by index.
		CCPACSWingPositioning& GetPositioning(int index) const;

		// Gets total positioning count
		int GetPositioningCount(void) const;

		// Returns the positioning matrix for a given section-uid
		CTiglTransformation GetPositioningTransformation(std::string sectionIndex);

	protected:
		// Cleanup routine
		void Cleanup(void);

		// Update internal positioning structure
		void Update(void);

        // Update connected positionings recursive
        void UpdateNextPositioning(CCPACSWingPositioning& currPos);

	private:
		// Copy constructor
		CCPACSWingPositionings(const CCPACSWingPositionings& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWingPositionings& ) { /* Do nothing */ }

	private:
		CCPACSWingPositioningContainer positionings;      /**< Positioning elements */
		CCPACSTransformationMap        transformations;   /**< Map of transformations */
		bool                           invalidated;       /**< Internal state flag  */

	};

} // end namespace tigl

#endif // CCPACSWINGPOSITIONINGS_H
