/* 
* Copyright (C) 2016 Airbus Defence and Space
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

#include "CCPACSWingSparSegments.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingSpars.h"
#include "CCPACSWingSparSegment.h"
#include "CTiglError.h"
#include "CTiglLogging.h"

namespace tigl
{

CCPACSWingSparSegments::CCPACSWingSparSegments(CCPACSWingSpars* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSparSegments(parent, uidMgr) {}

void CCPACSWingSparSegments::Invalidate()
{
    for (std::vector<unique_ptr<CCPACSWingSparSegment> >::iterator it = m_sparSegments.begin(); it != m_sparSegments.end(); ++it) {
        (*it)->Invalidate();
    }
}

int CCPACSWingSparSegments::GetSparSegmentCount() const
{
    return static_cast<int>(m_sparSegments.size());
}

CCPACSWingSparSegment& CCPACSWingSparSegments::GetSparSegment(int index) const
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetSparSegmentCount()) {
        LOG(ERROR) << "Invalid index value";
        throw CTiglError("Invalid index value in CCPACSWingSparSegments::getSparSegment", TIGL_INDEX_ERROR);
    }
    return (*(m_sparSegments[idx]));
}

CCPACSWingSparSegment& CCPACSWingSparSegments::GetSparSegment(const std::string& uid) const
{
    for (std::vector<unique_ptr<CCPACSWingSparSegment> >::const_iterator it = m_sparSegments.begin(); it != m_sparSegments.end(); ++it) {
        CCPACSWingSparSegment& sparSegment = **it;
        if (sparSegment.GetUID() == uid) {
            return sparSegment;
        }
    }
    const std::string& referenceUID = CTiglWingStructureReference(*GetParent()->GetParent()).GetUID();
    LOG(ERROR) << "Spar Segment \"" << uid << "\" not found in component segment or trailing edge device with UID \"" << referenceUID << "\"";
    throw CTiglError("Spar Segment \"" + uid + "\" not found in component segment or trailing edge device with UID \"" + referenceUID + "\". Please check the CPACS document!", TIGL_ERROR);
}
} // end namespace tigl
