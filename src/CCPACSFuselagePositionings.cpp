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

namespace tigl {

    // Constructor
    CCPACSFuselagePositionings::CCPACSFuselagePositionings(void)
        : positionings()
        , transformations()
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
        for (int i = 1; i <= GetPositioningCount(); i++)
        {
            GetPositioning(i).Invalidate();
        }
        transformations.clear();
    }

    // Cleanup routine
    void CCPACSFuselagePositionings::Cleanup(void)
    {
        for (CCPACSFuselagePositioningContainer::size_type i = 0; i < positionings.size(); i++)
        {
            delete positionings[i];
        }
        positionings.clear();
        transformations.clear();
        invalidated = true;
    }

    // Gets a positioning by index.
    CCPACSFuselagePositioning& CCPACSFuselagePositionings::GetPositioning(int index) const
    {
        index--;
        if (index < 0 || index >= GetPositioningCount())
            throw CTiglError("Error: Invalid index value in CCPACSFuselagePositionings::GetPositioning", TIGL_INDEX_ERROR);
        return (*(positionings[index]));
    }

    // Gets total positioning count
    int CCPACSFuselagePositionings::GetPositioningCount(void) const
    {
        return static_cast<int>(positionings.size());
    }

    // Returns the positioning matrix for a given section index
    CTiglTransformation CCPACSFuselagePositionings::GetPositioningTransformation(std::string sectionIndex)
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
    void CCPACSFuselagePositionings::Update(void)
    {
        if (!invalidated)
            return;

        invalidated = false;
        
        // test if positionings are defined recursively
        for (int ipos = 1; ipos <= GetPositioningCount(); ++ipos){
            TestRecursiveDefinition(ipos,0);
        }
        
        // reset all position base points
        for (int ipos = 1; ipos <= GetPositioningCount(); ++ipos){
            GetPositioning(ipos).SetStartPoint(CTiglPoint(0,0,0));
        }
        
        // find out root nodes
        std::vector<int> rootNodes;
        for (int ipos = 1; ipos <= GetPositioningCount(); ++ipos){
            CCPACSFuselagePositioning& actPos = GetPositioning(ipos);
            if(actPos.GetStartSectionIndex() == "")
                rootNodes.push_back(ipos);
        }
        
        for (std::vector<int>::iterator it = rootNodes.begin(); it != rootNodes.end(); it++)
            UpdateNextPositioning(*it);
    }
    
    void CCPACSFuselagePositionings::TestRecursiveDefinition(int positioningIndex, int rec_depth) {
        if (rec_depth > 1000) {
            throw CTiglError("Recursive definition of fuselage positionings");
        }
        
        CCPACSFuselagePositioning& currPos = GetPositioning(positioningIndex);
        for (int i = 1; i <= GetPositioningCount(); i++)
        {
            CCPACSFuselagePositioning& nextPos = GetPositioning(i);
            if (currPos.GetEndSectionIndex() == nextPos.GetStartSectionIndex() && positioningIndex != i)
            {
                TestRecursiveDefinition(i, rec_depth + 1);
            }
        }
    }


    // @todo: This code is only working, if the first positions is basis for everything else
    // and if its fromSectionUID is empty in CPACS. We should completely rewrite this code
    void CCPACSFuselagePositionings::UpdateNextPositioning(int positioningIndex)
    {
        CCPACSFuselagePositioning& currPos = GetPositioning(positioningIndex);

        // Store the transformation of the end section of the current positioning in a map.
        // Note: Internally we use 0-based indices, but in the CPACS file the indices are 1-based.
        transformations[currPos.GetEndSectionIndex()] = currPos.GetEndTransformation();

        if(currPos.GetEndSectionIndex() == ""){
            throw CTiglError("illegal definition of fuselage positionings");
        }
        
        // Find all positionings which have the end section of the current positioning
        // defined as their start section.
        for (int i = 1; i <= GetPositioningCount(); i++)
        {
            CCPACSFuselagePositioning& nextPos = GetPositioning(i);
            if (currPos.GetEndSectionIndex() == nextPos.GetStartSectionIndex() && positioningIndex != i)
            {
                nextPos.SetStartPoint(currPos.GetEndPoint());
                UpdateNextPositioning(i);
            }
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
        if (tixiRet != SUCCESS)
            throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSFuselagePositionings::ReadCPACS", TIGL_XML_ERROR);

        // Loop over all positionings
        for (int i = 1; i <= positioningCount; i++)
        {
            CCPACSFuselagePositioning* positioning = new CCPACSFuselagePositioning();
            positionings.push_back(positioning);

            tempString = fuselageXPath + "/positionings/positioning[";
            std::ostringstream xpath;
            xpath << tempString << i << "]";
            positioning->ReadCPACS(tixiHandle, xpath.str());
        }

        Update();
    }

} // end namespace tigl
