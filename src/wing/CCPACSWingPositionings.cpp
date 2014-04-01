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
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSWingPositionings::CCPACSWingPositionings(void)
    : positionings()
    , transformations()
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
    for (int i = 1; i <= GetPositioningCount(); i++) {
        GetPositioning(i).Invalidate();
    }
    transformations.clear();
}

// Cleanup routine
void CCPACSWingPositionings::Cleanup(void)
{
    for (CCPACSWingPositioningContainer::size_type i = 0; i < positionings.size(); i++) {
        delete positionings[i];
    }
    positionings.clear();
    transformations.clear();
    invalidated = true;
}

// Gets a positioning by index.
CCPACSWingPositioning& CCPACSWingPositionings::GetPositioning(int index) const
{
    index--;
    if (index < 0 || index >= GetPositioningCount()) {
        throw CTiglError("Error: Invalid index value in CCPACSWingPositionings::GetPositioning", TIGL_INDEX_ERROR);
    }
    return (*(positionings[index]));
}

// Gets total positioning count
int CCPACSWingPositionings::GetPositioningCount(void) const
{
    return static_cast<int>(positionings.size());
}

// Returns the positioning matrix for a given section index
CTiglTransformation CCPACSWingPositionings::GetPositioningTransformation(std::string sectionIndex)
{
    Update();
    CCPACSTransformationMapIterator iter = transformations.find(sectionIndex);
        
    // check, if section has positioning definition, if not
    // return Zero-Transformation
    if (iter == transformations.end()){
        CTiglTransformation zeroTrans;
        zeroTrans.SetIdentity();
        return zeroTrans;
    }
       
    return iter->second;
}

// Update internal positionings structure
void CCPACSWingPositionings::Update(void)
{
    if (!invalidated) {
        return;
    }
        
    // reset all position base points
    for (int ipos = 1; ipos <= GetPositioningCount(); ++ipos) {
        GetPositioning(ipos).SetInnerPoint(CTiglPoint(0,0,0));
    }
        
    for (int ipos = 1; ipos <= GetPositioningCount(); ++ipos) {
        UpdateNextPositioning(ipos, 0);
    }

    invalidated = false;
}

void CCPACSWingPositionings::UpdateNextPositioning(int positioningIndex, int rec_depth)
{
    if (rec_depth > 1000) {
        throw CTiglError("Recursive definition of wing positionings");
    }
        
    CCPACSWingPositioning& currPos = GetPositioning(positioningIndex);

    // Store the transformation of the outer section of the current positioning in a map.
    // Note: Internally we use 0-based indices, but in the CPACS file the indices are 1-based.
    transformations[currPos.GetOuterSectionIndex()] = currPos.GetOuterTransformation();

    // Find all positionings which have the outer section of the current positioning
    // defined as their inner section.
    for (int i = 1; i <= GetPositioningCount(); i++) {
        CCPACSWingPositioning& nextPos = GetPositioning(i);
        if (currPos.GetOuterSectionIndex() == nextPos.GetInnerSectionIndex() && i != positioningIndex) {
            nextPos.SetInnerPoint(currPos.GetOuterPoint());
            UpdateNextPositioning(i, rec_depth + 1);
        }
    }
}

// Read CPACS positionings element
void CCPACSWingPositionings::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
{
    Cleanup();

    ReturnCode    tixiRet;
    int           positioningCount;
    std::string   tempString;
    char*         elementPath;

    /* Get positioning element count */
    tempString  = wingXPath + "/positionings";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath, "positioning", &positioningCount);
    if (tixiRet != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWingPositionings::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all positionings
    for (int i = 1; i <= positioningCount; i++) {
        CCPACSWingPositioning* positioning = new CCPACSWingPositioning();
        positionings.push_back(positioning);

        tempString = wingXPath + "/positionings/positioning[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";
        positioning->ReadCPACS(tixiHandle, xpath.str());
    }

    Update();
}

} // end namespace tigl
