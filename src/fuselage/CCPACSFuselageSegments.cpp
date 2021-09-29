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
    bool segment_follows(const std::unique_ptr<tigl::CCPACSFuselageSegment>& s2, const std::unique_ptr<tigl::CCPACSFuselageSegment>& s1)
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
void CCPACSFuselageSegments::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 1; i <= GetSegmentCount(); i++) {
        GetSegment(i).Invalidate(source);
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

    if (GetSegmentCount() <= 0) {
        return;
    }

    // check order of segments - each segment must start with the element of the previous segment
    bool mustReorderSegments = false;
    std::string prevElementUID = GetSegment(1).GetToElementUID();
    for (int i = 2; i <= GetSegmentCount(); ++i) {
        CCPACSFuselageSegment& segment = GetSegment(i);
        if (prevElementUID != segment.GetFromElementUID()) {
            mustReorderSegments = true;
        }
        prevElementUID = segment.GetToElementUID();
    }

    if (mustReorderSegments) {
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

CTiglFuselageConnection& CCPACSFuselageSegments::GetConnection(const std::string& elementUID) const
{
    for (std::size_t i = 0; i < m_segments.size(); i++) {
        CTiglFuselageConnection& startConnection = m_segments[i]->GetStartConnection();
        CTiglFuselageConnection& endConnection   = m_segments[i]->GetEndConnection();
        if (startConnection.GetSectionElementUID() == elementUID) {
            return startConnection;
        }
        if (endConnection.GetSectionElementUID() == elementUID) {
            return endConnection;
        }
    }

    throw CTiglError("Invalid uid in CCPACSFuselageSegments::GetConnection", TIGL_UID_ERROR);
}

std::vector<std::string> CCPACSFuselageSegments::GetElementUIDsInOrder() const
{
    std::vector<std::string> elementUIDs;
    std::string tempStartUID;
    std::string tempEndUID;
    for (int i = 0; i < m_segments.size(); i++) {
        tempStartUID = m_segments[i]->GetStartSectionElementUID();
        tempEndUID   = m_segments[i]->GetEndSectionElementUID();
        if (std::find(elementUIDs.begin(), elementUIDs.end(), tempStartUID) == elementUIDs.end()) {
            elementUIDs.push_back(tempStartUID);
        }
        if (std::find(elementUIDs.begin(), elementUIDs.end(), tempEndUID) == elementUIDs.end()) {
            elementUIDs.push_back(tempEndUID);
        }
    }
    return elementUIDs;
}

std::map<std::string, CTiglFuselageConnection*> CCPACSFuselageSegments::GetConnectionsMap() const
{
    std::map<std::string, CTiglFuselageConnection*> connectionsMap;

    for (std::size_t i = 0; i < m_segments.size(); i++) {
        CTiglFuselageConnection& startConnection               = m_segments[i]->GetStartConnection();
        CTiglFuselageConnection& endConnection                 = m_segments[i]->GetEndConnection();
        connectionsMap[startConnection.GetSectionElementUID()] = &startConnection; // create or replace
        connectionsMap[endConnection.GetSectionElementUID()]   = &endConnection; // create or replace
    }

    return connectionsMap;
}

} // end namespace tigl
