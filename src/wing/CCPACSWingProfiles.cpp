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
    librarypath = "";

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
    // [[CAS_AES]] call separate import method for reading (allows for importing
    // additional profiles from other files)
    ImportCPACS(tixiHandle);
}

void CCPACSWingProfiles::ImportCPACS(TixiDocumentHandle tixiHandle)
{
    ReturnCode    tixiRet;
    int           elementCount;
    std::string   elementPath;

    /* Get wing element count */
    elementPath = "/cpacs/vehicles/profiles/wingAirfoils";
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "wingAirfoil", &elementCount);
    if (tixiRet != SUCCESS) {
        cerr << "Warning: no wing profiles are defined!" << endl;
        return;
    }

    // Loop over all section elements
    for (int i = 1; i <= elementCount; i++) {
        /* Get the appropriate airfoil */
        std::ostringstream airfoilTmpStream;
        airfoilTmpStream << "/cpacs/vehicles/profiles/wingAirfoils/wingAirfoil[" << i << "]";
        CCPACSWingProfile* profile = new CCPACSWingProfile(airfoilTmpStream.str());
        profile->ReadCPACS(tixiHandle);
        // [[CAS_AES]] free memory for existing profiles
        if (profiles.find(profile->GetUID()) != profiles.end()) {
            delete profiles[profile->GetUID()];
        }
        profiles[profile->GetUID()] = profile;
    }
}

// Write CPACS wing profiles
void CCPACSWingProfiles::WriteCPACS(TixiDocumentHandle tixiHandle)
{
    std::string elementPath = "/cpacs/vehicles/profiles/wingAirfoils";
    std::string path;
    ReturnCode tixiRet;
    int wingAirfoilCount, test;

    TixiSaveExt::TixiSaveElement(tixiHandle, "/cpacs/vehicles", "profiles");
    TixiSaveExt::TixiSaveElement(tixiHandle, "/cpacs/vehicles/profiles", "wingAirfoils");

    if (tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "wingAirfoil", &test) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWings::ReadCPACS", TIGL_XML_ERROR);
    }

    wingAirfoilCount = profiles.size();

    for (int i = 1;i <= wingAirfoilCount;i++) {
        std::stringstream ss;
        ss << elementPath << "/wingAirfoil[" << i << "]";
        path = ss.str();
        CCPACSWingProfile& wingAirfoil = GetProfile(i);
        if ((tixiRet = tixiCheckElement(tixiHandle, path.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, elementPath.c_str(), "wingAirfoil")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSWings::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        wingAirfoil.WriteCPACS(tixiHandle, path);
    }

    for (int i = wingAirfoilCount+1; i <= test; i++) {
        std::stringstream ss;
        ss << elementPath << "/wingAirfoil[" << wingAirfoilCount+1 << "]";
        path = ss.str();
        tixiRet = tixiRemoveElement(tixiHandle, path.c_str());
    }
}

void CCPACSWingProfiles::AddProfile(CCPACSWingProfile* profile)
{
    // [[CAS_AES]] free memory for existing profiles
    if (profiles.find(profile->GetUID()) != profiles.end()) {
        delete profiles[profile->GetUID()];
    }
    profiles[profile->GetUID()] = profile;
}


void CCPACSWingProfiles::DeleteProfile( std::string uid ) {
        
    // [[CAS_AES]] free memory for existing profiles
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
