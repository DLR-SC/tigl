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
* @brief  Implementation of CPACS fuselages handling routines.
*/

#ifndef CCPACSFUSELAGES_H
#define CCPACSFUSELAGES_H

#include "tigl_internal.h"
#include <vector>

#include "tixi.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageProfile.h"
#include "CCPACSFuselageProfiles.h"


namespace tigl
{

class CCPACSConfiguration;

class CCPACSFuselages
{

private:
    // Typedef for a CCPACSFuselage container to store the fuselages of a configuration.
    typedef std::vector<CCPACSFuselage*> CCPACSFuselageContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSFuselages(CCPACSConfiguration* config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSFuselages(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS fuselage elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID);

    // Write CPACS fuselage elements
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& configurationUID);

    TIGL_EXPORT bool HasProfile(std::string uid) const;

    // Returns the total count of fuselage profiles in this configuration
    TIGL_EXPORT int GetProfileCount(void) const;
        
    // Returns the wing profiles in this configuration
    TIGL_EXPORT CCPACSFuselageProfiles& GetProfiles(void);

    // Returns the fuselage profile for a given index.
    TIGL_EXPORT CCPACSFuselageProfile& GetProfile(int index) const;
    TIGL_EXPORT CCPACSFuselageProfile& GetProfile(std::string uid) const;

    // Returns the total count of fuselages in a configuration
    TIGL_EXPORT int GetFuselageCount(void) const;

    // Returns the fuselage for a given index.
    TIGL_EXPORT CCPACSFuselage& GetFuselage(int index) const;

    // Returns the fuselage for a given UID.
    TIGL_EXPORT CCPACSFuselage& GetFuselage(const std::string& UID) const;

    // Adds a new fuselage to the list of fuselages
    TIGL_EXPORT void AddFuselage(CCPACSFuselage* fuselage);

    // Returns the fuselage index for a given UID.
    TIGL_EXPORT int GetFuselageIndex(const std::string& UID) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSFuselages(const CCPACSFuselages& );

    // Assignment operator
    void operator=(const CCPACSFuselages& );

private:
    CCPACSFuselageProfiles   profiles;      /**< Fuselage profile elements */
    CCPACSFuselageContainer  fuselages;     /**< Fuselage elements */
    CCPACSConfiguration*     configuration; /**< Pointer to parent configuration */

};

} // end namespace tigl

#endif // CCPACSFUSELAGES_H
