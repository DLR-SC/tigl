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
* @brief  Implementation of CPACS fuselage positionings handling routines.
*/

#include "CCPACSFuselagePositionings.h"
#include <iostream>
#include <sstream>

#include "TixiSaveExt.h"

namespace tigl
{

// Constructor
CCPACSFuselagePositionings::CCPACSFuselagePositionings(void)
    : positionings()
    , invalidated(true)
{
}

// Destructor
CCPACSFuselagePositionings::~CCPACSFuselagePositionings(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSFuselagePositionings::Invalidate(void)
{
    invalidated = true;
    CCPACSFuselagePositioningIterator it;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSFuselagePositioning* pos = it->second;
        pos->Invalidate();
    }
}

// Cleanup routine
void CCPACSFuselagePositionings::Cleanup(void)
{
    CCPACSFuselagePositioningIterator it;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSFuselagePositioning* pos = it->second;
        delete pos;
    }
    positionings.clear();
    invalidated = true;
}

// Returns the positioning matrix for a given section index
CTiglTransformation CCPACSFuselagePositionings::GetPositioningTransformation(std::string sectionIndex)
{
    Update();
    CCPACSFuselagePositioningIterator iter = positionings.find(sectionIndex);
        
    // check, if section has positioning definition, if not
    // return Zero-Transformation
    if (iter == positionings.end()){
        CTiglTransformation zeroTrans;
        zeroTrans.SetIdentity();
        return zeroTrans;
    }
    return iter->second->GetEndTransformation();
}

// Update internal positionings structure
void CCPACSFuselagePositionings::Update(void)
{
    if (!invalidated) {
        return;
    }

    invalidated = false;
        
    // reset all position base points
    CCPACSFuselagePositioningIterator it;
    
    // diconnect and reset
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSFuselagePositioning* actPos = it->second;
        actPos->DisconnectChilds();
        actPos->SetStartPoint(CTiglPoint(0,0,0));
    }
    
    // connect positionings, find roots
    std::vector<CCPACSFuselagePositioning*> rootNodes;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSFuselagePositioning* actPos = it->second;
        std::string fromUID = actPos->GetStartSectionIndex();
        if (fromUID != "") {
            CCPACSFuselagePositioningIterator pos = positionings.find(fromUID);
            if (pos != positionings.end()) {
                CCPACSFuselagePositioning* fromPos = pos->second;
                fromPos->ConnectChildPositioning(actPos);
            }
            else {
                // invalid from UID
                throw CTiglError("Positioning fromSectionUID " + fromUID + " does not exist");
            }
        }
        else {
            rootNodes.push_back(actPos);
        }
    }

    for (std::vector<CCPACSFuselagePositioning*>::iterator it = rootNodes.begin(); it != rootNodes.end(); it++) {
        UpdateNextPositioning(*it, 0);
    }
}



void CCPACSFuselagePositionings::UpdateNextPositioning(CCPACSFuselagePositioning* currPos, int depth)
{
    // check for recursive definition
    if (depth > 1000) {
        throw CTiglError("Recursive definition of fuselage positioning is not allowed");
    }

    if (currPos->GetEndSectionIndex() == ""){
        throw CTiglError("illegal definition of fuselage positionings");
    }
        
    // Find all positionings which have the end section of the current positioning
    // defined as their start section.
    std::vector<CCPACSFuselagePositioning*>::const_iterator it;
    const std::vector<CCPACSFuselagePositioning*>& childs = currPos->GetChilds();
    for (it = childs.begin(); it != childs.end(); ++it) {
        CCPACSFuselagePositioning* childPos = *it;
        childPos->SetStartPoint(currPos->GetEndPoint());
        UpdateNextPositioning(childPos, depth+1);
    }
}

// Read CPACS positionings element
void CCPACSFuselagePositionings::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath)
{
    Cleanup();

    ReturnCode    tixiRet;
    int           positioningCount;
    std::string   tempString;
    char*         elementPath;

    /* Get positioning element count */
    tempString  = fuselageXPath + "/positionings";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath, "positioning", &positioningCount);
    if (tixiRet != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselagePositionings::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all positionings
    for (int i = 1; i <= positioningCount; i++) {
        CCPACSFuselagePositioning* positioning = new CCPACSFuselagePositioning();

        tempString = fuselageXPath + "/positionings/positioning[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";
        positioning->ReadCPACS(tixiHandle, xpath.str());
        
        positionings[positioning->GetEndSectionIndex()] = positioning;
    }

    Update();
}

// Write CPACS positionings element
void CCPACSFuselagePositionings::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath)
{
    std::string elementPath;
    std::string xpath;
    ReturnCode    tixiRet;
    int test = 0;

    elementPath = fuselageXPath + "/positionings";
    
    TixiSaveExt::TixiSaveElement(tixiHandle, fuselageXPath.c_str(), "positionings");
    
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "positioning", &test);
    size_t positioningCount = positionings.size();

    CCPACSFuselagePositioningIterator it;
    int i = 0;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        i++;
        CCPACSFuselagePositioning* positioning = it->second;
        std::stringstream ss;
        ss << elementPath << "/positioning[" << i << "]";
        xpath = ss.str();
        if ((tixiRet = tixiCheckElement(tixiHandle, xpath.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, elementPath.c_str(), "positioning")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSWingsPositionnings::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        positioning->WriteCPACS(tixiHandle, xpath);
    }
    for (size_t i = positioningCount + 1; i <= test; i++) {
        std::stringstream ss;
        ss << elementPath << "/positioning[" << positioningCount + 1 << "]";
        xpath = ss.str();
        tixiRemoveElement(tixiHandle, xpath.c_str());
    }
}

} // end namespace tigl
