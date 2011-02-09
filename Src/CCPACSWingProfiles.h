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
* @brief  Implementation of CPACS wing profiles handling routines.
*/

#ifndef CCPACSWINGPROFILES_H
#define CCPACSWINGPROFILES_H

#include "tixi.h"
#include "CCPACSWingProfile.h"
#include <string>
#include <map>

namespace tigl {

	class CCPACSWingProfiles
	{

	private:
		// Typedef for a container to store the wing profiles and their uid.
		typedef std::map<std::string, CCPACSWingProfile*> CCPACSWingProfileContainer;

	public:
		// Constructor
		CCPACSWingProfiles(void);

		// Virtual Destructor
		virtual ~CCPACSWingProfiles(void);

		// Read CPACS wing profiles
		void ReadCPACS(TixiDocumentHandle tixiHandle);

		// Returns the total count of wing profiles in this configuration
		int GetProfileCount(void) const;

		// Returns the wing profile for a given index or uid.
		CCPACSWingProfile& GetProfile(std::string uid) const;

		// Returns the wing profile for a given index or uid - TODO: depricated!
		CCPACSWingProfile& GetProfile(int index) const;

		// Invalidates internal state
		void Invalidate(void);

	protected:
		// Cleanup routine
		void Cleanup(void);

	private:
		// Copy constructor
		CCPACSWingProfiles(const CCPACSWingProfiles& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CCPACSWingProfiles& ) { /* Do nothing */ }

	private:
		std::string                librarypath; // Directory path to wing profiles
		CCPACSWingProfileContainer profiles;    // All wing profiles

	};

} // end namespace tigl

#endif // CCPACSWINGPROFILES_H
