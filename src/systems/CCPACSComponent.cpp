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

namespace tigl
{

static const CCPACSElementGeometry* TryGetGeometryByUID(CTiglUIDManager* mgr, const std::string& uid)
{
    try {
        return &mgr->ResolveObject<generated::CPACSVehicleElementBase>(uid).GetGeometry();
    }
    catch (...) {
    }
    try {
        return &mgr->ResolveObject<generated::CPACSElectricMotor>(uid).GetGeometry();
    }
    catch (...) {
    }
    try {
        return &mgr->ResolveObject<generated::CPACSBattery>(uid).GetGeometry();
    }
    catch (...) {
    }
    try {
        return &mgr->ResolveObject<generated::CPACSGearBox>(uid).GetGeometry();
    }
    catch (...) {
    }
    try {
        return &mgr->ResolveObject<generated::CPACSGasTurbine>(uid).GetGeometry();
    }
    catch (...) {
    }
    try {
        return &mgr->ResolveObject<generated::CPACSGenerator>(uid).GetGeometry();
    }
    catch (...) {
    }
    try {
        return &mgr->ResolveObject<generated::CPACSTurboGenerator>(uid).GetGeometry();
    }
    catch (...) {
    }
    try {
        return &mgr->ResolveObject<generated::CPACSHeatExchanger>(uid).GetGeometry();
    }
    catch (...) {
    }


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

    const CCPACSElementGeometry* geom = TryGetGeometryByUID(m_uidMgr, systemElementUID);

    const auto transform              = this->GetTransformationMatrix();
    CTiglVehicleElementBuilder builder(*geom, transform);
    return builder.BuildShape();
}

} //namespace tigl
