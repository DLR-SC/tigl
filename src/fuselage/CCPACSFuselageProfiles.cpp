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

#include "CCPACSFuselageProfiles.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <sstream>
#include <iostream>

namespace tigl
{

// Constructor
CCPACSFuselageProfiles::CCPACSFuselageProfiles(void)
{
    Cleanup();
}

// Destructor
CCPACSFuselageProfiles::~CCPACSFuselageProfiles(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSFuselageProfiles::Cleanup(void)
{
    librarypath = "";

    CCPACSFuselageProfileContainer::iterator p;
    for (p = profiles.begin(); p!=profiles.end(); ++p) {
        CCPACSFuselageProfile *pro = p->second;
        delete pro;
    }
    profiles.clear();
}

// Invalidates internal state
void CCPACSFuselageProfiles::Invalidate(void)
{
    for (int i = 1; i < GetProfileCount(); i++) {
        GetProfile(i).Invalidate();
    }
}

// Read CPACS fuselage profiles
void CCPACSFuselageProfiles::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    Cleanup();

    if (tixiCheckElement(tixiHandle, "/cpacs/vehicles/profiles/fuselageProfiles") != SUCCESS) {
        return;
    }

    /* Get <geometry> element count */
    int geometryCount;
    if (tixiGetNamedChildrenCount(tixiHandle, "/cpacs/vehicles/profiles/fuselageProfiles", "fuselageProfile", &geometryCount) != SUCCESS) {
        throw CTiglError("Error: tixiGetNamedChildrenCount failed in CCPACSFuselageProfiles::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all <fuselageProfile> elements
    for (int i = 1; i <= geometryCount; i++) {
        std::ostringstream xpath;
        xpath << "/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[" << i << "]";

        CCPACSFuselageProfile* profile = new CCPACSFuselageProfile(xpath.str());
        profile->ReadCPACS(tixiHandle);
        profiles[profile->GetUID()] = profile;
    }
}

// Write CPACS fuselage profiles
void CCPACSFuselageProfiles::WriteCPACS(TixiDocumentHandle tixiHandle)
{
    const char* elementPath = "/cpacs/vehicles/profiles/fuselageProfiles";
    std::string path;
    ReturnCode tixiRet;
    int fuselageProfileCount, test;
    
    TixiSaveExt::TixiSaveElement(tixiHandle, "/cpacs/vehicles", "profiles");
    TixiSaveExt::TixiSaveElement(tixiHandle, "/cpacs/vehicles/profiles", "fuselageProfiles");
    
    if (tixiGetNamedChildrenCount(tixiHandle, elementPath, "fuselageProfile", &test) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselageProfiles::WriteCPACS", TIGL_XML_ERROR);
    }

    fuselageProfileCount = GetProfileCount();

    for (int i = 1; i <= fuselageProfileCount; i++) {
        std::stringstream ss;
        ss << elementPath << "/fuselageProfile[" << i << "]";
        path = ss.str();
        CCPACSFuselageProfile& fuselageProfile = GetProfile(i);
        if ((tixiRet = tixiCheckElement(tixiHandle, path.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, elementPath, "fuselageProfile")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSFuselageProfiles::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        fuselageProfile.WriteCPACS(tixiHandle, path);
    }

    for (int i = fuselageProfileCount + 1; i <= test; i++) {
        std::stringstream ss;
        ss << elementPath << "/fuselageProfile[" << fuselageProfileCount + 1 << "]";
        path = ss.str();
        tixiRet = tixiRemoveElement(tixiHandle, path.c_str());
    }
}

bool CCPACSFuselageProfiles::HasProfile(std::string uid) const
{
    CCPACSFuselageProfileContainer::const_iterator it = profiles.find(uid);
    if (it != profiles.end() && it->second) {
        return true;
    }
    else {
        return false;
    }
}

void CCPACSFuselageProfiles::AddProfile(CCPACSFuselageProfile* profile)
{
    // free memory for existing profiles
    if (profiles.find(profile->GetUID()) != profiles.end()) {
        delete profiles[profile->GetUID()];
    }
    profiles[profile->GetUID()] = profile;
}

void CCPACSFuselageProfiles::DeleteProfile( std::string uid )
{
    // free memory for existing profiles
    if (profiles.find( uid ) != profiles.end()) {
        profiles[ uid ]->Invalidate();
        delete profiles[ uid ];
        profiles.erase( uid );
    }
}

// Returns the total count of fuselage profiles in this configuration
int CCPACSFuselageProfiles::GetProfileCount(void) const
{
    return (static_cast<int>(profiles.size()));
}

// Returns the fuselage profile for a given uid.
CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(std::string uid) const
{
    CCPACSFuselageProfileContainer::const_iterator it = profiles.find(uid);
    if (it != profiles.end() && it->second) {
        return *(it->second);
    }
    else {
        throw CTiglError("Fuselage profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
    }
}

// Returns the fuselage profile for a given index - TODO: depricated function!
CCPACSFuselageProfile& CCPACSFuselageProfiles::GetProfile(int index) const
{
    int i = 0;
    CCPACSFuselageProfileContainer::const_iterator p;
    for (p = profiles.begin(); p!=profiles.end(); ++p) {
        if (i == index-1) {
            break;
        }
        i++;
    }
    return (*p->second);
}


} // end namespace tigl
