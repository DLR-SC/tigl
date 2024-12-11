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
CCPACSPositionings::CCPACSPositionings(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPositionings(parent, uidMgr), invalidated(true)
{
}

CCPACSPositionings::CCPACSPositionings(CCPACSFuselage* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPositionings(parent, uidMgr), invalidated(true)
{
}

CCPACSPositionings::CCPACSPositionings(CCPACSDuct* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPositionings(parent, uidMgr), invalidated(true)
{
}

CCPACSPositionings::CCPACSPositionings(CCPACSWing* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPositionings(parent, uidMgr), invalidated(true)
{
}

// Destructor
CCPACSPositionings::~CCPACSPositionings()
{
    Cleanup();
}

// Cleanup routine
void CCPACSPositionings::Cleanup()
{
    invalidated = true;
}

// Returns the positioning matrix for a given section index
CTiglTransformation CCPACSPositionings::GetPositioningTransformation(const std::string& sectionIndex)
{
    Update();
    for (auto& p : m_positionings) {
        if (p->GetToSectionUID() == sectionIndex) {
            return p->GetToTransformation();
        }
    }
    return CTiglTransformation();
}

//TODO write test
int CCPACSPositionings::GetPositioningCount() const
{
    return CPACSPositionings::m_positionings.size();
}

//TODO write test
CCPACSPositioning& CCPACSPositionings::GetPositioning(int index)
{
    {
        index --;
        if (index < 0 || index >= GetPositioningCount()) {
            throw CTiglError("Invalid index in CCPACSPositionings::GetPositioning", TIGL_INDEX_ERROR);
        }
        return *m_positionings[index];
    }
}

namespace
{
void UpdateNextPositioning(CCPACSPositioning* currPos, int depth)
{
    // check for recursive definition
    if (depth > 1000) {
        throw CTiglError("Recursive definition of positioning is not allowed");
    }

        if (currPos->GetToSectionUID().empty()){
        throw CTiglError("illegal definition of positionings");
    }

    // Find all positionings which have the end section of the current positioning
    // defined as their start section.
    const std::vector<CCPACSPositioning*>& children = currPos->GetDependentPositionings();
    for (std::vector<CCPACSPositioning*>::const_iterator it = children.begin(); it != children.end(); ++it) {
        CCPACSPositioning* childPos = *it;
        childPos->SetFromPoint(currPos->GetToPoint());
        UpdateNextPositioning(childPos, depth + 1);
    }
}
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
    for (auto& pos : m_positionings) {
        pos->DisconnectDependentPositionings();
        pos->SetFromPoint(CTiglPoint(0,0,0));
    }

    // connect positionings, find roots
    std::vector<CCPACSPositioning*> rootNodes;
    for (auto& pos : m_positionings) {
        // fromSectionUID element may be present but empty
        if (pos->GetFromSectionUID() && !pos->GetFromSectionUID()->empty()) {
            const std::string& fromUID = *pos->GetFromSectionUID();
            bool found = false;
            for (auto& pos2 : m_positionings) {
                if (pos2->GetToSectionUID() == fromUID) {
                    pos2->AddDependentPositioning(pos.get());
                    found = true;
                    break;
                }
            }
            if (!found) {
                // invalid from UID
                throw CTiglError("Positioning fromSectionUID " + fromUID + " does not exist");
            }
        }
        else {
            rootNodes.push_back(pos.get());
        }
    }

    for (std::vector<CCPACSPositioning*>::iterator it = rootNodes.begin(); it != rootNodes.end(); it++) {
        UpdateNextPositioning(*it, 0);
    }
}

// Read CPACS positionings element
void CCPACSPositionings::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& wingXPath)
{
    Cleanup();
    generated::CPACSPositionings::ReadCPACS(tixiHandle, wingXPath);
    Update();
}

} // end namespace tigl
