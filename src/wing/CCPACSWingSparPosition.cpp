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

#include "CCPACSWingSparPosition.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSpars.h"
#include "CTiglError.h"


namespace tigl
{

CCPACSWingSparPosition::CCPACSWingSparPosition(CCPACSWingSparPositions* sparPositions, CTiglUIDManager* uidMgr)
: generated::CPACSSparPosition(sparPositions, uidMgr) {}

bool CCPACSWingSparPosition::isOnInnerSectionElement() const
{
    return GetSparPoint().GetEta() < 1.E-6;
}

bool CCPACSWingSparPosition::isOnOuterSectionElement() const
{
    return (1 - GetSparPoint().GetEta()) < 1.E-6;
}

bool CCPACSWingSparPosition::isOnSectionElement() const
{
    return isOnInnerSectionElement() || isOnOuterSectionElement();
}

std::string CCPACSWingSparPosition::GetElementUID() const
{
    CTiglWingStructureReference wsr(*m_parent->GetParent()->GetParent());
    if (GetSparPoint().GetReferenceUID() == wsr.GetUID()) {
        if (isOnInnerSectionElement() && wsr.GetType() == CTiglWingStructureReference::ComponentSegmentType)
            return wsr.GetWingComponentSegment().GetFromElementUID();
        if (isOnOuterSectionElement() && wsr.GetType() == CTiglWingStructureReference::ComponentSegmentType)
            return wsr.GetWingComponentSegment().GetToElementUID();
    }
    else if (wsr.GetType() == CTiglWingStructureReference::ComponentSegmentType &&
        wsr.GetWingComponentSegment().IsSegmentContained(wsr.GetWing().GetSegment(GetSparPoint().GetReferenceUID()))) {
        if (isOnInnerSectionElement())
            return wsr.GetWing().GetSegment(GetSparPoint().GetReferenceUID()).GetFromElementUID();
        else if (isOnOuterSectionElement())
            return wsr.GetWing().GetSegment(GetSparPoint().GetReferenceUID()).GetToElementUID();
    }

    throw CTiglError("Requested section element UID was not found for spar position '" + m_uID + "'.");
}

} // end namespace tigl
