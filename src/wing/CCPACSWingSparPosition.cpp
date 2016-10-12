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

#include "CCPACSWingSparPosition.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingSpars.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingSparPosition::CCPACSWingSparPosition(CCPACSWingSpars* spars)
: spars(*spars)
{
    Cleanup();
}

CCPACSWingSparPosition::~CCPACSWingSparPosition()
{
    Cleanup();
}

void CCPACSWingSparPosition::Cleanup()
{
    uid.clear();
    eta = 0;
    xsi = 0;
    elementUID.clear();
    inputType = None;
}

void CCPACSWingSparPosition::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sparPositionXPath)
{
    Cleanup();

    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, sparPositionXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        uid = ptrUID;
    }

    // Get subelement "eta" OR "elementUID"
    const std::string etaElementPath = sparPositionXPath + "/eta";
    const std::string elementUIDElementPath = sparPositionXPath + "/elementUID";
    if (tixiCheckElement(tixiHandle, etaElementPath.c_str()) == SUCCESS) {
        if (tixiGetDoubleElement(tixiHandle, etaElementPath.c_str(), &eta) != SUCCESS) {
            LOG(ERROR) << "Missing eta";
            throw CTiglError("Error: Missing eta in CCPACSWingSparPosition::ReadCPACS!", TIGL_XML_ERROR);
        }
        inputType = Eta;
    }
    else if (tixiCheckElement(tixiHandle, elementUIDElementPath.c_str()) == SUCCESS) {
        if (tixiGetTextElement(tixiHandle, elementUIDElementPath.c_str(), &ptrUID) != SUCCESS) {
            LOG(ERROR) << "Missing elementUID";
            throw CTiglError("Error: Missing elementUID in CCPACSWingSparPosition::ReadCPACS!", TIGL_XML_ERROR);
        }
        elementUID = ptrUID;
        inputType = ElementUID;
    }
    else {
        LOG(ERROR) << "Missing eta or elementUID OR spacing";
        throw CTiglError("Error: Missing eta or elementUID OR spacing in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }

    if (tixiGetDoubleElement(tixiHandle, (sparPositionXPath + "/xsi").c_str(), &xsi) != SUCCESS) {
        LOG(ERROR) << "Missing xsi";
        throw CTiglError("Error: Missing xsi in CCPACSWingSparPosition::ReadCPACS!", TIGL_XML_ERROR);
    }
}

void CCPACSWingSparPosition::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& sparPositionXPath) const
{
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, sparPositionXPath.c_str(), "uID", uid.c_str());
    
    switch (inputType) {
    case ElementUID:
        TixiSaveExt::TixiSaveTextElement(tixiHandle, sparPositionXPath.c_str(), "elementUID", elementUID.c_str());
        break;
    case Eta:
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, sparPositionXPath.c_str(), "eta", eta, NULL);
        break;
    default:
        throw CTiglError("Unsupported InputType found in CCPACSWingSparPosition::WriteCPACS");
    }

    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, sparPositionXPath.c_str(), "xsi", GetXsi(), NULL);
}

const std::string& CCPACSWingSparPosition::GetUID(void) const
{
    return uid;
}

CCPACSWingSparPosition::InputType CCPACSWingSparPosition::GetInputType() const
{
    return inputType;
}

const std::string& CCPACSWingSparPosition::GetElementUID() const
{
    if (inputType != ElementUID) {
        throw CTiglError("SparPosition is not defined via elementUID. Please check InputType first before calling CCPACSWingSparPosition::GetElementUID()");
    }
    return elementUID;
}

void CCPACSWingSparPosition::SetElementUID(const std::string& uid)
{
    elementUID = uid;

    eta = 0;
    inputType = ElementUID;

    // invalidate whole component segment structure, since ribs or cells could reference the spar
    spars.GetStructure().Invalidate();
}

double CCPACSWingSparPosition::GetEta() const
{
    if (inputType != Eta) {
        throw CTiglError("SparPosition is not defined via eta. Please check InputType first before calling CCPACSWingSparPosition::GetEta()");
    }
    return eta;
}

void CCPACSWingSparPosition::SetEta(double value)
{
    eta = value;
   
    elementUID.clear();
    inputType = Eta;

    // invalidate whole component segment structure, since ribs or cells could reference the spar
    spars.GetStructure().Invalidate();
}

double CCPACSWingSparPosition::GetXsi() const
{
    return xsi;
}

void CCPACSWingSparPosition::SetXsi(double value)
{
    xsi = value;

    // invalidate whole component segment structure, since ribs or cells could reference the spar
    spars.GetStructure().Invalidate();
}

} // end namespace tigl
