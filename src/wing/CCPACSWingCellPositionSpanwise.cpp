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

#include "CCPACSWingCellPositionSpanwise.h"

#include "CCPACSWingCell.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSWingCellPositionSpanwise::CCPACSWingCellPositionSpanwise(CCPACSWingCell* parent)
    : generated::CPACSCellPositioningSpanwise(parent) {}

CCPACSWingCellPositionSpanwise::CCPACSWingCellPositionSpanwise(CPACSWingIntermediateStructureCell* parent)
    : generated::CPACSCellPositioningSpanwise(parent) {
    throw std::logic_error("CPACSWingIntermediateStructureCell as parent is not implemented");
}

CCPACSWingCellPositionSpanwise::InputType CCPACSWingCellPositionSpanwise::GetInputType() const {
    if (m_eta1_choice1 && m_eta2_choice1)
        return InputType::Eta;
    if (m_ribDefinitionUID_choice2 && m_ribNumber_choice2)
        return InputType::Rib;
    return InputType::None;
}

// get and set Eta definition
void CCPACSWingCellPositionSpanwise::SetEta(double eta1, double eta2) {
    m_eta1_choice1 = eta1;
    m_eta2_choice1 = eta2;

    m_ribNumber_choice2 = boost::none;
    m_ribDefinitionUID_choice2 = boost::none;

    invalidateParent();
}

void CCPACSWingCellPositionSpanwise::GetEta(double& eta1, double& eta2) const {
    // std::tie(eta1, eta2) = GetEta();
    std::pair<double, double> etas = GetEta();
    eta1 = etas.first;
    eta2 = etas.second;
}

std::pair<double, double> CCPACSWingCellPositionSpanwise::GetEta() const {
    if (GetInputType() != InputType::Eta) {
        throw CTiglError("CCPACSWingCellPositionSpanwise::GetEta method called, but position is defined via ribDefinitionUID!");
    }
    return std::make_pair(*m_eta1_choice1, *m_eta2_choice1);
}

void CCPACSWingCellPositionSpanwise::SetRib(const std::string& ribUId, int nRib) {
    m_eta1_choice1 = boost::none;
    m_eta2_choice1 = boost::none;

    m_ribNumber_choice2 = nRib;
    m_ribDefinitionUID_choice2 = ribUId;

    invalidateParent();
}

void CCPACSWingCellPositionSpanwise::GetRib(std::string& ribUid, int& ribNumber) const {
    //std::tie(ribUid, ribNumber) = GetRib();
    std::pair<std::string, int> ribDefinition = GetRib();
    ribUid = ribDefinition.first;
    ribNumber = ribDefinition.second;
}

std::pair<std::string, int> CCPACSWingCellPositionSpanwise::GetRib() const {
    if (GetInputType() != InputType::Rib) {
        throw CTiglError("CCPACSWingCellPositionSpanwise::GetRib method called, but position is defined via eta1/eta2!");
    }
    return std::make_pair(*m_ribDefinitionUID_choice2, *m_ribNumber_choice2);
}
void CCPACSWingCellPositionSpanwise::invalidateParent()
{
    GetParent<CCPACSWingCell>()->Invalidate();
}

}
