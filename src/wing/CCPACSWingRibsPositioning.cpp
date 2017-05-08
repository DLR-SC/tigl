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

#include "generated/TixiHelper.h"


namespace tigl
{

CCPACSWingRibsPositioning::CCPACSWingRibsPositioning(CCPACSWingRibsDefinition* parent)
    : generated::CPACSWingRibsPositioning(parent) {}

CCPACSWingRibsPositioning::StartDefinitionType CCPACSWingRibsPositioning::GetStartDefinitionType() const
{
    if (m_etaStart_choice1)
        return ETA_START;
    if (m_elementStartUID_choice2)
        return ELEMENT_START;
    if (m_sparPositionStartUID_choice3)
        return SPARPOSITION_START;
    throw CTiglError("Invalid start definition");
}

double CCPACSWingRibsPositioning::GetEtaStart() const
{
    if (!m_etaStart_choice1) {
        throw CTiglError("RibsPositioning is not defined via etaStart. Please check StartDefinitionType first before calling CCPACSWingRibsPositioning::GetEtaStart()");
    }
    return *m_etaStart_choice1;
}

void CCPACSWingRibsPositioning::SetEtaStart(double value)
{
    generated::CPACSWingRibsPositioning::SetEtaStart_choice1(value);

    m_elementStartUID_choice2 = boost::none;
    m_sparPositionStartUID_choice3 = boost::none;

    // invalidate whole component segment structure, since cells could reference the ribs
    GetParent()->GetParent()->Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetElementStartUID() const
{
    if (!m_elementStartUID_choice2) {
        throw CTiglError("RibsPositioning is not defined via elementStartUID. Please check StartDefinitionType first before calling CCPACSWingRibsPositioning::GetElementStartUID()");
    }
    return *m_elementStartUID_choice2;
}

void CCPACSWingRibsPositioning::SetElementStartUID(const std::string& uid)
{
    generated::CPACSWingRibsPositioning::SetElementStartUID_choice2(uid);

    m_etaStart_choice1 = boost::none;
    m_sparPositionStartUID_choice3 = boost::none;

    GetParent()->GetParent()->Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetSparPositionStartUID() const
{
    if (!m_sparPositionStartUID_choice3) {
        throw CTiglError("RibsPositioning is not defined via sparPositionStartUID. Please check StartDefinitionType first before calling CCPACSWingRibsPositioning::GetSparPositionStartUID()");
    }
    return *m_sparPositionStartUID_choice3;
}

void CCPACSWingRibsPositioning::SetSparPositionStartUID(const std::string& uid)
{
    generated::CPACSWingRibsPositioning::SetSparPositionStartUID_choice3(uid);

    m_etaStart_choice1 = boost::none;
    m_elementStartUID_choice2 = boost::none;

    GetParent()->GetParent()->Invalidate();
}

CCPACSWingRibsPositioning::EndDefinitionType CCPACSWingRibsPositioning::GetEndDefinitionType() const
{
    if (m_etaEnd_choice1)
        return ETA_END;
    if (m_elementEndUID_choice2)
        return ELEMENT_END;
    if (m_sparPositionEndUID_choice3)
        return SPARPOSITION_END;
    throw CTiglError("Invalid end definition");
}

double CCPACSWingRibsPositioning::GetEtaEnd() const
{
    if (!m_etaEnd_choice1) {
        throw CTiglError("RibsPositioning is not defined via etaEnd. Please check EndDefinitionType first before calling CCPACSWingRibsPositioning::GetEtaEnd()");
    }
    return *m_etaEnd_choice1;
}

void CCPACSWingRibsPositioning::SetEtaEnd(double value)
{
    generated::CPACSWingRibsPositioning::SetEtaEnd_choice1(value);

    m_elementEndUID_choice2 = boost::none;
    m_sparPositionEndUID_choice3 = boost::none;

    GetParent()->GetParent()->Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetElementEndUID() const
{
    if (!m_elementEndUID_choice2) {
        throw CTiglError("RibsPositioning is not defined via elementEndUID. Please check EndDefinitionType first before calling CCPACSWingRibsPositioning::GetElementEndUID()");
    }
    return *m_elementEndUID_choice2;
}

void CCPACSWingRibsPositioning::SetElementEndUID(const std::string& uid)
{
    generated::CPACSWingRibsPositioning::SetElementEndUID_choice2(uid);

    m_etaEnd_choice1 = boost::none;
    m_sparPositionEndUID_choice3 = boost::none;

    GetParent()->GetParent()->Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetSparPositionEndUID() const
{
    if (!m_sparPositionEndUID_choice3) {
        throw CTiglError("RibsPositioning is not defined via sparPositionEndUID. Please check EndDefinitionType first before calling CCPACSWingRibsPositioning::GetSparPositionEndUID()");
    }
    return *m_sparPositionEndUID_choice3;
}

void CCPACSWingRibsPositioning::SetSparPositionEndUID(const std::string& uid)
{
    generated::CPACSWingRibsPositioning::SetSparPositionEndUID_choice3(uid);

    m_etaEnd_choice1 = boost::none;
    m_elementEndUID_choice2 = boost::none;

    GetParent()->GetParent()->Invalidate();
}

CCPACSWingRibsPositioning::RibCountDefinitionType CCPACSWingRibsPositioning::GetRibCountDefinitionType() const
{
    if (m_spacing_choice1)
        return SPACING;
    if (m_numberOfRibs_choice2)
        return NUMBER_OF_RIBS;
    throw CTiglError("Invalid rib count definition");
}

int CCPACSWingRibsPositioning::GetNumberOfRibs() const
{
    if (!m_numberOfRibs_choice2) {
        throw CTiglError("RibsPositioning is not defined via numberOfRibs. Please check RibCountDefinitionType first before calling CCPACSWingRibsPositioning::GetNumberOfRibs()");
    }
    return *m_numberOfRibs_choice2;
}

void CCPACSWingRibsPositioning::SetNumberOfRibs(int numRibs)
{
    m_numberOfRibs_choice2 = numRibs;

    m_spacing_choice1 = boost::none;

    GetParent()->GetParent()->Invalidate();
}

double CCPACSWingRibsPositioning::GetSpacing() const
{
    if (!m_spacing_choice1) {
        throw CTiglError("RibsPositioning is not defined via spacing. Please check RibCountDefinitionType first before calling CCPACSWingRibsPositioning::GetSpacing()");
    }
    return *m_spacing_choice1;
}

void CCPACSWingRibsPositioning::SetSpacing(double value)
{
    m_spacing_choice1 = value;

    m_numberOfRibs_choice2 = boost::none;

    GetParent()->GetParent()->Invalidate();
}

} // end namespace tigl
