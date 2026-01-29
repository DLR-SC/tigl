/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-25 Marko Alder <marko.alder@dlr.de>
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

#include "CCPACSComponent.h"
#include "generated/CPACSComponent.h"
#include "CTiglUIDManager.h"
#include "CTiglVehicleElementBuilder.h"
#include "tiglcommonfunctions.h"
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

template <typename T>
static const CCPACSElementGeometry* ResolveGeometry(CTiglUIDManager& uidMgr, const std::string& uid)
{
    if (!uidMgr.IsType<T>(uid)) {
        return nullptr;
    }
    return &uidMgr.ResolveObject<T>(uid).GetGeometry();
}

template <typename... Ts>
static const CCPACSElementGeometry* GetGeomFromTypes(CTiglUIDManager& uidMgr, const std::string& uid)
{
    const CCPACSElementGeometry* g = nullptr;
    ((g = g ? g : ResolveGeometry<Ts>(uidMgr, uid)), ...);
    return g;
}

// List of supported system element types
static const CCPACSElementGeometry* GetGeometry(CTiglUIDManager& uidMgr, const std::string& uid)
{
    return GetGeomFromTypes<generated::CPACSVehicleElementBase, generated::CPACSElectricMotor, generated::CPACSBattery,
                            generated::CPACSGearBox, generated::CPACSGasTurbine, generated::CPACSGenerator,
                            generated::CPACSTurboGenerator, generated::CPACSHeatExchanger>(uidMgr, uid);
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

void CCPACSComponent::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath)
{
    Reset();
    generated::CPACSComponent::ReadCPACS(tixiHandle, objectXPath);

    char* cCPACSPath = NULL;
    tixiGetDocumentPath(tixiHandle, &cCPACSPath);
    _cpacsDocPath = cCPACSPath ? std::string(cCPACSPath) : std::string();
}

PNamedShape CCPACSComponent::BuildLoft() const
{
    auto systemElementUID             = m_systemElementUID_choice1.get();
    const CCPACSElementGeometry* geom = GetGeometry(*m_uidMgr, systemElementUID);

    if (!geom) {
        throw CTiglError("Unsupported system element for uid \"" + systemElementUID +
                         "\" in CCPACSComponent::BuildLoft");
    }

    // Use component UID as shape name
    std::string compUid = this->GetObjectUID().get_value_or("unnamed");
    CTiglVehicleElementBuilder builder(*geom, this->GetTransformationMatrix(), compUid, _cpacsDocPath);
    return builder.BuildShape();
}

} //namespace tigl
