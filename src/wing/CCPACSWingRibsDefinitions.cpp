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
#include "CCPACSWingRibsDefinitions.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "IOHelper.h"
#include "TixiSaveExt.h"


namespace tigl 
{

CCPACSWingRibsDefinitions::CCPACSWingRibsDefinitions(CCPACSWingCSStructure& structure)
: structure(structure)
{
    Cleanup();
}

CCPACSWingRibsDefinitions::~CCPACSWingRibsDefinitions()
{
    Cleanup();
}

void CCPACSWingRibsDefinitions::Invalidate()
{
    CCPACSWingRibsDefinitionContainer::iterator it;
    for (it = ribsDefinitions.begin(); it != ribsDefinitions.end(); ++it) {
        (*it)->Invalidate();
    }
}

void CCPACSWingRibsDefinitions::Cleanup()
{
    CCPACSWingRibsDefinitionContainer::iterator it;
    for (it = ribsDefinitions.begin(); it != ribsDefinitions.end(); ++it) {
        delete *it;
    }
    ribsDefinitions.clear();
}

// Read CPACS element
void CCPACSWingRibsDefinitions::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    Cleanup();
    ReadContainerElement(tixiHandle, xpath, "ribsDefinition", 1, ribsDefinitions, &structure);
}

void CCPACSWingRibsDefinitions::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    WriteContainerElement(tixiHandle, xpath, "ribsDefinition", ribsDefinitions);
}

int CCPACSWingRibsDefinitions::GetRibsDefinitionCount() const
{
    return static_cast<int>(ribsDefinitions.size());
}

const CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const int index) const
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetRibsDefinitionCount()) {
        LOG(ERROR) << "Invalid index value";
        throw CTiglError("Error: Invalid index value in CCPACSWingRibsDefinitions::GetRibsDefinition", TIGL_INDEX_ERROR);
    }
    return static_cast<CCPACSWingRibsDefinition&>(*(ribsDefinitions[idx]));
}

CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const int index)
{
    // forward call to const method
    return const_cast<CCPACSWingRibsDefinition&>(static_cast<const CCPACSWingRibsDefinitions&>(*this).GetRibsDefinition(index));
}

const CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const std::string& uid) const
{
    CCPACSWingRibsDefinitionContainer::const_iterator it;
    for (it = ribsDefinitions.begin(); it != ribsDefinitions.end(); ++it) {
        CCPACSWingRibsDefinition* tempRib = *it;
        if (tempRib->GetUID() == uid) {
            return *tempRib;
        }
    }

    std::string referenceUID = structure.GetWingStructureReference().GetUID();
    LOG(ERROR) << "Ribs Definition \"" << uid << "\" not found in component segment or trailing edge device with UID \"" << referenceUID << "\"";
    throw CTiglError("Error: Ribs Definition \"" + uid + "\" not found in component segment or trailing edge device with UID \"" + referenceUID + "\". Please check the CPACS document!", TIGL_ERROR);
}

CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const std::string& uid)
{
    // forward call to const method
    return const_cast<CCPACSWingRibsDefinition&>(static_cast<const CCPACSWingRibsDefinitions&>(*this).GetRibsDefinition(uid));
}


} // end namespace tigl
