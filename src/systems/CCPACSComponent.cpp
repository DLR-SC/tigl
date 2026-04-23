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
#include "CPACSSysElemBattery.h"
#include "CPACSSysElemCable.h"
#include "CPACSSysElemCompressor.h"
#include "CPACSSysElemConverter.h"
#include "CPACSSysElemDCDCConverter.h"
#include "CPACSSysElemElectricMachine.h"
#include "CPACSSysElemElectricMotor.h"
#include "CPACSSysElemFuelCell.h"
#include "CPACSSysElemGearBox.h"
#include "CPACSSysElemGenerator.h"
#include "CPACSSysElemHeatExchanger.h"
#include "CPACSSysElemInverter.h"
#include "CPACSSysElemPowerDistributionUnit.h"
#include "CPACSSysElemPowerElectronic.h"
#include "CPACSSysElemPump.h"
#include "CPACSSysElemRectifier.h"
#include "CPACSSysElemReservoir.h"
#include "CPACSSysElemSwitchgear.h"
#include "CPACSSysElemTurboGenerator.h"
#include "CPACSVehicleElementBase.h"
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
    return GetGeomFromTypes<
        CCPACSSysElemBattery, CCPACSSysElemCable, CCPACSSysElemCompressor, CCPACSSysElemConverter,
        CCPACSSysElemDCDCConverter, CCPACSSysElemElectricMachine, CCPACSSysElemElectricMotor, CCPACSSysElemFuelCell,
        CCPACSSysElemGearBox, CCPACSSysElemGenerator, CCPACSSysElemHeatExchanger, CCPACSSysElemInverter,
        CCPACSSysElemPowerDistributionUnit, CCPACSSysElemPowerElectronic, CCPACSSysElemPump, CCPACSSysElemRectifier,
        CCPACSSysElemReservoir, CCPACSSysElemSwitchgear, CCPACSSysElemTurboGenerator, CCPACSVehicleElementBase>(uidMgr,
                                                                                                                uid);
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
    return GetMassDescriptionFromTypes<
        CCPACSSysElemBattery, CCPACSSysElemCable, CCPACSSysElemCompressor, CCPACSSysElemConverter,
        CCPACSSysElemDCDCConverter, CCPACSSysElemElectricMachine, CCPACSSysElemElectricMotor, CCPACSSysElemFuelCell,
        CCPACSSysElemGearBox, CCPACSSysElemGenerator, CCPACSSysElemHeatExchanger, CCPACSSysElemInverter,
        CCPACSSysElemPowerDistributionUnit, CCPACSSysElemPowerElectronic, CCPACSSysElemPump, CCPACSSysElemRectifier,
        CCPACSSysElemReservoir, CCPACSSysElemSwitchgear, CCPACSSysElemTurboGenerator, CCPACSVehicleElementBase>(uidMgr,
                                                                                                                uid);
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

TiglGeometryRepresentation CCPACSComponent::GetComponentRepresentation() const
{
    using CPACSGeomRep        = generated::CPACSGeometryRepresentation;
    const auto representation = GetElementGeometry().GetRepresentation().get_value_or(CPACSGeomRep::physical);

    if (representation == CPACSGeomRep::physical) {
        return TIGL_GEOMREP_PHYSICAL;
    }
    if (representation == CPACSGeomRep::envelope) {
        return TIGL_GEOMREP_ENVELOPE;
    }

    throw CTiglError("Invalid geometry representation for component with uID \"" +
                     GetObjectUID().get_value_or("unnamed") + "\".");
}

std::string CCPACSComponent::GetComponentRepresentationAsString() const
{
    switch (GetComponentRepresentation()) {
    case TIGL_GEOMREP_PHYSICAL:
        return "physical";
    case TIGL_GEOMREP_ENVELOPE:
        return "envelope";
    }
}

void CCPACSComponent::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath)
{
    Reset();
    generated::CPACSComponent::ReadCPACS(tixiHandle, objectXPath);

    char* cCPACSPath = NULL;
    tixiGetDocumentPath(tixiHandle, &cCPACSPath);
    _cpacsDocPath = cCPACSPath ? std::string(cCPACSPath) : std::string();
}

CTiglPoint CCPACSComponent::GetCentroidLocal() const
{
    const TopoDS_Shape shape = BuildLocalLoft()->Shape();

    GProp_GProps props;
    BRepGProp::VolumeProperties(shape, props);

    if (props.Mass() <= 0.0) {
        throw CTiglError("Cannot compute geometric centroid of component with uID=\"" +
                         GetObjectUID().get_value_or("unnamed") + "\" (zero volume).");
    }

    const gp_Pnt c = props.CentreOfMass();
    return CTiglPoint(c.X(), c.Y(), c.Z());
}

boost::optional<CTiglPoint> CCPACSComponent::GetCentroidGlobal() const
{
    if (!IsPositioned()) {
        LOG(WARNING) << "Global centroid of component with uID=\"" << GetObjectUID().get_value_or("unnamed")
                     << "\" is only available if <transformation> is defined.";
        return boost::none;
    }
    return GetTransformationMatrix() * GetCentroidLocal();
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
        LOG(WARNING) << "Global center of gravity of component with uID=\"" << GetObjectUID().get_value_or("unnamed")
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

const CCPACSElementGeometry& CCPACSComponent::GetElementGeometry() const
{
    const CCPACSElementGeometry* const geom = GetGeometry(*m_uidMgr, GetSystemElementUID());

    if (!geom) {
        throw CTiglError("Unsupported system element for uID \"" + GetSystemElementUID() + "\".");
    }

    return *geom;
}

PNamedShape CCPACSComponent::BuildLocalLoft() const
{
    const CCPACSElementGeometry& geom = GetElementGeometry();
    const std::string compName        = GetObjectUID().get_value_or("unnamed");

    CTiglElementGeometryBuilder builder(*this, GetConfiguration(), geom, compName, _cpacsDocPath);
    return builder.BuildShape();
}

PNamedShape CCPACSComponent::BuildLoft() const
{
    return GetTransformationMatrix().Transform(BuildLocalLoft());
}

void CCPACSComponent::BuildMass(MassCache& cache) const
{
    const auto* massPtr = GetMassDescription(*m_uidMgr, GetSystemElementUID());
    if (!massPtr || !*massPtr) {
        LOG(WARNING) << "No mass definition for uID \"" + GetSystemElementUID() + "\"!";
        return;
    }

    const CCPACSElementMass& massDef = massPtr->get();

    CTiglElementMassBuilder builder(massDef, GetSystemElementUID(), BuildLocalLoft()->Shape());

    const auto result  = builder.EvaluateMass();
    cache.mass         = result.mass;
    cache.cogLocal     = result.cogLocal;
    cache.inertiaLocal = result.inertiaLocal;
    return;
}

} //namespace tigl
