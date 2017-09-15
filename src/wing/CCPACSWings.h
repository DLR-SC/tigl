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
* @brief  Implementation of CPACS wings handling routines.
*/

#ifndef CCPACSWINGS_H
#define CCPACSWINGS_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSWing.h"
#include "CCPACSWingProfile.h"
#include "CCPACSWingProfiles.h"
#include <vector>

namespace tigl
{

class CCPACSConfiguration;

class CCPACSWings
{
private:
    // Typedef for a CCPACSWing container to store the wings of a configuration.
    typedef std::vector<CCPACSWing*> CCPACSWingContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSWings(CCPACSConfiguration* config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWings(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS wings elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID);

    // Write CPACS wings elements
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& configurationUID);

    TIGL_EXPORT bool HasProfile(std::string uid) const;

    // Returns the wing profiles in this configuration
    TIGL_EXPORT CCPACSWingProfiles& GetProfiles(void);

    // Returns the total count of wing profiles in this configuration
    TIGL_EXPORT int GetProfileCount(void) const;

    // Returns the wing profile for a given uid.
    TIGL_EXPORT CCPACSWingProfile& GetProfile(std::string uid) const;

    // Returns the wing profile for a given index - TODO: depricated function!
    TIGL_EXPORT CCPACSWingProfile& GetProfile(int index) const;

    // Returns the total count of wings in a configuration
    TIGL_EXPORT int GetWingCount(void) const;

    // Returns the count of wings in a configuration with the property isRotorBlade set to true
    TIGL_EXPORT int GetRotorBladeCount(void) const;

    // Returns the wing for a given index.
    TIGL_EXPORT CCPACSWing& GetWing(int index) const;

    // Returns the wing for a given UID.
    TIGL_EXPORT CCPACSWing& GetWing(const std::string& UID) const;
    
    // Adds a wing to the wing list
    TIGL_EXPORT void AddWing(CCPACSWing* wing);

    // Returns the wing index for a given UID.
    TIGL_EXPORT int GetWingIndex(const std::string& UID) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSWings(const CCPACSWings& );

    // Assignment operator
    void operator=(const CCPACSWings& );

    void ReadCPACSWing(TixiDocumentHandle tixiHandle,
                       const char* configurationUID,
                       const char* wingsLibraryName,
                       const char* wingElementName);
    
private:
    CCPACSWingProfiles   profiles;      /**< Wing profile elements */
    CCPACSWingContainer  wings;         /**< Wing elements */
    CCPACSConfiguration* configuration; /**< Pointer to parent configuration */

};

} // end namespace tigl

#endif // CCPACSWINGS_H
