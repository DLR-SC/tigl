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

#include "CCPACSWingProfiles.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include <sstream>
#include <iostream>

namespace tigl
{

// Constructor
CCPACSWingProfiles::CCPACSWingProfiles(void)
{
    Cleanup();
}

// Destructor
CCPACSWingProfiles::~CCPACSWingProfiles(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingProfiles::Cleanup(void)
{
    CCPACSWingProfileContainer::iterator p;
    for (p = profiles.begin(); p!=profiles.end(); ++p) {
        CCPACSWingProfile *pro = p->second;
        delete pro;
    }
    profiles.clear();
}

// Invalidates internal state
void CCPACSWingProfiles::Invalidate(void)
{
    CCPACSWingProfileContainer::iterator p;
    for (p = profiles.begin(); p!=profiles.end(); ++p) {
        CCPACSWingProfile *pro = p->second;
        pro->Invalidate();
    }
}

// Read CPACS wing profiles
void CCPACSWingProfiles::ReadCPACS(TixiDocumentHandle tixiHandle, const bool doAppend, const std::string wingProfilesLibraryPath, const std::string wingProfileElementName)
{
    if (!doAppend) {
        Cleanup();
    }

    ReturnCode    tixiRet;
    int           elementCount;
    std::string   elementString;
    char*         elementPath;

    /* Get wing profile count */
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, wingProfilesLibraryPath.c_str(), wingProfileElementName.c_str(), &elementCount);
    if (tixiRet != SUCCESS) {
        //cerr << "Warning: no wing profiles are defined!" << endl;
        return;
    }

    // Loop over all profile elements
    for (int i = 1; i <= elementCount; i++) {
        /* Get the appropriate airfoil */
        std::ostringstream airfoilTmpStream;
        airfoilTmpStream << wingProfilesLibraryPath << "/" << wingProfileElementName << "[" << i << "]";
        CCPACSWingProfile* profile = new CCPACSWingProfile(airfoilTmpStream.str());
        profile->ReadCPACS(tixiHandle);
        CCPACSWingProfileContainer::const_iterator it = profiles.find(profile->GetUID());
        if (it != profiles.end() && it->second) {
            LOG(WARNING) << "CCPACSWingProfiles::ReadCPACS(): A wing profile with UID \"" + profile->GetUID() + "\" does already exist in the wing profile collection. It will not be replaced." << endl;
            delete profile;
        }
        else {
            profiles[profile->GetUID()] = profile;
        }
    }
}

// Returns the total count of wing profiles in this configuration
int CCPACSWingProfiles::GetProfileCount(void) const
{
    return (static_cast<int>(profiles.size()));
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(std::string uid) const
{
    CCPACSWingProfileContainer::const_iterator it = profiles.find(uid);
    if (it != profiles.end() && it->second) {
        return *(it->second);
    }
    else {
        throw CTiglError("Wing profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
    }
}

// Returns the wing profile for a given index - TODO: depricated function!
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(int index) const
{
    int i = 0;
    CCPACSWingProfileContainer::const_iterator p;
    for (p = profiles.begin(); p!=profiles.end(); ++p) {
        if (i == index-1) {
            break;
        }
        i++;
    }
    return (*p->second);
}

} // end namespace tigl
