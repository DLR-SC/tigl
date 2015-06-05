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
* @brief  Implementation of CPACS wing segments handling routines.
*/

#include <iostream>
#include <sstream>
#include <exception>

#include "CCPACSWingSegments.h"

#include "CCPACSWingSegment.h"
#include "CTiglError.h"
// [[CAS_AES]] include helper routines for save method
#include "TixiSaveExt.h"

namespace tigl 
{

// Constructor
CCPACSWingSegments::CCPACSWingSegments(CCPACSWing* aWing)
    : segments()
    , wing(aWing)
{
}

// Destructor
CCPACSWingSegments::~CCPACSWingSegments(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWingSegments::Invalidate(void)
{
    for (int i = 1; i <= GetSegmentCount(); i++) {
        GetSegment(i).Invalidate();
    }
}

// Cleanup routine
void CCPACSWingSegments::Cleanup(void)
{
    for (CCPACSWingSegmentContainer::size_type i = 0; i < segments.size(); i++) {
        delete segments[i];
    }
    segments.clear();
}

// Gets a segment by index. 
CCPACSWingSegment & CCPACSWingSegments::GetSegment(const int index)
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetSegmentCount()) {
        throw CTiglError("Error: Invalid index value in CCPACSWingSegments::GetSegment", TIGL_INDEX_ERROR);
    }
    return (CCPACSWingSegment &) (*(segments[idx]));
}

// Gets a segment by uid. 
CCPACSWingSegment & CCPACSWingSegments::GetSegment(const std::string& segmentUID)
{
    for (CCPACSWingSegmentContainer::size_type i = 0; i < segments.size(); i++) {
        if (segments[i]->GetUID() == segmentUID) {
            return (CCPACSWingSegment &) (*(segments[i]));
        }
    }
    throw CTiglError("Error: Invalid uid in CCPACSWingSegments::GetSegment", TIGL_UID_ERROR);
}

// Gets total segment count
int CCPACSWingSegments::GetSegmentCount(void) const
{
    return static_cast<int>(segments.size());
}

// Read CPACS segments element
void CCPACSWingSegments::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
{
    Cleanup();

    ReturnCode    tixiRet;
    int           segmentCount;
    std::string   tempString;
    char*         elementPath;

    /* Get segment element count */
    tempString  = wingXPath + "/segments";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath, "segment", &segmentCount);
    if (tixiRet != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWingSegments::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all segments
    for (int i = 1; i <= segmentCount; i++) {
        CCPACSWingSegment* segment = new CCPACSWingSegment(wing, i);
        segments.push_back(segment);

        tempString = wingXPath + "/segments/segment[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";
        segment->ReadCPACS(tixiHandle, xpath.str());
    }

}

// [[CAS_AES]] Write CPACS segments element
void CCPACSWingSegments::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
{
    std::string elementPath;
    std::string xpath;
    ReturnCode    tixiRet;
    int           segmentCount, test;

    elementPath = wingXPath + "/segments";
    TixiSaveExt::TixiSaveElement(tixiHandle, wingXPath.c_str(), "segments");
    
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "segment", &test);
    segmentCount = GetSegmentCount();

    for (int i = 1; i <= segmentCount; i++) {
        std::stringstream ss;
        ss << elementPath << "/segment[" << i << "]";
        xpath = ss.str();
        CCPACSWingSegment& segment = GetSegment(i);
        if ((tixiRet = tixiCheckElement(tixiHandle, xpath.c_str())) == ELEMENT_NOT_FOUND) {
            if ((tixiRet = tixiCreateElement(tixiHandle, elementPath.c_str(), "segment")) != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSWingElements::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        segment.WriteCPACS(tixiHandle, xpath);
    }

    for (int i = segmentCount+1; i <= test; i++) {
        std::stringstream ss;
        ss << elementPath << "/segment[" << segmentCount+1 << "]";
        xpath = ss.str();
        tixiRemoveElement(tixiHandle, xpath.c_str());
    }
}

} // end namespace tigl
