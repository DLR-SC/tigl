/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2019-01-31 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSComponent.h"
#include "generated/CPACSComponent.h"
#include "CTiglUIDManager.h"
#include "CTiglVehicleElementBuilder.h"
#include "generated/CPACSVehicleElementBase.h"

namespace tigl
{

CCPACSComponent::CCPACSComponent(CCPACSComponents* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSComponent(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation)
{
}

std::string CCPACSComponent::GetDefaultedUID() const
{
    return generated::CPACSComponent::GetUID();
}

PNamedShape CCPACSComponent::BuildLoft() const
{
    auto systemElementUID = m_systemElementUID_choice1.get();

    try {
        CCPACSVehicleElementBase& element = m_uidMgr->ResolveObject<CCPACSVehicleElementBase>(systemElementUID);
        auto transform                    = this->GetTransformationMatrix();
        CTiglVehicleElementBuilder builder(element, transform);
        return builder.BuildShape();
    }
    catch (...) {
        throw(CTiglError("CCPACSComponent: Unable to build shape for the system component with UID " +
                             systemElementUID + ".",
                         TIGL_ERROR));
    }
}

} //namespace tigl
