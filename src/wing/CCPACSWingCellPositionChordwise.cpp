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

#include "CCPACSWingCell.h"
#include "CCPACSWingCellPositionChordwise.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingCellPositionChordwise::CCPACSWingCellPositionChordwise(CCPACSWingCell* parent)
    : generated::CPACSCellPositioningChordwise(parent) {}

CCPACSWingCellPositionChordwise::CCPACSWingCellPositionChordwise(CPACSWingIntermediateStructureCell* parent)
    : generated::CPACSCellPositioningChordwise(parent) {}

CCPACSWingCellPositionChordwise::InputType CCPACSWingCellPositionChordwise::GetInputType() const
{
    if (m_xsi1_choice2 && m_xsi2_choice2)
        return InputType::Xsi;
    if (m_sparUID_choice1)
        return InputType::Spar;
    return InputType::None;
}

void CCPACSWingCellPositionChordwise::SetXsi(double xsi1, double xsi2)
{
    m_xsi1_choice2 = xsi1;
    m_xsi2_choice2 = xsi2;
    m_sparUID_choice1 = boost::none;

    if (IsParent<CCPACSWingCell>())
        GetParent<CCPACSWingCell>()->Invalidate();
}

void CCPACSWingCellPositionChordwise::GetXsi(double& xsi1, double& xsi2) const
{
    //std::tie(xsi1, xsi2) = GetXsi();
    std::pair<double, double> xsis = GetXsi();
    xsi1 = xsis.first;
    xsi2 = xsis.second;
}

std::pair<double, double> CCPACSWingCellPositionChordwise::GetXsi() const
{
    if (GetInputType() != InputType::Xsi) {
        throw CTiglError("CCPACSWingCellPositionChordwise::GetXsi method called, but position is defined via sparUID!");
    }
    return std::make_pair(*m_xsi1_choice2, *m_xsi2_choice2);
}

void CCPACSWingCellPositionChordwise::SetSparUId(std::string sparUId)
{
    m_xsi1_choice2 = boost::none;
    m_xsi2_choice2 = boost::none;
    m_sparUID_choice1 = sparUId;

    if (IsParent<CCPACSWingCell>())
        GetParent<CCPACSWingCell>()->Invalidate();
}

const std::string& CCPACSWingCellPositionChordwise::GetSparUId() const
{
    if (GetInputType() != InputType::Spar) {
        throw CTiglError("CCPACSWingCellPositionChordwise::GetSparUId method called, but position is defined via xsi1/xsi2!");
    }
    return *m_sparUID_choice1;
}

}
