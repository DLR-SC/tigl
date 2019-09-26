/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
#include "CTiglUIDManager.h"
#include "CCPACSFuselageSectionElement.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include "CCPACSFuselageSegment.h"
#include "CCPACSFuselage.h"
#include "CTiglError.h"
#include "sorting.h"
#include "CTiglLogging.h"

namespace
{
    bool segment_follows(const tigl::unique_ptr<tigl::CCPACSFuselageSegment>& s2, const tigl::unique_ptr<tigl::CCPACSFuselageSegment>& s1)
    {
        if (!s2 || !s1) {
            return false;
        }

        return s2->GetFromElementUID() == s1->GetToElementUID();
    }
}

namespace tigl
{
CCPACSFuselageSegments::CCPACSFuselageSegments(CCPACSFuselage* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSegments(parent, uidMgr) {}

// Invalidates internal state
void CCPACSFuselageSegments::Invalidate()
{
    for (int i = 1; i <= GetSegmentCount(); i++) {
        GetSegment(i).Invalidate();
    }

    if ( NeedReordering() ){
        try { // we use a try-catch to not rise two time a exception if the reordering occurs during the first cpacs parsing
            ReorderSegments();
        } catch (  const CTiglError& err) {
            LOG(ERROR) << err.what();
        }

    }
}

// Gets a segment by index. 
CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(int index)
{
    return const_cast<CCPACSFuselageSegment&>(static_cast<const CCPACSFuselageSegments&>(*this).GetSegment(index));
}

const CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(int index) const
{
    index--;
    if (index < 0 || index >= GetSegmentCount()) {
        throw CTiglError("Invalid index value in CCPACSFuselageSegments::GetSegment", TIGL_INDEX_ERROR);
    }
    return *m_segments[index];
}

// Gets a segment by uid. 
CCPACSFuselageSegment & CCPACSFuselageSegments::GetSegment(const std::string& segmentUID)
{
    for (std::size_t i = 0; i < m_segments.size(); i++) {
        if (m_segments[i]->GetUID() == segmentUID) {
            return *m_segments[i];
        }
    }
    throw CTiglError("Invalid uid in CCPACSFuselageSegments::GetSegment", TIGL_UID_ERROR);
}

// Gets total segment count
int CCPACSFuselageSegments::GetSegmentCount() const
{
    return static_cast<int>(m_segments.size());
}

void tigl::CCPACSFuselageSegments::ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath)
{
    tigl::generated::CPACSFuselageSegments::ReadCPACS(tixiHandle, xpath);

    if (NeedReordering()) {
        LOG(WARNING) << "Fuselage segments in wrong order! Trying to reorder.";
        ReorderSegments();
    }

}

void CCPACSFuselageSegments::ReorderSegments()
{
    try {
        tigl::follow_sort(GetSegments().begin(), GetSegments().end(), segment_follows);
    } catch (std::invalid_argument) {
        throw CTiglError("Fuselage segments not continous.");
    }
}

bool CCPACSFuselageSegments::NeedReordering()
{
    if ( GetSegmentCount() <= 1 ) {
        return false;
    }

    bool mustReorderSegments = false;
    std::string prevElementUID = GetSegment(1).GetToElementUID();
    for (int i = 2; i <= GetSegmentCount(); ++i) {
        CCPACSFuselageSegment& segment = GetSegment(i);
        if (prevElementUID != segment.GetFromElementUID()) {
            mustReorderSegments = true;
        }
        prevElementUID = segment.GetToElementUID();
    }
    return mustReorderSegments;
}

std::vector<std::string> CCPACSFuselageSegments::GetElementUIDsInOrder() const
{
    std::vector<std::string> elementUIDs;
    std::string tempStartUID;
    std::string tempEndUID;
    for (int i = 0; i < m_segments.size(); i++) {
        tempStartUID = m_segments[i]->GetFromElementUID();
        tempEndUID   = m_segments[i]->GetToElementUID();
        if (std::find(elementUIDs.begin(), elementUIDs.end(), tempStartUID) == elementUIDs.end()) {
            elementUIDs.push_back(tempStartUID);
        }
        if (std::find(elementUIDs.begin(), elementUIDs.end(), tempEndUID) == elementUIDs.end()) {
            elementUIDs.push_back(tempEndUID);
        }
    }
    return elementUIDs;
}


CCPACSFuselageSegment& CCPACSFuselageSegments::SplitSegment(const std::string& segmentUID, const std::string& splitterElementUID)
{
    CCPACSFuselageSegment& segment = this->GetSegment(segmentUID);
    CCPACSFuselageSectionElement& splitterElement = GetUIDManager().ResolveObject<CCPACSFuselageSectionElement>(splitterElementUID);

    // create a additional segment
    CCPACSFuselageSegment& additionalSegment =  this->AddSegment();
    std::string additionalSegmentUID = GetUIDManager().MakeUIDUnique(segment.GetUID() + "bis" );
    additionalSegment.SetUID(additionalSegmentUID);
    additionalSegment.SetName(additionalSegmentUID);

    // set the segment from splitter element to the old end
    additionalSegment.SetFromElementUID(splitterElement.GetUID());
    additionalSegment.SetToElementUID(segment.GetToElementUID());

    // reset the old segment to end at the splitter
    segment.SetToElementUID(splitterElement.GetUID());

    Invalidate();

    return additionalSegment;
}

CCPACSFuselageSegment& CCPACSFuselageSegments::GetSegmentFromTo(const std::string &fromElemUID,
                                                                const std::string toElementUID)
{

    for (int i = 0; i < m_segments.size(); i++) {
        if ( m_segments[i]->GetFromElementUID() == fromElemUID && m_segments[i]->GetToElementUID() == toElementUID ) {
            return GetSegment(i+1);
        }

    }

    throw  CTiglError("Segment with the given from and to UID not found", TIGL_UID_ERROR);

}

} // end namespace tigl
