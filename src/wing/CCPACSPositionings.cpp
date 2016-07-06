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

#include "CCPACSPositionings.h"
#include "TixiSaveExt.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSPositionings::CCPACSPositionings()
    : invalidated(true)
{
}

// Destructor
CCPACSPositionings::~CCPACSPositionings()
{
    Cleanup();
}

// Invalidates internal state
void CCPACSPositionings::Invalidate()
{
    invalidated = true;
    CCPACSPositioningIterator it;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSPositioning* pos = it->second;
        pos->Invalidate();
    }
}

// Cleanup routine
void CCPACSPositionings::Cleanup()
{
    invalidated = true;
}

CCPACSPositionings::CCPACSPositioningContainer& CCPACSPositionings::GetPositionings()
{
    return positionings;
}

// Returns the positioning matrix for a given section index
CTiglTransformation CCPACSPositionings::GetPositioningTransformation(std::string sectionIndex)
{
    Update();
    CCPACSPositioningIterator iter = positionings.find(sectionIndex);
        
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
void CCPACSPositionings::Update()
{
    if (!invalidated) {
        return;
    }

    invalidated = false;
        
    // reset all position base points
    CCPACSPositioningIterator it;
    
    // diconnect and reset
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSPositioning* actPos = it->second;
        actPos->DisconnectChilds();
        actPos->SetInnerPoint(CTiglPoint(0,0,0));
    }
    
    // connect positionings, find roots
    std::vector<CCPACSPositioning*> rootNodes;
    for (it = positionings.begin(); it != positionings.end(); ++it) {
        CCPACSPositioning* actPos = it->second;
        std::string fromUID = actPos->GetInnerSectionIndex();
        if (fromUID != "") {
            CCPACSPositioningIterator pos = positionings.find(fromUID);
            if (pos != positionings.end()) {
                CCPACSPositioning* fromPos = pos->second;
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

    for (std::vector<CCPACSPositioning*>::iterator it = rootNodes.begin(); it != rootNodes.end(); it++) {
        UpdateNextPositioning(*it, 0);
    }
}

void CCPACSPositionings::UpdateNextPositioning(CCPACSPositioning* currPos, int depth)
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
    std::vector<CCPACSPositioning*>::const_iterator it;
    const std::vector<CCPACSPositioning*>& childs = currPos->GetChilds();
    for (it = childs.begin(); it != childs.end(); ++it) {
        CCPACSPositioning* childPos = *it;
        childPos->SetInnerPoint(currPos->GetOuterPoint());
        UpdateNextPositioning(childPos, depth+1);
    }
}

// Read CPACS positionings element
void CCPACSPositionings::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
{
    Cleanup();
    generated::CPACSPositioning::ReadCPACS(tixiHandle, wingXPath);
    Update();
}

} // end namespace tigl
