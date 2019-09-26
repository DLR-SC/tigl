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
#include "CCPACSWingRibExplicitPositioning.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"

namespace tigl
{
CCPACSWingRibExplicitPositioning::CCPACSWingRibExplicitPositioning(CCPACSWingRibsDefinition* parent)
    : generated::CPACSWingRibExplicitPositioning(parent)
{
}

void CCPACSWingRibExplicitPositioning::SetStartCurvePoint(const CCPACSCurvePoint &curve_point)
{
    CCPACSCurvePoint& cp = GetStartCurvePoint_choice2(CreateIfNotExists);
    cp.SetEta(curve_point.GetEta());
    cp.SetReferenceUID(curve_point.GetReferenceUID());

    m_startEtaXsiPoint_choice1 = boost::none;
    m_startSparPositionUID_choice3 = boost::none;

    Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetStartEtaXsiPoint(const CCPACSEtaXsiPoint &etaxsi)
{
    CCPACSEtaXsiPoint& ex = GetStartEtaXsiPoint_choice1(CreateIfNotExists);
    ex.SetEta(etaxsi.GetEta());
    ex.SetXsi(etaxsi.GetXsi());
    ex.SetReferenceUID(etaxsi.GetReferenceUID());

    m_startCurvePoint_choice2 = boost::none;
    m_startSparPositionUID_choice3 = boost::none;

    Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetStartSparPositionUID(const std::string &sparPosition)
{
    CPACSWingRibExplicitPositioning::SetStartSparPositionUID_choice3(sparPosition);

    m_startEtaXsiPoint_choice1 = boost::none;
    m_startCurvePoint_choice2 = boost::none;

    Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetEndCurvePoint(const CCPACSCurvePoint &curve_point)
{
    CCPACSCurvePoint& cp = GetEndCurvePoint_choice2(CreateIfNotExists);
    cp.SetEta(curve_point.GetEta());
    cp.SetReferenceUID(curve_point.GetReferenceUID());

    m_endEtaXsiPoint_choice1 = boost::none;
    m_endSparPositionUID_choice3 = boost::none;

    Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetEndEtaXsiPoint(const CCPACSEtaXsiPoint &etaxsi)
{
    CCPACSEtaXsiPoint& ex = GetEndEtaXsiPoint_choice1(CreateIfNotExists);
    ex.SetEta(etaxsi.GetEta());
    ex.SetXsi(etaxsi.GetXsi());
    ex.SetReferenceUID(etaxsi.GetReferenceUID());

    m_endCurvePoint_choice2 = boost::none;
    m_endSparPositionUID_choice3 = boost::none;

    Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetEndSparPositionUID(const std::string &sparPosition)
{
    CPACSWingRibExplicitPositioning::SetEndSparPositionUID_choice3(sparPosition);

    m_endEtaXsiPoint_choice1 = boost::none;
    m_endCurvePoint_choice2 = boost::none;

    Invalidate();
}


void CCPACSWingRibExplicitPositioning::Invalidate()
{
    m_parent->GetParent()->Invalidate();
}
}
