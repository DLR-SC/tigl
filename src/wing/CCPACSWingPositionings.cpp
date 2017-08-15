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
* @brief  Implementation of CPACS wing positionings handling routines.
*/

#include "CCPACSWingPositionings.h"
#include "TixiSaveExt.h"
#include "IOHelper.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSWingPositionings::CCPACSWingPositionings(void)
    : positionings()
    , invalidated(true)
{
}

// Destructor
CCPACSWingPositionings::~CCPACSWingPositionings(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWingPositionings::Invalidate(void)
{
    invalidated = true;
    CCPACSWingPositioningIterator it;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSWingPositioning* pos = it->second;
        pos->Invalidate();
    }
}

// Cleanup routine
void CCPACSWingPositionings::Cleanup(void)
{
    CCPACSWingPositioningIterator it;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSWingPositioning* pos = it->second;
        delete pos;
    }
    positionings.clear();
    invalidated = true;
}

CCPACSWingPositionings::CCPACSWingPositioningContainer& CCPACSWingPositionings::GetPositionings()
{
    return positionings;
}

// Returns the positioning matrix for a given section index
CTiglTransformation CCPACSWingPositionings::GetPositioningTransformation(std::string sectionIndex)
{
    Update();
    CCPACSWingPositioningIterator iter = positionings.find(sectionIndex);
        
    // check, if section has positioning definition, if not
    // return Zero-Transformation
    if (iter == positionings.end()){
        CTiglTransformation zeroTrans;
        zeroTrans.SetIdentity();
        return zeroTrans;
    }
       
    return iter->second->GetOuterTransformation();
}

// Update internal positionings structure
void CCPACSWingPositionings::Update(void)
{
    if (!invalidated) {
        return;
    }

    invalidated = false;
        
    // reset all position base points
    CCPACSWingPositioningIterator it;
    
    // diconnect and reset
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSWingPositioning* actPos = it->second;
        actPos->DisconnectChilds();
        actPos->SetInnerPoint(CTiglPoint(0,0,0));
    }
    
    // connect positionings, find roots
    std::vector<CCPACSWingPositioning*> rootNodes;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSWingPositioning* actPos = it->second;
        std::string fromUID = actPos->GetInnerSectionIndex();
        if (fromUID != "") {
            CCPACSWingPositioningIterator pos = positionings.find(fromUID);
            if (pos != positionings.end()) {
                CCPACSWingPositioning* fromPos = pos->second;
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

    for (std::vector<CCPACSWingPositioning*>::iterator it = rootNodes.begin(); it != rootNodes.end(); it++) {
        UpdateNextPositioning(*it, 0);
    }
}

void CCPACSWingPositionings::UpdateNextPositioning(CCPACSWingPositioning* currPos, int depth)
{
    // check for recursive definition
    if (depth > 1000) {
        throw CTiglError("Recursive definition of wing positioning is not allowed");
    }

    if (currPos->GetOuterSectionIndex() == ""){
        throw CTiglError("illegal definition of wing positionings");
    }
        
    // Find all positionings which have the end section of the current positioning
    // defined as their start section.
    std::vector<CCPACSWingPositioning*>::const_iterator it;
    const std::vector<CCPACSWingPositioning*>& childs = currPos->GetChilds();
    for (it = childs.begin(); it != childs.end(); ++it) {
        CCPACSWingPositioning* childPos = *it;
        childPos->SetInnerPoint(currPos->GetOuterPoint());
        UpdateNextPositioning(childPos, depth+1);
    }
}

// Read CPACS positionings element
void CCPACSWingPositionings::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    Cleanup();
    std::vector<CCPACSWingPositioning*> children;
    ReadContainerElement(tixiHandle, xpath, "positioning", 1, children);
    for (std::size_t i = 0; i < children.size(); i++) {
        positionings[children[i]->GetOuterSectionIndex()] = children[i];
    }
    Update();
}

// Write CPACS positionings element
void CCPACSWingPositionings::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    WriteContainerElement(tixiHandle, xpath, "positioning", positionings);
}

} // end namespace tigl
