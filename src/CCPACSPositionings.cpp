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
    for (std::vector<unique_ptr<CCPACSPositioning>>::iterator it = m_positioning.begin(); it != m_positioning.end(); ++it) {
        CCPACSPositioning* actPos = it->get();
        actPos->Invalidate();
    }
}

// Cleanup routine
void CCPACSPositionings::Cleanup()
{
    invalidated = true;
}

// Returns the positioning matrix for a given section index
CTiglTransformation CCPACSPositionings::GetPositioningTransformation(std::string sectionIndex)
{
    Update();
    for (const auto& p : m_positioning) {
        if (p->GetOuterSectionIndex() == sectionIndex) {
            return p->GetOuterTransformation();
        }
    }
    return CTiglTransformation();
}

// Update internal positionings structure
void CCPACSPositionings::Update()
{
    if (!invalidated) {
        return;
    }

    invalidated = false;

    // reset all position base points

    // diconnect and reset
    for (std::vector<unique_ptr<CCPACSPositioning>>::iterator it = m_positioning.begin(); it != m_positioning.end(); ++it) {
        CCPACSPositioning* actPos = it->get();
        actPos->DisconnectChilds();
        actPos->SetInnerPoint(CTiglPoint(0,0,0));
    }
    
    // connect positionings, find roots
    std::vector<CCPACSPositioning*> rootNodes;
    for (std::vector<unique_ptr<CCPACSPositioning>>::iterator it = m_positioning.begin(); it != m_positioning.end(); ++it) {
        CCPACSPositioning* actPos = it->get();
        // fromSectionUID element may be present but empty
        if (actPos->HasFromSectionUID() && !actPos->GetFromSectionUID().empty()) {
            const std::string fromUID = actPos->GetFromSectionUID();
            const auto pos = std::find_if(std::begin(m_positioning), std::end(m_positioning), [&](const unique_ptr<CCPACSPositioning>& p) {
                return p->GetOuterSectionIndex() == fromUID;
            });
            if (pos != std::end(m_positioning)) {
                (*pos)->ConnectChildPositioning(actPos);
            } else {
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
    generated::CPACSPositionings::ReadCPACS(tixiHandle, wingXPath);
    Update();
}

} // end namespace tigl
