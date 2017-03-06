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

namespace tigl
{

// Constructor
CCPACSWingSegments::CCPACSWingSegments(CCPACSWing* parent)
    : generated::CPACSWingSegments(parent) {}

// Invalidates internal state
void CCPACSWingSegments::Invalidate()
{
    for (std::size_t i = 0; i < m_segment.size(); i++) {
        m_segment[i]->Invalidate();
    }
}

// Gets a segment by index. 
CCPACSWingSegment& CCPACSWingSegments::GetSegment(int index)
{
    index--;
    if (index < 0 || index >= GetSegmentCount()) {
        throw CTiglError("Error: Invalid index value in CCPACSWingSegments::GetSegment", TIGL_INDEX_ERROR);
    }
    return *m_segment[index];
}

// Gets a segment by uid. 
CCPACSWingSegment& CCPACSWingSegments::GetSegment(const std::string& segmentUID)
{
    for (std::size_t i = 0; i < m_segment.size(); i++) {
        if (m_segment[i]->GetUID() == segmentUID) {
            return *m_segment[i];
        }
    }
    throw CTiglError("Error: Invalid uid in CCPACSWingSegments::GetSegment", TIGL_UID_ERROR);
}

// Gets total segment count
int CCPACSWingSegments::GetSegmentCount() const
{
    return static_cast<int>(m_segment.size());
}

} // end namespace tigl
