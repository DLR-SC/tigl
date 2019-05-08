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

namespace tigl
{

CCPACSWingRibsPositioning::CCPACSWingRibsPositioning(CCPACSWingRibsDefinition* parent)
    : generated::CPACSWingRibsPositioning(parent) {}


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

    Invalidate();
}

void CCPACSWingRibsPositioning::SetSpacing(double value)
{
    m_spacing_choice1 = value;

    m_numberOfRibs_choice2 = boost::none;

    Invalidate();
}

void CCPACSWingRibsPositioning::Invalidate()
{
    GetParent()->GetParent()->GetParent()->Invalidate();
}

void CCPACSWingRibsPositioning::SetStartCurvePoint(const CCPACSCurvePoint &curve_point)
{
    CCPACSCurvePoint& cp = GetStartCurvePoint_choice2(CreateIfNotExists);
    cp.SetEta(curve_point.GetEta());
    cp.SetReferenceUID(curve_point.GetReferenceUID());
    Invalidate();
}

void CCPACSWingRibsPositioning::SetStartEtaXsiPoint(const CCPACSEtaXsiPoint &etaxsi)
{
    CCPACSEtaXsiPoint& ex = CPACSWingRibsPositioning::GetStartEtaXsiPoint_choice1(CreateIfNotExists);
    ex.SetEta(etaxsi.GetEta());
    ex.SetXsi(etaxsi.GetXsi());
    ex.SetReferenceUID(etaxsi.GetReferenceUID());
    Invalidate();
}

void CCPACSWingRibsPositioning::SetStartSparPositionUID(const std::string &sparPosition)
{
    CPACSWingRibsPositioning::SetStartSparPositionUID_choice3(sparPosition);
    Invalidate();
}

void CCPACSWingRibsPositioning::SetEndCurvePoint(const CCPACSCurvePoint &curve_point)
{
    CCPACSCurvePoint& cp = GetEndCurvePoint_choice2(CreateIfNotExists);
    cp.SetEta(curve_point.GetEta());
    cp.SetReferenceUID(curve_point.GetReferenceUID());
    Invalidate();
}

void CCPACSWingRibsPositioning::SetEndEtaXsiPoint(const CCPACSEtaXsiPoint &etaxsi)
{
    CCPACSEtaXsiPoint& ex = CPACSWingRibsPositioning::GetEndEtaXsiPoint_choice1(CreateIfNotExists);
    ex.SetEta(etaxsi.GetEta());
    ex.SetXsi(etaxsi.GetXsi());
    ex.SetReferenceUID(etaxsi.GetReferenceUID());
    Invalidate();
}

void CCPACSWingRibsPositioning::SetEndSparPositionUID(const std::string &sparPosition)
{
    CPACSWingRibsPositioning::SetEndSparPositionUID_choice3(sparPosition);
    Invalidate();
}

void CCPACSWingRibsPositioning::SetRibCrossingBehaviour(const generated::CPACSRibCrossingBehaviour& value)
{
    generated::CPACSWingRibsPositioning::SetRibCrossingBehaviour(value);
    Invalidate();
}

} // end namespace tigl
