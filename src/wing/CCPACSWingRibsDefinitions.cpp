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


namespace tigl 
{

CCPACSWingRibsDefinitions::CCPACSWingRibsDefinitions(CCPACSWingCSStructure* structure, CTiglUIDManager* uidMgr)
    : generated::CPACSWingRibsDefinitions(structure, uidMgr) {}

void CCPACSWingRibsDefinitions::Invalidate()
{
    for (std::vector<unique_ptr<CCPACSWingRibsDefinition> >::iterator it = m_ribsDefinitions.begin(); it != m_ribsDefinitions.end(); ++it) {
        (*it)->Invalidate();
    }
}

int CCPACSWingRibsDefinitions::GetRibsDefinitionCount() const
{
    return static_cast<int>(m_ribsDefinitions.size());
}

const CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const int index) const
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetRibsDefinitionCount()) {
        LOG(ERROR) << "Invalid index value";
        throw CTiglError("Invalid index value in CCPACSWingRibsDefinitions::GetRibsDefinition", TIGL_INDEX_ERROR);
    }
    return static_cast<CCPACSWingRibsDefinition&>(*(m_ribsDefinitions[idx]));
}

CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const int index)
{
    // forward call to const method
    return const_cast<CCPACSWingRibsDefinition&>(static_cast<const CCPACSWingRibsDefinitions&>(*this).GetRibsDefinition(index));
}

const CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const std::string& uid) const
{
    for (std::vector<unique_ptr<CCPACSWingRibsDefinition> >::const_iterator it = m_ribsDefinitions.begin(); it != m_ribsDefinitions.end(); ++it) {
        const unique_ptr<CCPACSWingRibsDefinition>& tempRib = *it;
        if (tempRib->GetUID() == uid) {
            return *tempRib;
        }
    }

    const std::string& referenceUID = CTiglWingStructureReference(*GetParent()).GetUID();
    LOG(ERROR) << "Ribs Definition \"" << uid << "\" not found in component segment or trailing edge device with UID \"" << referenceUID << "\"";
    throw CTiglError("Ribs Definition \"" + uid + "\" not found in component segment or trailing edge device with UID \"" + referenceUID + "\". Please check the CPACS document!", TIGL_ERROR);
}

CCPACSWingRibsDefinition& CCPACSWingRibsDefinitions::GetRibsDefinition(const std::string& uid)
{
    // forward call to const method
    return const_cast<CCPACSWingRibsDefinition&>(static_cast<const CCPACSWingRibsDefinitions&>(*this).GetRibsDefinition(uid));
}


} // end namespace tigl
