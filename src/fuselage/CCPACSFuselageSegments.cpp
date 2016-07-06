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
* @brief  Implementation of CPACS fuselage segments handling routines.
*/

#include "CCPACSFuselageSegments.h"

#include "CCPACSFuselageSegment.h"
#include "CTiglError.h"

#include <iostream>
#include <sstream>


namespace tigl
{
CCPACSFuselageSegments::CCPACSFuselageSegments(CCPACSFuselage* parent)
	: generated::CPACSFuselageSegments(parent) {}

// Invalidates internal state
void CCPACSFuselageSegments::Invalidate(void)
{
	for (int i = 1; i <= GetSegmentCount(); i++) {
		GetSegment(i).Invalidate();
	}
}

// Gets a segment by index. 
CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(const int index)
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetSegmentCount()) {
        throw CTiglError("Error: Invalid index value in CCPACSFuselageSegments::GetSegment", TIGL_INDEX_ERROR);
    }
    return *m_segment[idx];
}

// Gets a segment by uid. 
CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(const std::string& segmentUID)
{
    for (std::size_t i = 0; i < m_segment.size(); i++) {
        if (m_segment[i]->GetUID() == segmentUID) {
            return *m_segment[i];
        }
    }
    throw CTiglError("Error: Invalid uid in CCPACSFuselageSegments::GetSegment", TIGL_UID_ERROR);
}

// Gets total segment count
int CCPACSFuselageSegments::GetSegmentCount(void) const
{
    return static_cast<int>(m_segment.size());
}

} // end namespace tigl
