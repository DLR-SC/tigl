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
#include "IOHelper.h"
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
void CCPACSWingSegments::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    Cleanup();
    ReadContainerElement(tixiHandle, xpath, "segment", 1, segments, wing, ContainerElementIndex);
}

// Write CPACS segments element
void CCPACSWingSegments::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    WriteContainerElement(tixiHandle, xpath, "segment", segments);
}

} // end namespace tigl
