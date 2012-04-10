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

#ifndef CCPACSFUSELAGECONNECTION_H
#define CCPACSFUSELAGECONNECTION_H

#include "tixi.h"
#include "CCPACSFuselageProfile.h"
#include "CTiglTransformation.h"
#include <string>

namespace tigl {

	class CCPACSFuselageSegment;

	class CCPACSFuselageConnection
	{

	public:
		// Constructor
		CCPACSFuselageConnection(CCPACSFuselageSegment* aSegment);

		// Virtual Destructor
		virtual ~CCPACSFuselageConnection(void);

		// Read CPACS connection element
		void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& connectionXPath);

        // Returns the section UID of this connection
		std::string GetSectionUID(void) const;

        // Returns the section element UID of this connection
        std::string GetSectionElementUID(void) const;

        // Returns the section index of this connection
		int GetSectionIndex(void) const;

        // Returns the section element index of this connection
        int GetSectionElementIndex(void) const;

        // Returns the fuselage profile referenced by this connection
		CCPACSFuselageProfile& GetProfile(void) const;

		// Returns the positioning transformation (segment transformation) for the referenced section
		CTiglTransformation GetPositioningTransformation(void) const;

		// Returns the section matrix referenced by this connection
		CTiglTransformation GetSectionTransformation(void) const;

		// Returns the section element matrix referenced by this connection
		CTiglTransformation GetSectionElementTransformation(void) const;

	protected:
		// Cleanup routine
		void Cleanup(void);

	private:
		// Copy constructor
		CCPACSFuselageConnection(const CCPACSFuselageConnection& ) : segment(0) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSFuselageConnection& ) { /* Do nothing */ }

	private:
		int 						   sectionIndex; /**< Index in sections */
		int							   elementIndex; /**< Index in section/elements */
		std::string                    sectionUID; 	 /**< UID in sections */
		std::string                    elementUID; 	 /**< UID in section/elements */
		CCPACSFuselageSegment* 		   segment;      /**< Parent segment */

	};

} // end namespace tigl

#endif // CCPACSFUSELAGECONNECTION_H
