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
: m_parent(parent)
{
    Reset();
}

void CCPACSWingCellPositionChordwise::Reset()
{
    m_inputType = None;
    m_xsi1 = 0;
    m_xsi2 = 0;
    m_sparUID.clear();
}

void CCPACSWingCellPositionChordwise::ReadCPACS(TixiDocumentHandle tixiHandle,  const std::string& xpath)
{
    Reset();

    // Get subelement "xsi1, xsi2" or Spar Uid (choice)
    const std::string sparUIdString  = xpath + "/sparUID";
    const std::string xsi1String     = xpath + "/xsi1";

    if (tixiCheckElement(tixiHandle, xsi1String.c_str()) == SUCCESS) {
        m_inputType = Xsi;
        if (tixiGetDoubleElement(tixiHandle, xsi1String.c_str(), &m_xsi1) != SUCCESS) {
            LOG(ERROR) << "Error during read of <xsi1>";
            throw CTiglError("Error: Error during read of <xsi1> in CCPACSWingCellPositionChordwise::ReadCPACS!", TIGL_XML_ERROR);
        }
        if (tixiGetDoubleElement(tixiHandle, (xpath + "/xsi2").c_str(), &m_xsi2) != SUCCESS) {
            LOG(ERROR) << "Error during read of <xsi2>";
            throw CTiglError("Error: Error during read of <xsi2> in CCPACSWingCellPositionChordwise::ReadCPACS!", TIGL_XML_ERROR);
        }
    }
    else if (tixiCheckElement(tixiHandle, sparUIdString.c_str()) == SUCCESS) {
        m_inputType = Spar;
        char* ptrSUId = NULL;
        if (tixiGetTextElement(tixiHandle, sparUIdString.c_str(), &ptrSUId) != SUCCESS) {
            LOG(ERROR) << "Error during read of <sparUID>";
            throw CTiglError("Error: Error during read of <sparUID> in CCPACSWingCellPositionChordwise::ReadCPACS!", TIGL_XML_ERROR);
        }
        m_sparUID = ptrSUId;

    }
    else {
        LOG(ERROR) << "Missing element <xsi1> or <sparUID>";
        throw CTiglError("Error: Missing element <xsi1> or <sparUID> in CCPACSWingCellPositionChordwise::ReadCPACS!", TIGL_XML_ERROR);
    }
}

void CCPACSWingCellPositionChordwise::WriteCPACS(TixiDocumentHandle tixiHandle,  const std::string& xpath) const
{
    switch (m_inputType) {
    case Xsi:
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), "xsi1", m_xsi1, NULL);
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, xpath.c_str(), "xsi2", m_xsi2, NULL);
        break;
    case Spar:
        TixiSaveExt::TixiSaveTextElement(tixiHandle, xpath.c_str(), "sparUID", this->m_sparUID.c_str());
        break;
    default:
        LOG(ERROR) << "Missing element <xsi1> or <sparUID>";
        throw CTiglError("Error: Missing element <xsi1> or <sparUID> in CCPACSWingCellPositionChordwise::WriteCPACS!", TIGL_XML_ERROR);
    }
}

CCPACSWingCellPositionChordwise::InputType CCPACSWingCellPositionChordwise::GetInputType() const
{
    return m_inputType;
}

void CCPACSWingCellPositionChordwise::SetXsi(double xsi1, double xsi2)
{
    m_inputType = Xsi;

    m_xsi1 = xsi1;
    m_xsi2 = xsi2;
    m_sparUID.clear();

    m_parent->Invalidate();
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
    if (m_inputType != Xsi) {
        throw CTiglError("CCPACSWingCellPositionChordwise::GetXsi method called, but position is defined via sparUID!");
    }
    return std::make_pair(m_xsi1, m_xsi2);
}

void CCPACSWingCellPositionChordwise::SetSparUId(std::string sparUId)
{
    m_inputType = Spar;

    m_xsi1 = 0.;
    m_xsi2 = 0.;
    m_sparUID = sparUId;

    m_parent->Invalidate();
}

const std::string& CCPACSWingCellPositionChordwise::GetSparUId() const
{
    if (m_inputType != Spar) {
        throw CTiglError("CCPACSWingCellPositionChordwise::GetSparUId method called, but position is defined via xsi1/xsi2!");
    }
    return m_sparUID;
}

}
