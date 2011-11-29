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
* @brief  Implementation of CPACS wings handling routines.
*/

#ifndef CCPACSWINGS_H
#define CCPACSWINGS_H

#include "tixi.h"
#include "CCPACSWing.h"
#include "CCPACSWingProfile.h"
#include "CCPACSWingProfiles.h"
#include <vector>

namespace tigl {

	class CCPACSConfiguration;

	class CCPACSWings
	{
	private:
		// Typedef for a CCPACSWing container to store the wings of a configuration.
		typedef std::vector<CCPACSWing*> CCPACSWingContainer;

	public:
		// Constructor
		CCPACSWings(CCPACSConfiguration* config);

		// Virtual Destructor
		virtual ~CCPACSWings(void);

		// Invalidates internal state
		void Invalidate(void);

		// Read CPACS wings elements
		void ReadCPACS(TixiDocumentHandle tixiHandle, char* configurationUID);

		// Returns the total count of wing profiles in this configuration
		int GetProfileCount(void) const;

		// Returns the wing profile for a given uid.
		CCPACSWingProfile& GetProfile(std::string uid) const;

		// Returns the wing profile for a given index - TODO: depricated function!
		CCPACSWingProfile& GetProfile(int index) const;

		// Returns the total count of wings in a configuration
		int GetWingCount(void) const;

		// Returns the wing for a given index.
		CCPACSWing& GetWing(int index) const;

        // Returns the wing for a given UID.
        CCPACSWing& GetWing(const std::string UID) const;

	protected:
		// Cleanup routine
		void Cleanup(void);

	private:
		// Copy constructor
		CCPACSWings(const CCPACSWings& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWings& ) { /* Do nothing */ }

	private:
		CCPACSWingProfiles   profiles;      /**< Wing profile elements */
		CCPACSWingContainer  wings;         /**< Wing elements */
		CCPACSConfiguration* configuration; /**< Pointer to parent configuration */

	};

} // end namespace tigl

#endif // CCPACSWINGS_H
