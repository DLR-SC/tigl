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
#include "CCPACSWingRibRotation.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingRibRotation::CCPACSWingRibRotation(CCPACSWingRibsDefinition& parent)
: parent(parent)
{
    Cleanup();
}

CCPACSWingRibRotation::~CCPACSWingRibRotation()
{
    Cleanup();
}

void CCPACSWingRibRotation::Cleanup()
{
    ribRotationReference.clear();
    zRotation = 90.0;
}

void CCPACSWingRibRotation::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& ribRotationXPath)
{
    Cleanup();

    // Get subelement "ribRotationReference"
    char* ptrRibRotationReference = NULL;
    if (tixiGetTextElement(tixiHandle, (ribRotationXPath + "/ribRotationReference").c_str(), &ptrRibRotationReference) == SUCCESS) {
        ribRotationReference = ptrRibRotationReference;
    }

    // Get subelement "z"
    if (tixiGetDoubleElement(tixiHandle, (ribRotationXPath + "/z").c_str(), &zRotation) != SUCCESS) {
        LOG(ERROR) << "Missing z";
        throw CTiglError("Error: Missing z in CCPACSWingRibRotation::ReadCPACS!", TIGL_XML_ERROR);
    }
}

void CCPACSWingRibRotation::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& ribRotationXPath) const
{
    if (!ribRotationReference.empty()) {
        TixiSaveExt::TixiSaveTextElement(tixiHandle, ribRotationXPath.c_str(), "ribRotationReference", ribRotationReference.c_str());
    }
    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, ribRotationXPath.c_str(), "z", zRotation, NULL);
}

const std::string& CCPACSWingRibRotation::GetRibRotationReference() const
{
    return ribRotationReference;
}

void CCPACSWingRibRotation::SetRibRotationReference(const std::string& reference)
{
    ribRotationReference = reference;
    // invalidate whole component segment structure since rib may be referenced anywhere
    parent.GetStructure().Invalidate();
}

double CCPACSWingRibRotation::GetZRotation() const
{
    return zRotation;
}

void CCPACSWingRibRotation::SetZRotation(double value)
{
    zRotation = value;
    // invalidate whole component segment structure since rib may be referenced anywhere
    parent.GetStructure().Invalidate();
}

} // end namespace tigl
