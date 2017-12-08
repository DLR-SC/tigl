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


void CCPACSWingRibsPositioning::SetEtaStart(double value)
{
    m_etaStart.SetEta(value);

    // invalidate whole component segment structure, since cells could reference the ribs
    invalidateStructure();
}


void CCPACSWingRibsPositioning::SetRibStart(const std::string& uid)
{
    generated::CPACSWingRibsPositioning::SetRibStart(uid);

    invalidateStructure();
}

void CCPACSWingRibsPositioning::SetEtaEnd(double value)
{
    m_etaEnd.SetEta(value);

    invalidateStructure();
}

void CCPACSWingRibsPositioning::SetRibEnd(const std::string& uid)
{
    generated::CPACSWingRibsPositioning::SetRibEnd(uid);

    invalidateStructure();
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

    invalidateStructure();
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

    invalidateStructure();
}

void CCPACSWingRibsPositioning::invalidateStructure()
{
    GetParent()->GetParent()->GetParent()->Invalidate();
}

} // end namespace tigl
