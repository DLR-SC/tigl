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
#include "CCPACSWingSegment.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSpars.h"
#include "CTiglError.h"
#include "CPACSWingRibPoint.h"
#include "CPACSEtaXsiPoint.h"
#include "CTiglUIDManager.h"


namespace tigl
{

CCPACSWingSparPosition::CCPACSWingSparPosition(CCPACSWingSparPositions* sparPositions, CTiglUIDManager* uidMgr)
: generated::CPACSSparPosition(sparPositions, uidMgr) {}

bool CCPACSWingSparPosition::isOnInnerSectionElement() const
{
    if (GetSparPositionEtaXsi_choice2()) {
        return GetEta() < 1.E-6;
    }
    else {
        return false;
    }
}

bool CCPACSWingSparPosition::isOnOuterSectionElement() const
{
    if (GetSparPositionEtaXsi_choice2()) {
        return (1 - GetEta()) < 1.E-6;
    }
    else {
        return false;
    }
}

bool CCPACSWingSparPosition::isOnSectionElement() const
{
    return isOnInnerSectionElement() || isOnOuterSectionElement();
}

bool CCPACSWingSparPosition::isOnRib() const
{
    return m_sparPositionRib_choice1 ? true : false;
}

const std::string& CCPACSWingSparPosition::GetReferenceUID() const
{
    if (m_sparPositionRib_choice1) {
        return m_sparPositionRib_choice1->GetRibDefinitionUID();
    }
    else if (m_sparPositionEtaXsi_choice2){
        return m_sparPositionEtaXsi_choice2->GetReferenceUID();
    }
    else {
        throw CTiglError("Invalid choice type");
    }
}


double CCPACSWingSparPosition::GetEta() const
{
    if (!m_sparPositionEtaXsi_choice2) {
        throw CTiglError("SparPosition is not defined via eta/xsi. Please check InputType first before calling CCPACSWingSparPosition::GetXsi()");
    }
    return m_sparPositionEtaXsi_choice2->GetEta();
}

double CCPACSWingSparPosition::GetXsi() const
{
    if (m_sparPositionRib_choice1) {
        return m_sparPositionRib_choice1->GetXsi();
    }
    else if (m_sparPositionEtaXsi_choice2) {
        return m_sparPositionEtaXsi_choice2->GetXsi();
    }
    throw CTiglError("Invalid spar position type");
}

const generated::CPACSEtaXsiPoint &CCPACSWingSparPosition::GetEtaXsiPoint() const
{
    if (!GetSparPositionEtaXsi_choice2()) {
        throw CTiglError("No EtaXsiPoint definied in SparPosition '" + GetUID() + "'");
    }
    
    return GetSparPositionEtaXsi_choice2().value();
}

const generated::CPACSWingRibPoint& CCPACSWingSparPosition::GetRibPoint() const
{
    if (!GetSparPositionRib_choice1()) {
        throw CTiglError("No RibPoint definied in SparPosition '" + GetUID() + "'");
    }

    return GetSparPositionRib_choice1().value();
}

int WingRibPointGetRibNumber(const generated::CPACSWingRibPoint& ribPoint)
{
    return ribPoint.GetRibNumber().value_or(1);
}

std::string WingSparPosGetElementUID(const CCPACSWingSparPosition & pos)
{
    if (!pos.isOnSectionElement()) {
        throw CTiglError("Spar positioning is not on section element");
    }

    const CTiglUIDManager::TypedPtr tp = pos.GetUIDManager().ResolveObject(pos.GetEtaXsiPoint().GetReferenceUID());
    if (tp.type == &typeid(CCPACSWingComponentSegment)) {
        const CCPACSWingComponentSegment& cs = *reinterpret_cast<CCPACSWingComponentSegment*>(tp.ptr);
        const SegmentList& segments = cs.GetSegmentList();
        if (pos.isOnInnerSectionElement()) {
            return segments.front()->GetInnerSectionElementUID();
        }
        else {
            return segments.back()->GetOuterSectionElementUID();
        }
    }
    else if (tp.type == &typeid(CCPACSWingSegment)) {
        const CCPACSWingSegment& segment = *reinterpret_cast<CCPACSWingSegment*>(tp.ptr);
        if (pos.isOnInnerSectionElement()) {
            return segment.GetInnerSectionElementUID();
        }
        else {
            return segment.GetOuterSectionElementUID();
        }
    }
    else {
        throw CTiglError("'" + pos.GetEtaXsiPoint().GetReferenceUID() + "' in not a wing segment or a component segment.");
    }
}

} // end namespace tigl
