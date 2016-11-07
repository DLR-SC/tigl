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
#include "IOHelper.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingSparSegments::CCPACSWingSparSegments(CCPACSWingSpars& parent)
: parent(parent)
{
    Cleanup();
}

CCPACSWingSparSegments::~CCPACSWingSparSegments(void)
{
    Cleanup();
}

void CCPACSWingSparSegments::Invalidate(void)
{
    CCPACSWingSparSegmentContainer::iterator it;
    for (it = sparSegments.begin(); it != sparSegments.end(); ++it) {
        (*it)->Invalidate();
    }
}

void CCPACSWingSparSegments::Cleanup(void)
{
    CCPACSWingSparSegmentContainer::iterator it;
    for (it = sparSegments.begin(); it != sparSegments.end(); ++it) {
        delete *it;
    }
    sparSegments.clear();
}

void CCPACSWingSparSegments::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    Cleanup();
    ReadContainerElement(tixiHandle, xpath, "sparSegment", 1, sparSegments, &parent);
}

void CCPACSWingSparSegments::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    WriteContainerElement(tixiHandle, xpath, "sparSegment", sparSegments);
}

int CCPACSWingSparSegments::GetSparSegmentCount() const
{
    return static_cast<int>(sparSegments.size());
}

CCPACSWingSparSegment& CCPACSWingSparSegments::GetSparSegment(int index) const
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetSparSegmentCount()) {
        LOG(ERROR) << "Invalid index value";
        throw CTiglError("Error: Invalid index value in CCPACSWingSparSegments::getSparSegment", TIGL_INDEX_ERROR);
    }
    return (*(sparSegments[idx]));
}

CCPACSWingSparSegment& CCPACSWingSparSegments::GetSparSegment(const std::string& uid) const
{
    CCPACSWingSparSegmentContainer::const_iterator it;
    for (it = sparSegments.begin(); it != sparSegments.end(); ++it) {
        CCPACSWingSparSegment* sparSegment = *it;
        if (sparSegment->GetUID() == uid) {
            return *sparSegment;
        }
    }
    std::string referenceUID = parent.GetStructure().GetWingStructureReference().GetUID();
    LOG(ERROR) << "Spar Segment \"" << uid << "\" not found in component segment or trailing edge device with UID \"" << referenceUID << "\"";
    throw CTiglError("Error: Spar Segment \"" + uid + "\" not found in component segment or trailing edge device with UID \"" + referenceUID + "\". Please check the CPACS document!", TIGL_ERROR);
}

} // end namespace tigl
