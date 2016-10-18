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

#include "CTiglWingSpanwiseBorder.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl 
{

// Constructor
CTiglWingSpanwiseBorder::CTiglWingSpanwiseBorder()
: m_eta1CPACSName("eta1"),
  m_eta2CPACSName("eta2")
{
    Reset();
}


void CTiglWingSpanwiseBorder::Reset()
{
    m_inputType = None;
    m_eta1 = 0;
    m_eta2 = 0;
    m_ribNumber = 0;
    m_ribDefinitionUID.clear();
}

// Read CPACS wings elements
void CTiglWingSpanwiseBorder::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    Reset();

    // Get subelement "eta1, eta2" or Rib Uid and rib Nr (choice)
    const std::string eta1String = xpath + "/" + m_eta1CPACSName;
    const std::string ribUIdString  =  xpath + "/ribDefinitionUID";

    if (tixiCheckElement(tixiHandle, eta1String.c_str()) == SUCCESS) {
        m_inputType = Eta;
        if (tixiGetDoubleElement(tixiHandle, eta1String.c_str(), &m_eta1) != SUCCESS) {
            LOG(ERROR) << "Error during read of <" << m_eta1CPACSName << ">";
            throw CTiglError("Error: Error during read of <" + m_eta1CPACSName + "> in CTiglWingSpanwiseBorder::ReadCPACS!", TIGL_XML_ERROR);
        }
        if (tixiGetDoubleElement(tixiHandle, (xpath + "/" + m_eta2CPACSName).c_str(), &m_eta2) != SUCCESS) {
            LOG(ERROR) << "Error during read of <" << m_eta2CPACSName << ">";
            throw CTiglError("Error: Error during read of <" + m_eta2CPACSName + "> in CTiglWingSpanwiseBorder::ReadCPACS!", TIGL_XML_ERROR);
        }
    }
    else if (tixiCheckElement(tixiHandle, ribUIdString.c_str()) == SUCCESS) {
        m_inputType = Rib;
        char* ptrRUId = NULL;
        if (tixiGetTextElement(tixiHandle, ribUIdString.c_str(), &ptrRUId) != SUCCESS) {
            LOG(ERROR) << "Error during read of <ribDefinitionUID>";
            throw CTiglError("Error: Error during read of <ribDefinitionUID> in CTiglWingSpanwiseBorder::ReadCPACS!", TIGL_XML_ERROR);
        }
        m_ribDefinitionUID = ptrRUId;
        if (tixiGetIntegerElement(tixiHandle, (xpath + "/ribNumber").c_str(), &m_ribNumber) != SUCCESS) {
            LOG(ERROR) << "Error during read of <ribNumber>";
            throw CTiglError("Error: Error during read of <ribNumber> in CTiglWingSpanwiseBorder::ReadCPACS!", TIGL_XML_ERROR);
        }
    }
    else {
        LOG(ERROR) << "Missing element " << m_eta1CPACSName << " or ribDefinitionUID";
        throw CTiglError("Error: Missing element <" + m_eta1CPACSName + "> or <ribDefinitionUID> in CTiglWingSpanwiseBorder::ReadCPACS!", TIGL_XML_ERROR);
    }
}

// Read CPACS wings elements
void CTiglWingSpanwiseBorder::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    switch (m_inputType) {
    case Eta:
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), m_eta1CPACSName.c_str(), m_eta1, NULL);
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), m_eta2CPACSName.c_str(), m_eta2, NULL);
        break;
    case Rib:
        TixiSaveExt::TixiSaveTextElement(tixiHandle, xpath.c_str(), "ribDefinitionUID", m_ribDefinitionUID.c_str());
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), "ribNumber", m_ribNumber, NULL);
        break;
    default:
        LOG(ERROR) << "Missing element <eta1> or <ribDefinitionUID>";
        throw CTiglError("Error: Missing element <eta1> or <ribDefinitionUID> in CTiglWingSpanwiseBorder::WriteCPACS!", TIGL_XML_ERROR);
    }
}

CTiglWingSpanwiseBorder::InputType CTiglWingSpanwiseBorder::GetInputType() const
{
    return m_inputType;
}

// get and set Eta definition
void CTiglWingSpanwiseBorder::SetEta(double eta1, double eta2)
{
    m_inputType = Eta;

    m_eta1 = eta1;
    m_eta2 = eta2;
    m_ribNumber = 0;
    m_ribDefinitionUID.clear();

    invalidateParent();
}

void CTiglWingSpanwiseBorder::GetEta(double& eta1, double& eta2) const
{
    // std::tie(eta1, eta2) = GetEta();
    std::pair<double, double> etas = GetEta();
    eta1 = etas.first;
    eta2 = etas.second;
}

std::pair<double, double> CTiglWingSpanwiseBorder::GetEta() const
{
    if (m_inputType != Eta) {
        throw CTiglError("CTiglWingSpanwiseBorder::GetEta method called, but position is defined via ribDefinitionUID!");
    }
    return std::make_pair(m_eta1, m_eta2);
}

void CTiglWingSpanwiseBorder::SetRib(const std::string& ribUId, int nRib)
{
    m_inputType = Rib;

    m_eta1 = 0.;
    m_eta2 = 0.;
    m_ribNumber = nRib;
    m_ribDefinitionUID = ribUId;

    invalidateParent();
}

void CTiglWingSpanwiseBorder::GetRib(std::string& ribUid, int& ribNumber) const
{
    //std::tie(ribUid, ribNumber) = GetRib();
    std::pair<std::string, int> ribDefinition = GetRib();
    ribUid = ribDefinition.first;
    ribNumber = ribDefinition.second;
}

std::pair<std::string, int> CTiglWingSpanwiseBorder::GetRib() const
{
    if (m_inputType != Rib) {
        throw CTiglError("CTiglWingSpanwiseBorder::GetRib method called, but position is defined via eta1/eta2!");
    }
    return std::make_pair(m_ribDefinitionUID, m_ribNumber);
}

}
