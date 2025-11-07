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
#include "generated/CPACSElectricMotor.h"
#include "generated/CPACSBattery.h"
#include "generated/CPACSGearBox.h"
#include "generated/CPACSGasTurbine.h"
#include "generated/CPACSGenerator.h"
#include "generated/CPACSTurboGenerator.h"
#include "generated/CPACSHeatExchanger.h"
#include "CPACSElementGeometry.h" 

namespace tigl
{

    static const CCPACSElementGeometry* GetGeometryFrom(CTiglUIDObject& obj)
    {
        if (auto* ve = dynamic_cast<generated::CPACSVehicleElementBase*>(&obj)) return &ve->GetGeometry();
        if (auto* em = dynamic_cast<generated::CPACSElectricMotor*>(&obj)) return &em->GetGeometry();
        if (auto* b = dynamic_cast<generated::CPACSBattery*>(&obj)) return &b->GetGeometry();
        if (auto* gb = dynamic_cast<generated::CPACSGearBox*>(&obj)) return &gb->GetGeometry();
        if (auto* gt = dynamic_cast<generated::CPACSGasTurbine*>(&obj)) return &gt->GetGeometry();
        if (auto* gen = dynamic_cast<generated::CPACSGenerator*>(&obj)) return &gen->GetGeometry();
        if (auto* tg = dynamic_cast<generated::CPACSTurboGenerator*>(&obj)) return &tg->GetGeometry();
        if (auto* hx = dynamic_cast<generated::CPACSHeatExchanger*>(&obj)) return &hx->GetGeometry();
        return nullptr;
    }

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

    CTiglUIDObject& any = m_uidMgr->ResolveUIDObject(systemElementUID);
    const CCPACSElementGeometry* geom = GetGeometryFrom(any);
    if (!geom) {
        throw CTiglError("Unsupported system element for uid \"" + systemElementUID + "\" in CCPACSComponent::BuildLoft");
    }

    CTiglVehicleElementBuilder builder(*geom, this->GetTransformationMatrix());
    return builder.BuildShape();
}

} //namespace tigl
