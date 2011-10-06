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

#ifndef CCPACSFUSELAGES_H
#define CCPACSFUSELAGES_H

#include <vector>

#include "tixi.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageProfile.h"
#include "CCPACSFuselageProfiles.h"


namespace tigl {

	class CCPACSConfiguration;

	class CCPACSFuselages
	{

	private:
		// Typedef for a CCPACSFuselage container to store the fuselages of a configuration.
		typedef std::vector<CCPACSFuselage*> CCPACSFuselageContainer;

	public:
		// Constructor
		CCPACSFuselages(CCPACSConfiguration* config);

		// Virtual Destructor
		virtual ~CCPACSFuselages(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS fuselage elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, char* configurationUID);

		// Returns the total count of fuselage profiles in this configuration
		int GetProfileCount(void) const;

		// Returns the fuselage profile for a given index.
		CCPACSFuselageProfile& GetProfile(int index) const;
		CCPACSFuselageProfile& GetProfile(std::string uid) const;

		// Returns the total count of fuselages in a configuration
		int GetFuselageCount(void) const;

		// Returns the fuselage for a given index.
		CCPACSFuselage& GetFuselage(int index) const;

        // Returns the fuselage for a given UID.
        CCPACSFuselage& GetFuselage(const std::string UID) const;

	protected:
		// Cleanup routine
		void Cleanup(void);

	private:
		// Copy constructor
		CCPACSFuselages(const CCPACSFuselages& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSFuselages& ) { /* Do nothing */ }

	private:
		CCPACSFuselageProfiles   profiles;      /**< Fuselage profile elements */
		CCPACSFuselageContainer  fuselages;     /**< Fuselage elements */
		CCPACSConfiguration*     configuration; /**< Pointer to parent configuration */

	};

} // end namespace tigl

#endif // CCPACSFUSELAGES_H
