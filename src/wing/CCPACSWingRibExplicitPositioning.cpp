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
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl 
{

CCPACSWingRibExplicitPositioning::CCPACSWingRibExplicitPositioning(CCPACSWingRibsDefinition& parent)
: parent(parent)
{
    Cleanup();
}

CCPACSWingRibExplicitPositioning::~CCPACSWingRibExplicitPositioning()
{
    Cleanup();
}

void CCPACSWingRibExplicitPositioning::Cleanup()
{
    startReference.clear();
    endReference.clear();
    startEta = 0.0;
    endEta = 0.0;
}

void CCPACSWingRibExplicitPositioning::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& explicitRibPosXPath)
{
    Cleanup();

    // Get subelement "startReference"
    char* ptrStartReference = NULL;
    if (tixiGetTextElement(tixiHandle, (explicitRibPosXPath + "/startReference").c_str(), &ptrStartReference) != SUCCESS) {
        LOG(ERROR) << "Missing startReference";
        throw CTiglError("Error: Missing startReference in CCPACSWingRibExplicitPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }
    startReference = ptrStartReference;

    // Get subelement "etaStart"
    ReturnCode tixiRet = tixiGetDoubleElement(tixiHandle, (explicitRibPosXPath + "/etaStart").c_str(), &startEta);
    if (tixiRet != SUCCESS) {
        LOG(ERROR) << "Missing etaStart";
        throw CTiglError("Error: Missing etaStart in CCPACSWingRibExplicitPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }

    // Get subelement "endReference"
    char* ptrEndReference = NULL;
    if (tixiGetTextElement(tixiHandle, (explicitRibPosXPath + "/endReference").c_str(), &ptrEndReference) != SUCCESS) {
        LOG(ERROR) << "Missing endReference";
        throw CTiglError("Error: Missing endReference in CCPACSWingRibExplicitPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }
    endReference = ptrEndReference;

    // Get subelement "etaEnd"
    tixiRet = tixiGetDoubleElement(tixiHandle, (explicitRibPosXPath + "/etaEnd").c_str(), &endEta);
    if (tixiRet != SUCCESS) {
        LOG(ERROR) << "Missing etaEnd";
        throw CTiglError("Error: Missing etaEnd in CCPACSWingRibExplicitPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }
}

void CCPACSWingRibExplicitPositioning::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& explicitRibPosXPath) const
{
    TixiSaveExt::TixiSaveTextElement(tixiHandle, explicitRibPosXPath.c_str(), "startReference", startReference.c_str());
    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, explicitRibPosXPath.c_str(), "etaStart", startEta, NULL);
    TixiSaveExt::TixiSaveTextElement(tixiHandle, explicitRibPosXPath.c_str(), "endReference", endReference.c_str());
    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, explicitRibPosXPath.c_str(), "etaEnd", endEta, NULL);
}

const std::string& CCPACSWingRibExplicitPositioning::GetStartReference() const
{
    return startReference;
}

void CCPACSWingRibExplicitPositioning::SetStartReference(const std::string& ref)
{
    startReference = ref;
    // invalidate whole component segment structure since rib could be referenced anywhere
    parent.GetStructure().Invalidate();
}

const std::string& CCPACSWingRibExplicitPositioning::GetEndReference() const
{
    return endReference;
}

void CCPACSWingRibExplicitPositioning::SetEndReference(const std::string& ref)
{
    endReference = ref;
    // invalidate whole component segment structure since rib could be referenced anywhere
    parent.GetStructure().Invalidate();
}

double CCPACSWingRibExplicitPositioning::GetStartEta() const
{
    return startEta;
}

void CCPACSWingRibExplicitPositioning::SetStartEta(double eta)
{
    startEta = eta;
    // invalidate whole component segment structure since rib could be referenced anywhere
    parent.GetStructure().Invalidate();
}

double CCPACSWingRibExplicitPositioning::GetEndEta() const
{
    return endEta;
}

void CCPACSWingRibExplicitPositioning::SetEndEta(double eta)
{
    endEta = eta;
    // invalidate whole component segment structure since rib could be referenced anywhere
    parent.GetStructure().Invalidate();
}

} // end namespace tigl
