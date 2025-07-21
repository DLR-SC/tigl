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
* @brief  Implementation of CPACS wing segments handling routines.
*/

#include "CCPACSWingSegments.h"

#include "CTiglError.h"
#include "CCPACSWing.h"
#include "CCPACSEnginePylon.h"
#include "CCPACSWingSegment.h"
#include "sorting.h"
#include "CCPACSWingSectionElement.h"

namespace
{
    bool segment_follows(const tigl::unique_ptr<tigl::CCPACSWingSegment>& s2, const tigl::unique_ptr<tigl::CCPACSWingSegment>& s1)
    {
        if (!s2 || !s1) {
            return false;
        }

        return s2->GetFromElementUID() == s1->GetToElementUID();
    }
}

namespace tigl
{

// Constructor
CCPACSWingSegments::CCPACSWingSegments(CCPACSWing* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSegments(parent, uidMgr)
    , m_parentVariant(parent)
{
}

CCPACSWingSegments::CCPACSWingSegments(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSegments(parent, uidMgr)
    , m_parentVariant(parent)
{
}

void CCPACSWingSegments::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSWingSegments::ReadCPACS(tixiHandle, xpath);

    if (NeedReordering()) {
        LOG(WARNING) << "Wing segments in wrong order! Trying to reorder.";
        ReorderSegments();
    }
}

// Invalidates internal state
void CCPACSWingSegments::Invalidate(const boost::optional<std::string>& source) const
{
    for (const auto& segment : m_segments) {
        segment->Invalidate(source);
    }
}

CCPACSWingSegment& CCPACSWingSegments::AddSegment()
{
    CCPACSWingSegment& result = generated::CPACSWingSegments::AddSegment();
    InvalidateParent();
    return result;
}

void CCPACSWingSegments::RemoveSegment(CCPACSWingSegment& ref)
{
    generated::CPACSWingSegments::RemoveSegment(ref);
    InvalidateParent();
}

void CCPACSWingSegments::InvalidateParent() const
{
    // Invalidate wing or EnginePylon
    if (const auto* parent = GetNextUIDParent()) {
        parent->Invalidate();
    }
}

bool CCPACSWingSegments::NeedReordering() const
{
    if (GetSegmentCount() <= 1) {
        return false;
    }

    bool mustReorderSegments   = false;
    std::string prevElementUID = GetSegment(1).GetToElementUID();
    for (int i = 2; i <= GetSegmentCount(); ++i) {
        const CCPACSWingSegment& segment = GetSegment(i);
        if (prevElementUID != segment.GetFromElementUID()) {
            mustReorderSegments = true;
        }
        prevElementUID = segment.GetToElementUID();
    }
    return mustReorderSegments;
}

void CCPACSWingSegments::ReorderSegments()
{
    try {
        tigl::follow_sort(GetSegments().begin(), GetSegments().end(), segment_follows);
    }
    catch (std::invalid_argument) {
        throw CTiglError("CCPACSWingSegments::ReorderSegments: The wing segments are not continuous.");
    }
}


CCPACSWingSegment& CCPACSWingSegments::GetSegmentFromTo(const std::string& fromElemUID,
                                                                const std::string toElementUID)
{

    for (int i = 0; i < m_segments.size(); i++) {
        if (m_segments[i]->GetFromElementUID() == fromElemUID && m_segments[i]->GetToElementUID() == toElementUID) {
            return GetSegment(i + 1);
        }
    }

    throw CTiglError("Segment with the given from and to UID not found", TIGL_UID_ERROR);
}

std::vector<std::string> CCPACSWingSegments::GetElementUIDsInOrder() const
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


CCPACSWingSegment& CCPACSWingSegments::SplitSegment(const std::string& segmentUID,
                                                    const std::string& splitterElementUID)
{
    CCPACSWingSegment& segment = this->GetSegment(segmentUID);
    CCPACSWingSectionElement& splitterElement =
        GetUIDManager().ResolveObject<CCPACSWingSectionElement>(splitterElementUID);

    // create a additional segment
    CCPACSWingSegment& additionalSegment = this->AddSegment();
    std::string additionalSegmentUID     = GetUIDManager().MakeUIDUnique(segment.GetUID() + "bis");
    additionalSegment.SetUID(additionalSegmentUID);
    additionalSegment.SetName(additionalSegmentUID);

    // set the segment from splitter element to the old end
    additionalSegment.SetFromElementUID(splitterElement.GetUID());
    additionalSegment.SetToElementUID(segment.GetToElementUID());

    // reset the old segment to end at the splitter
    segment.SetToElementUID(splitterElement.GetUID());

    Invalidate();
    // Reordering is necessary: After splitting a segment and adding another one, the order is not correct
    // Without reordering, this results in a wrong shape visible in the TiGLViewer
    ReorderSegments();

    return additionalSegment;
}

} // end namespace tigl
