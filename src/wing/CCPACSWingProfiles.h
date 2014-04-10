/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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

#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSWingProfile.h"
#include <string>
#include <map>

namespace tigl
{

class CCPACSWingProfiles
{

private:
    // Typedef for a container to store the wing profiles and their uid.
    typedef std::map<std::string, CCPACSWingProfile*> CCPACSWingProfileContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSWingProfiles(void);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingProfiles(void);

    // Read CPACS wing profiles
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const bool doAppend=false, const bool isRotorProfile=false, const std::string wingProfilesLibraryPath="/cpacs/vehicles/profiles/wingAirfoils", const std::string wingProfileElementName="wingAirfoil");

    // Returns the total count of wing profiles in this configuration
    TIGL_EXPORT int GetProfileCount(void) const;

    // Returns the wing profile for a given index or uid.
    TIGL_EXPORT CCPACSWingProfile& GetProfile(std::string uid) const;

    // Returns the wing profile for a given index or uid - TODO: depricated!
    TIGL_EXPORT CCPACSWingProfile& GetProfile(int index) const;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSWingProfiles(const CCPACSWingProfiles& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CCPACSWingProfiles& ) { /* Do nothing */ }

private:
    CCPACSWingProfileContainer profiles;    // All wing profiles

};

} // end namespace tigl

#endif // CCPACSWINGPROFILES_H
