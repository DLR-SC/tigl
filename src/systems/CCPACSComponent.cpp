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

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>

#include "CCPACSConfiguration.h"
#include "CCPACSComponent.h"
#include "generated/CPACSComponents.h"
#include "CCPACSGenericSystems.h"
#include "CCPACSGenericSystem.h"
#include "CCPACSACSystems.h"
#include "generated/CPACSComponent.h"
#include "CTiglUIDManager.h"
#include "CTiglElementGeometryBuilder.h"
#include "CTiglElementMassBuilder.h"
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
#include "CPACSElementMass.h"

#include "CNamedShape.h"

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

static const CCPACSElementGeometry* GetGeometry(CTiglUIDManager& uidMgr, const std::string& uid)
{
    return GetGeomFromTypes<generated::CPACSVehicleElementBase, generated::CPACSElectricMotor, generated::CPACSBattery,
                            generated::CPACSGearBox, generated::CPACSGasTurbine, generated::CPACSGenerator,
                            generated::CPACSTurboGenerator, generated::CPACSHeatExchanger>(uidMgr, uid);
}

template <typename T>
static const boost::optional<CCPACSElementMass>* ResolveMassDescription(CTiglUIDManager& uidMgr, const std::string& uid)
{
    if (!uidMgr.IsType<T>(uid)) {
        return nullptr;
    }
    return &uidMgr.ResolveObject<T>(uid).GetMass();
}

template <typename... Ts>
static const boost::optional<CCPACSElementMass>* GetMassDescriptionFromTypes(CTiglUIDManager& uidMgr,
                                                                             const std::string& uid)
{
    const boost::optional<CCPACSElementMass>* m = nullptr;
    ((m = m ? m : ResolveMassDescription<Ts>(uidMgr, uid)), ...);
    return m;
}

static const boost::optional<CCPACSElementMass>* GetMassDescription(CTiglUIDManager& uidMgr, const std::string& uid)
{
    return GetMassDescriptionFromTypes<generated::CPACSVehicleElementBase, generated::CPACSElectricMotor,
                                       generated::CPACSBattery, generated::CPACSGearBox, generated::CPACSGasTurbine,
                                       generated::CPACSGenerator, generated::CPACSTurboGenerator,
                                       generated::CPACSHeatExchanger>(uidMgr, uid);
}

CCPACSComponent::CCPACSComponent(CCPACSComponents* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSComponent(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation)
    , m_mass(*this, &CCPACSComponent::BuildMass)
{
}

std::string CCPACSComponent::GetDefaultedUID() const
{
    return generated::CPACSComponent::GetUID();
}

CCPACSConfiguration const& CCPACSComponent::GetConfiguration() const
{
    return m_parent->GetParent()->GetParent()->GetConfiguration();
}

void CCPACSComponent::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath)
{
    Reset();
    generated::CPACSComponent::ReadCPACS(tixiHandle, objectXPath);

    char* cCPACSPath = NULL;
    tixiGetDocumentPath(tixiHandle, &cCPACSPath);
    _cpacsDocPath = cCPACSPath ? std::string(cCPACSPath) : std::string();
}

boost::optional<double> CCPACSComponent::GetMass() const
{
    return m_mass->mass;
}

boost::optional<CTiglPoint> CCPACSComponent::GetCenterOfGravityLocal() const
{
    return m_mass->cogLocal;
}

boost::optional<CTiglPoint> CCPACSComponent::GetCenterOfGravityGlobal() const
{
    const auto cogLocal = m_mass->cogLocal;
    if (!IsPositioned()) {
        LOG(WARNING) << "Global center of gravity of component \"" << GetObjectUID().get_value_or("unnamed")
                     << "\" is only available if <transformation> is defined.";
        return boost::none;
    }
    return GetTransformationMatrix() * (*cogLocal);
}

boost::optional<CTiglMassInertia> CCPACSComponent::GetMassInertiaLocal() const
{
    return m_mass->inertiaLocal;
}

bool CCPACSComponent::IsPositioned() const
{
    return GetTransformation().is_initialized();
}

PNamedShape CCPACSComponent::BuildLoft() const
{
    const auto systemElementUID       = m_systemElementUID_choice1.get();
    const CCPACSElementGeometry* geom = GetGeometry(*m_uidMgr, systemElementUID);

    if (!geom) {
        throw CTiglError("Unsupported system element for uid \"" + systemElementUID +
                         "\" in CCPACSComponent::BuildLoft");
    }

    // Use component UID as shape name
    const std::string compName = this->GetObjectUID().get_value_or("unnamed");

    // The builder works on the generic CTiglRelativelyPositionedComponent,
    // therefore the CCPACSComponent-specific information (configuration, geometry, uID) needs to be extracted at this level
    CTiglElementGeometryBuilder builder(*this, this->GetConfiguration(), *geom, compName, _cpacsDocPath);
    const PNamedShape shape = builder.BuildShape();

    // Apply local transformation and return shape
    return GetTransformationMatrix().Transform(shape);
}

void CCPACSComponent::BuildMass(MassCache& cache) const
{
    const std::string uid = m_systemElementUID_choice1.get();

    const auto* massPtr = GetMassDescription(*m_uidMgr, uid);
    if (!massPtr || !*massPtr) {
        LOG(WARNING) << "No mass definition for uid \"" + uid + "\"!";
        return;
    }

    const CCPACSElementMass& massDef = massPtr->get();

    CTiglElementMassBuilder builder(massDef, uid, GetLoft()->Shape());

    const auto result  = builder.EvaluateMass();
    cache.mass         = result.mass;
    cache.cogLocal     = result.cogLocal; //ToDo: is it really local?
    cache.inertiaLocal = result.inertiaLocal;
}

} //namespace tigl
