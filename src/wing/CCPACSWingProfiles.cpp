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
#include "TixiSaveExt.h"
#include "IOHelper.h"
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
void CCPACSWingProfiles::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    Cleanup();
    // call separate import method for reading (allows for importing
    // additional profiles from other files)
    ImportCPACS(tixiHandle);
}

void CCPACSWingProfiles::ImportCPACS(TixiDocumentHandle tixiHandle)
{
    const std::string xpath = "/cpacs/vehicles/profiles";
    std::vector<CCPACSWingProfile*> children;
    ReadContainerElement(tixiHandle, xpath, "wingAirfoil", 0, children);
    ReadContainerElement(tixiHandle, xpath, "rotorAirfoil", 0, children);
    for (std::size_t i = 0; i < children.size(); i++) {
        CCPACSWingProfile* child = children[i];
        if (profiles.find(child->GetUID()) != profiles.end()) {
            delete profiles[child->GetUID()];
        }
        profiles[child->GetUID()] = child;
    }
}

// Write CPACS wing profiles
void CCPACSWingProfiles::WriteCPACS(TixiDocumentHandle tixiHandle)
{
    if (profiles.size() > 0) {
        TixiSaveExt::TixiSaveElement(tixiHandle, "/cpacs/vehicles", "profiles");
    }

    const std::string xpath = "/cpacs/vehicles/profiles";
    WriteContainerElement(tixiHandle, xpath, "wingAirfoil", profiles);
}

void CCPACSWingProfiles::AddProfile(CCPACSWingProfile* profile)
{
    // free memory for existing profiles
    if (profiles.find(profile->GetUID()) != profiles.end()) {
        delete profiles[profile->GetUID()];
    }
    profiles[profile->GetUID()] = profile;
}


void CCPACSWingProfiles::DeleteProfile( std::string uid )
{
    // free memory for existing profiles
    if (profiles.find( uid ) != profiles.end()) {
        profiles[ uid ]->Invalidate();
        delete profiles[ uid ];
        profiles.erase( uid );
    }
}

// Returns the total count of wing profiles in this configuration
int CCPACSWingProfiles::GetProfileCount(void) const
{
    return (static_cast<int>(profiles.size()));
}

bool CCPACSWingProfiles::HasProfile(std::string uid) const
{
    CCPACSWingProfileContainer::const_iterator it = profiles.find(uid);
    if (it != profiles.end() && it->second) {
        return true;
    }
    else {
        return false;
    }
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
