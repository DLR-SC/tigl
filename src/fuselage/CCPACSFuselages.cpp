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

#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSFuselages::CCPACSFuselages(CCPACSConfiguration* config)
    : configuration(config)
{
    Cleanup();
}

// Destructor
CCPACSFuselages::~CCPACSFuselages(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSFuselages::Invalidate(void)
{
    profiles.Invalidate();
    for (int i = 1; i <= GetFuselageCount(); i++) {
        GetFuselage(i).Invalidate();
    }
}

// Cleanup routine
void CCPACSFuselages::Cleanup(void)
{
    for (CCPACSFuselageContainer::size_type i = 0; i < fuselages.size(); i++) {
        delete fuselages[i];
    }
    fuselages.clear();
}

// Read CPACS fuselages element
void CCPACSFuselages::ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID)
{
    Cleanup();
    char *tmpString = NULL;

    if (tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSFuselages::ReadCPACS", TIGL_XML_ERROR);
    }

    std::string fuselagesXPath = tmpString;
    fuselagesXPath += "[@uID=\"";
    fuselagesXPath += configurationUID;
    fuselagesXPath += "\"]/fuselages";
    
    // Read fuselage profiles
    profiles.ReadCPACS(tixiHandle);

    if (tixiCheckElement(tixiHandle, fuselagesXPath.c_str()) != SUCCESS){
        return;
    }
    /* Get fuselage element count */
    int fuselageCount;
    if (tixiGetNamedChildrenCount(tixiHandle, fuselagesXPath.c_str(), "fuselage", &fuselageCount) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselages::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all fuselages
    for (int i = 1; i <= fuselageCount; i++) {
        CCPACSFuselage* fuselage = new CCPACSFuselage(configuration);
        fuselages.push_back(fuselage);

        std::ostringstream xpath;
        xpath << fuselagesXPath << "/fuselage[" << i << "]";
        fuselage->ReadCPACS(tixiHandle, xpath.str());
    }
}

// Write CPACS fuselage elements
void CCPACSFuselages::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& configurationUID)
{
    std::string fuselageXPathPrt;
    char *tmpString = NULL;
    std::string xpath;
    int test, fuselageCount;
    ReturnCode tixiRet;

    // when the model doesn't exist, an error is thrown
    if (tixiUIDGetXPath(tixiHandle, configurationUID.c_str(), &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSFuselages::WriteCPACS", TIGL_XML_ERROR);
    }
    if (strcmp(tmpString, "") == 0) {
        throw CTiglError("XML error in CCPACSFuselages::WriteCPACS : Path not found", TIGL_XML_ERROR);
    }

    std::stringstream ss;
    ss << tmpString << "[@uID=\"" << configurationUID << "\"]";
    xpath = ss.str();
    fuselageXPathPrt = xpath + "/fuselages";

    profiles.WriteCPACS(tixiHandle);
    TixiSaveExt::TixiSaveElement(tixiHandle, xpath.c_str(), "fuselages");

    tixiRet = tixiGetNamedChildrenCount(tixiHandle, fuselageXPathPrt.c_str(), "fuselage", &test);
    fuselageCount = this->GetFuselageCount();

    for (int i = 1; i <= fuselageCount; i++) {
        std::stringstream ss;
        ss << fuselageXPathPrt << "/fuselage[" << i << "]";
        xpath = ss.str();
        CCPACSFuselage& fuselage = GetFuselage(i);
        if ((tixiRet = tixiCheckElement(tixiHandle, xpath.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, fuselageXPathPrt.c_str(), "fuselage")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSFuselages::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        fuselage.WriteCPACS(tixiHandle, xpath);
    }

    for (int i = fuselageCount + 1; i <= test; i++) {
        std::stringstream ss;
        ss << fuselageXPathPrt << "/fuselage[" << fuselageCount + 1 << "]";
        xpath = ss.str();
        tixiRet = tixiRemoveElement(tixiHandle, xpath.c_str());
    }
}

bool CCPACSFuselages::HasProfile(std::string uid) const
{
    return profiles.HasProfile(uid);
}

// Returns the total count of fuselage profiles in this configuration
int CCPACSFuselages::GetProfileCount(void) const
{
    return profiles.GetProfileCount();
}

CCPACSFuselageProfiles& CCPACSFuselages::GetProfiles(void) 
{
    return profiles;
}

// Returns the fuselage profile for a given index.
CCPACSFuselageProfile& CCPACSFuselages::GetProfile(int index) const
{
    return profiles.GetProfile(index);
}

// Returns the fuselage profile for a given uid.
CCPACSFuselageProfile& CCPACSFuselages::GetProfile(std::string uid) const
{
    return profiles.GetProfile(uid);
}

// Returns the total count of fuselages in a configuration
int CCPACSFuselages::GetFuselageCount(void) const
{
    return (static_cast<int>(fuselages.size()));
}

// Returns the fuselage for a given index.
CCPACSFuselage& CCPACSFuselages::GetFuselage(int index) const
{
    index--;
    if (index < 0 || index >= GetFuselageCount()) {
        throw CTiglError("Error: Invalid index in CCPACSFuselages::GetFuselage", TIGL_INDEX_ERROR);
    }
    return (*fuselages[index]);
}

// Returns the fuselage for a given UID.
CCPACSFuselage& CCPACSFuselages::GetFuselage(const std::string& UID) const
{
    return (*fuselages[GetFuselageIndex(UID)-1]);
}

// Returns the fuselage index for a given UID.
int CCPACSFuselages::GetFuselageIndex(const std::string& UID) const
{
    for (int i=0; i < GetFuselageCount(); i++) {
        const std::string tmpUID(fuselages[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSFuselages::GetFuselageIndex", TIGL_UID_ERROR);
}


void CCPACSFuselages::AddFuselage(CCPACSFuselage* fuselage)
{
    // Check whether the same fuselage already exists if yes remove it before adding the new one
    CCPACSFuselageContainer::iterator it;
    for (it = fuselages.begin(); it != fuselages.end(); ++it) {
        if ((*it)->GetUID() == fuselage->GetUID()) {
            delete (*it);
            fuselages.erase(it);
            break;
        }
    }

    // Add the new fuselage to the fuselage list
    fuselages.push_back(fuselage);
}



} // end namespace tigl
