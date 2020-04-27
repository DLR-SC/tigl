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
#include "CCPACSWingRibsPositioning.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "stringtools.h"

namespace tigl
{

namespace {
    bool isUid(const std::string& s) {
        return (!s.empty() && to_lower(s) != "leadingedge" && to_lower(s) != "trailingedge");
    }
}

CCPACSWingRibsPositioning::CCPACSWingRibsPositioning(CCPACSWingRibsDefinition* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingRibsPositioning(parent, uidMgr) {}

CCPACSWingRibsPositioning::~CCPACSWingRibsPositioning()
{
    if (m_uidMgr) {
        if (isUid(m_ribStart)) m_uidMgr->TryUnregisterReference(m_ribStart, *this);
        if (isUid(m_ribEnd))   m_uidMgr->TryUnregisterReference(m_ribEnd,   *this);
    }
}

void CCPACSWingRibsPositioning::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSWingRibsPositioning::ReadCPACS(tixiHandle, xpath);
    if (m_uidMgr) {
        if (isUid(m_ribStart)) {
            m_uidMgr->RegisterReference(m_ribStart, *this);
        }
        if (isUid(m_ribEnd)) {
            m_uidMgr->RegisterReference(m_ribEnd, *this);
        }
    }
}

CCPACSWingRibsPositioning::StartEndDefinitionType CCPACSWingRibsPositioning::GetStartDefinitionType() const
{
    if (m_startEtaXsiPoint_choice1)
        return ETAXSI_STARTEND;
    if (m_startCurvePoint_choice2)
        return CURVEPOINT_STARTEND;
    if (m_startSparPositionUID_choice3)
        return SPARPOSITION_STARTEND;
    throw CTiglError("Invalid start definition");
}

CCPACSWingRibsPositioning::StartEndDefinitionType CCPACSWingRibsPositioning::GetEndDefinitionType() const
{
    if (m_endEtaXsiPoint_choice1)
        return ETAXSI_STARTEND;
    if (m_endCurvePoint_choice2)
        return CURVEPOINT_STARTEND;
    if (m_endSparPositionUID_choice3)
        return SPARPOSITION_STARTEND;
    throw CTiglError("Invalid end definition");
}

void CCPACSWingRibsPositioning::SetRibStart(const std::string& value)
{
    // handling registration to uid manager
    if (m_uidMgr) {
        if (isUid(m_ribStart)) m_uidMgr->TryUnregisterReference(m_ribStart, *this);
        if (isUid(value)) m_uidMgr->RegisterReference(value, *this);
    }

    generated::CPACSWingRibsPositioning::SetRibStart(value);
    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetRibEnd(const std::string& value)
{
    // handling registration to uid manager
    if (m_uidMgr) {
        if (isUid(m_ribEnd)) m_uidMgr->TryUnregisterReference(m_ribEnd, *this);
        if (isUid(value)) m_uidMgr->RegisterReference(value, *this);
    }

    generated::CPACSWingRibsPositioning::SetRibEnd(value);
    InvalidateParent();
}

CCPACSWingRibsPositioning::RibCountDefinitionType CCPACSWingRibsPositioning::GetRibCountDefinitionType() const
{
    if (m_spacing_choice1)
        return SPACING;
    if (m_numberOfRibs_choice2)
        return NUMBER_OF_RIBS;
    throw CTiglError("Invalid rib count definition");
}

void CCPACSWingRibsPositioning::SetNumberOfRibs(int numRibs)
{
    m_numberOfRibs_choice2 = numRibs;

    m_spacing_choice1 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetSpacing(double value)
{
    m_spacing_choice1 = value;

    m_numberOfRibs_choice2 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetRibReference(const std::string& value)
{
    generated::CPACSWingRibsPositioning::SetRibReference(value);
    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetStartCurvePoint(const CCPACSCurvePoint &curve_point)
{
    CCPACSCurvePoint& cp = GetStartCurvePoint_choice2(CreateIfNotExists);
    cp.SetEta(curve_point.GetEta());
    cp.SetReferenceUID(curve_point.GetReferenceUID());

    m_startEtaXsiPoint_choice1 = boost::none;
    m_startSparPositionUID_choice3 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetStartEtaXsiPoint(const CCPACSEtaXsiPoint &etaxsi)
{
    CCPACSEtaXsiPoint& ex = CPACSWingRibsPositioning::GetStartEtaXsiPoint_choice1(CreateIfNotExists);
    ex.SetEta(etaxsi.GetEta());
    ex.SetXsi(etaxsi.GetXsi());
    ex.SetReferenceUID(etaxsi.GetReferenceUID());

    m_startCurvePoint_choice2 = boost::none;
    m_startSparPositionUID_choice3 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetStartSparPositionUID(const std::string &sparPosition)
{
    CPACSWingRibsPositioning::SetStartSparPositionUID_choice3(sparPosition);

    m_startEtaXsiPoint_choice1 = boost::none;
    m_startCurvePoint_choice2 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetEndCurvePoint(const CCPACSCurvePoint &curve_point)
{
    CCPACSCurvePoint& cp = GetEndCurvePoint_choice2(CreateIfNotExists);
    cp.SetEta(curve_point.GetEta());
    cp.SetReferenceUID(curve_point.GetReferenceUID());

    m_endEtaXsiPoint_choice1 = boost::none;
    m_endSparPositionUID_choice3 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetEndEtaXsiPoint(const CCPACSEtaXsiPoint &etaxsi)
{
    CCPACSEtaXsiPoint& ex = CPACSWingRibsPositioning::GetEndEtaXsiPoint_choice1(CreateIfNotExists);
    ex.SetEta(etaxsi.GetEta());
    ex.SetXsi(etaxsi.GetXsi());
    ex.SetReferenceUID(etaxsi.GetReferenceUID());

    m_endCurvePoint_choice2 = boost::none;
    m_endSparPositionUID_choice3 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetEndSparPositionUID(const std::string &sparPosition)
{
    CPACSWingRibsPositioning::SetEndSparPositionUID_choice3(sparPosition);

    m_endEtaXsiPoint_choice1 = boost::none;
    m_endCurvePoint_choice2 = boost::none;

    InvalidateParent();
}

void CCPACSWingRibsPositioning::SetRibCrossingBehaviour(const generated::CPACSRibCrossingBehaviour& value)
{
    generated::CPACSWingRibsPositioning::SetRibCrossingBehaviour(value);
    InvalidateParent();
}

void CCPACSWingRibsPositioning::InvalidateParent() const
{
    GetParent()->Invalidate();
}

} // end namespace tigl
