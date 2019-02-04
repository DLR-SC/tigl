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
#include "CCPACSWingCells.h"
#include "CCPACSWingShell.h"
#include "CCPACSWingCSStructure.h"
#include "CTiglWingStructureReference.h"
#include "CTiglError.h"
#include "tigletaxsifunctions.h"

namespace tigl
{

namespace {
    // return uid of wing structure reference, or the empty string when not all parents are defined
    std::string tryGetWingStructureReferenceUID(CCPACSWingCell* parent)
    {
        if (!parent || !parent->GetParent() || !parent->GetParent()->GetParent() || !parent->GetParent()->GetParent()->GetParent()) {
            return "";
        }
        return CTiglWingStructureReference(*parent->GetParent()->GetParent()->GetParent()).GetUID();
    }
}

CCPACSWingCellPositionSpanwise::CCPACSWingCellPositionSpanwise(CCPACSWingCell* parent)
    : generated::CPACSCellPositioningSpanwise(parent) {}

CCPACSWingCellPositionSpanwise::InputType CCPACSWingCellPositionSpanwise::GetInputType() const {
    if (m_eta1_choice1 && m_eta2_choice1)
        return Eta;
    if (m_ribDefinitionUID_choice2 && m_ribNumber_choice2)
        return Rib;
    throw CTiglError("Invalid spanwise cell positioning", TIGL_XML_ERROR);
}

void CCPACSWingCellPositionSpanwise::SetEta1(double eta1) {
    const std::string uid = tryGetWingStructureReferenceUID(m_parent);
    GetEta1_choice1(CreateIfNotExists).SetEta(eta1);
    GetEta1_choice1(CreateIfNotExists).SetReferenceUID(uid);
    m_ribNumber_choice2 = boost::none;
    m_ribDefinitionUID_choice2 = boost::none;

    InvalidateParent();
}

void CCPACSWingCellPositionSpanwise::SetEta2(double eta2) {
    const std::string uid = tryGetWingStructureReferenceUID(m_parent);
    GetEta2_choice1(CreateIfNotExists).SetEta(eta2);
    GetEta2_choice1(CreateIfNotExists).SetReferenceUID(uid);
    m_ribNumber_choice2 = boost::none;
    m_ribDefinitionUID_choice2 = boost::none;

    InvalidateParent();
}

// get and set Eta definition
void CCPACSWingCellPositionSpanwise::SetEta(double eta1, double eta2)
{
    const std::string uid = tryGetWingStructureReferenceUID(m_parent);
    SetEta(eta1, uid, eta2, uid);
}

void CCPACSWingCellPositionSpanwise::SetEta(double eta1, const std::string& eta1RefUid, double eta2,
                                            const std::string& eta2RefUid)
{
    GetEta1_choice1(CreateIfNotExists).SetEta(eta1);
    GetEta1_choice1(CreateIfNotExists).SetReferenceUID(eta1RefUid);
    GetEta2_choice1(CreateIfNotExists).SetEta(eta2);
    GetEta2_choice1(CreateIfNotExists).SetReferenceUID(eta2RefUid);

    m_ribNumber_choice2 = boost::none;
    m_ribDefinitionUID_choice2 = boost::none;

    InvalidateParent();
}

void CCPACSWingCellPositionSpanwise::GetEta(double& eta1, double& eta2) const {
    // std::tie(eta1, eta2) = GetEta();
    std::pair<double, double> etas = GetEta();
    eta1 = etas.first;
    eta2 = etas.second;
}

std::pair<double, double> CCPACSWingCellPositionSpanwise::GetEta() const {
    if (GetInputType() != Eta) {
        throw CTiglError("CCPACSWingCellPositionSpanwise::GetEta method called, but position is defined via ribDefinitionUID!");
    }

    const CCPACSEtaIsoLine& etaIso1 = m_eta1_choice1.value();
    const CCPACSEtaIsoLine& etaIso2 = m_eta2_choice1.value();
    // special handling for testing
    if (etaIso1.GetReferenceUID().empty() && etaIso2.GetReferenceUID().empty()) {
        return std::make_pair(etaIso1.GetEta(), etaIso2.GetEta());
    }
    const double eta1 = transformEtaToCSOrTed(etaIso1, m_parent->GetUIDManager());
    const double eta2 = transformEtaToCSOrTed(etaIso2, m_parent->GetUIDManager());
    return std::make_pair(eta1, eta2);
}

void CCPACSWingCellPositionSpanwise::SetRib(const std::string& ribUId, int nRib) {
    m_eta1_choice1 = boost::none;
    m_eta2_choice1 = boost::none;
    m_ribNumber_choice2 = nRib;
    m_ribDefinitionUID_choice2 = ribUId;

    InvalidateParent();
}

void CCPACSWingCellPositionSpanwise::SetRibDefinitionUID(const std::string& ribUId) {
    m_eta1_choice1 = boost::none;
    m_eta2_choice1 = boost::none;
    m_ribDefinitionUID_choice2 = ribUId;

    InvalidateParent();
}

void CCPACSWingCellPositionSpanwise::SetRibNumber(int nRib) {
    m_eta1_choice1 = boost::none;
    m_eta2_choice1 = boost::none;
    m_ribNumber_choice2 = nRib;

    InvalidateParent();
}

void CCPACSWingCellPositionSpanwise::GetRib(std::string& ribUid, int& ribNumber) const {
    //std::tie(ribUid, ribNumber) = GetRib();
    std::pair<std::string, int> ribDefinition = GetRib();
    ribUid = ribDefinition.first;
    ribNumber = ribDefinition.second;
}

std::pair<std::string, int> CCPACSWingCellPositionSpanwise::GetRib() const {
    if (GetInputType() != Rib) {
        throw CTiglError("CCPACSWingCellPositionSpanwise::GetRib method called, but position is defined via eta1/eta2!");
    }
    return std::make_pair(m_ribDefinitionUID_choice2.value(), m_ribNumber_choice2.value());
}

void CCPACSWingCellPositionSpanwise::InvalidateParent()
{
    if (GetParent()) {
        GetParent()->Invalidate();
    }
}

}
