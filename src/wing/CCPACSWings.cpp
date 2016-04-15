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

#include "CCPACSWings.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSWings::CCPACSWings(CCPACSConfiguration* config)
    : configuration(config)
{
    Cleanup();
}

// Destructor
CCPACSWings::~CCPACSWings(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWings::Invalidate(void)
{
    profiles.Invalidate();
    for (int i = 1; i <= GetWingCount(); i++) {
        GetWing(i).Invalidate();
    }
}

// Cleanup routine
void CCPACSWings::Cleanup(void)
{
    for (CCPACSWingContainer::size_type i = 0; i < wings.size(); i++) {
        delete wings[i];
    }
    wings.clear();
}

// Read CPACS wings element
void CCPACSWings::ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID)
{
    Cleanup();
    char *tmpString = NULL;

    if (tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSWings::ReadCPACS", TIGL_XML_ERROR);
    }

    std::string wingXPath= tmpString;
    wingXPath += "[@uID=\"";
    wingXPath += configurationUID;
    wingXPath += "\"]/wings";

    // Read wing profiles
    profiles.ReadCPACS(tixiHandle);

    if (tixiCheckElement(tixiHandle, wingXPath.c_str()) != SUCCESS) {
        return;
    }

    /* Get wing element count */
    int wingCount;
    if (tixiGetNamedChildrenCount(tixiHandle, wingXPath.c_str(), "wing", &wingCount) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWings::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all wings
    for (int i = 1; i <= wingCount; i++) {
        CCPACSWing* wing = new CCPACSWing(configuration);
        wings.push_back(wing);

        std::ostringstream xpath;
        xpath << wingXPath << "/wing[" << i << "]";
        wing->ReadCPACS(tixiHandle, xpath.str());
    }
}

// Write CPACS wings elements
void CCPACSWings::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& configurationUID)
{
    std::string wingXPathPrt;
    char *tmpString = NULL;
    std::string xpath;
    int test, wingCount;
    ReturnCode tixiRet;

    // tixi frees tmpString internally when finished
    if (tixiUIDGetXPath(tixiHandle, configurationUID.c_str(), &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSWings::WriteCPACS", TIGL_XML_ERROR);
    }
    if (strcmp(tmpString, "") == 0) {
        throw CTiglError("XML error in CCPACSWings::WriteCPACS : Path not found", TIGL_XML_ERROR);
    }

    std::stringstream ss;
    ss << tmpString << "[@uID=\"" << configurationUID << "\"]";
    xpath = ss.str();
    wingXPathPrt = xpath;
    wingXPathPrt.append("/wings");

    // save the wing profiles
    profiles.WriteCPACS(tixiHandle);
    TixiSaveExt::TixiSaveElement(tixiHandle, xpath.c_str(), "wings");

    tixiRet = tixiGetNamedChildrenCount(tixiHandle, wingXPathPrt.c_str(), "wing", &test);
    wingCount = this->GetWingCount();

    for (int i = 1; i <= wingCount; i++) {
        std::stringstream ss;
        ss << wingXPathPrt << "/wing[" << i << "]";
        xpath = ss.str();
        CCPACSWing& wing = GetWing(i);
        if ((tixiRet = tixiCheckElement(tixiHandle, xpath.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, wingXPathPrt.c_str(), "wing")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSWings::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        wing.WriteCPACS(tixiHandle, xpath);
    }

    for (int i = wingCount + 1; i <= test; i++) {
        std::stringstream ss;
        ss << wingXPathPrt << "/wing[" << wingCount + 1 << "]";
        xpath = ss.str();
        tixiRet = tixiRemoveElement(tixiHandle, xpath.c_str());
    }
}

bool CCPACSWings::HasProfile(std::string uid) const
{
    return profiles.HasProfile(uid);
}

// Returns the total count of wing profiles in this configuration
int CCPACSWings::GetProfileCount(void) const
{
    return profiles.GetProfileCount();
}

CCPACSWingProfiles& CCPACSWings::GetProfiles(void)
{
    return profiles;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWings::GetProfile(std::string uid) const
{
    return profiles.GetProfile(uid);
}

// Returns the wing profile for a given index.
CCPACSWingProfile& CCPACSWings::GetProfile(int index) const
{
    return profiles.GetProfile(index);
}

// Returns the total count of wings in a configuration
int CCPACSWings::GetWingCount(void) const
{
    return (static_cast<int>(wings.size()));
}

// Returns the wing for a given index.
CCPACSWing& CCPACSWings::GetWing(int index) const
{
    index --;
    if (index < 0 || index >= GetWingCount()) {
        throw CTiglError("Error: Invalid index in CCPACSWings::GetWing", TIGL_INDEX_ERROR);
    }
    return (*wings[index]);
}

// Returns the wing for a given UID.
CCPACSWing& CCPACSWings::GetWing(const std::string& UID) const
{
    for (int i=0; i < GetWingCount(); i++) {
        const std::string tmpUID(wings[i]->GetUID());
        if (tmpUID == UID) {
            return (*wings[i]);
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSWings::GetWing", TIGL_INDEX_ERROR);
}

CCPACSWingFuelTank& CCPACSWings::GetWingFuelTank(std::string nUId, bool& sym) const
{
    for (int w = 0; w < GetWingCount(); w++) {
        CCPACSWing* cWing = wings[w];
            
        for (int c = 1; c <= cWing->GetComponentSegmentCount(); c++) {
            CCPACSWingComponentSegment& CpacsWCSegment = (CCPACSWingComponentSegment&) cWing->GetComponentSegment(c);
            CCPACSWingFuelTanks& fuelTanks = CpacsWCSegment.GetWingFuelTanks();
                
            for (int t = 1; t <= fuelTanks.GetWingFuelTankCount(); t++) {
                CCPACSWingFuelTank& tank = fuelTanks.GetWingFuelTank(t);
                    
                if (nUId == tank.GetUID()) {
                    TiglSymmetryAxis symmetryAxis = cWing->GetSymmetryAxis();
                        
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        sym = true;
                    }
                    
                    return tank;
                }
            }
        }
    }
        
    throw tigl::CTiglError("Error: Invalid uid in CCPACSWingFuelTanks::GetWingFuelTank", TIGL_UID_ERROR);
}

CCPACSWingFuelTank& CCPACSWings::GetWingFuelTank(std::string nUId, std::string& wingUId) const
{
    for (int w = 0; w < GetWingCount(); w++) {
        CCPACSWing* cWing = wings[w];
            
        for (int c = 1; c <= cWing->GetComponentSegmentCount(); c++) {
            CCPACSWingComponentSegment& CpacsWCSegment = (CCPACSWingComponentSegment&) cWing->GetComponentSegment(c);
            CCPACSWingFuelTanks& fuelTanks = CpacsWCSegment.GetWingFuelTanks();
                
            for (int t = 1; t <= fuelTanks.GetWingFuelTankCount(); t++) {
                CCPACSWingFuelTank& tank = fuelTanks.GetWingFuelTank(t);
                    
                if (nUId == tank.GetUID()) {
                    wingUId = cWing->GetUID();
                    return tank;
                }
            }
        }
    }
    
    throw tigl::CTiglError("Error: Invalid uid in CCPACSWingFuelTanks::GetWingFuelTank", TIGL_UID_ERROR);
}

void CCPACSWings::AddWing(CCPACSWing* wing)
{
    // Check whether the same wing already exists if yes remove it before adding the new one
    CCPACSWingContainer::iterator it;
    for (it = wings.begin(); it != wings.end(); ++it) {
        if ((*it)->GetUID() == wing->GetUID()) {
            delete (*it);
            wings.erase(it);
            break;
        }
    }

    // Add the new wing to the wing list
    wings.push_back(wing);
}

} // end namespace tigl
