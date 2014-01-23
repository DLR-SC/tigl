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
* @brief  Implementation of CPACS fuselage profiles handling routines.
*/

#ifndef CCPACSFUSELAGEPROFILES_H
#define CCPACSFUSELAGEPROFILES_H

#include "tixi.h"
#include "CCPACSFuselageProfile.h"
#include <string>
#include <map>

namespace tigl
{

class CCPACSFuselageProfiles
{

private:
    // Typedef for a container to store the fuselage profiles.
    typedef std::map<std::string, CCPACSFuselageProfile*> CCPACSFuselageProfileContainer;

public:
    // Constructor
    CCPACSFuselageProfiles(void);

    // Virtual Destructor
    virtual ~CCPACSFuselageProfiles(void);

    // Read CPACS fuselage profiles
    void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the total count of fuselage profiles in this configuration
    int GetProfileCount(void) const;

    // Returns the fuselage profile for a given index.
    CCPACSFuselageProfile& GetProfile(int index) const;

    // Returns the fuselage profile for a given uid.
    CCPACSFuselageProfile& GetProfile(std::string uid) const;

    // Invalidates internal state
    void Invalidate(void);

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSFuselageProfiles(const CCPACSFuselageProfiles& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CCPACSFuselageProfiles& ) { /* Do nothing */ }

private:
    std::string                    librarypath;  // Directory path to fuselage profiles
    CCPACSFuselageProfileContainer profiles;     // All fuselage profiles

};

} // end namespace tigl

#endif // CCPACSFUSELAGEPROFILES_H
