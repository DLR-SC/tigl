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
#include "tigl.h"
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
namespace
{

    template <typename T>
    const CCPACSElementGeometry* ResolveGeometry(const CTiglUIDManager& uidMgr, const std::string& uid)
    {
        if (!uidMgr.IsType<T>(uid)) {
            return nullptr;
        }
        return &uidMgr.ResolveObject<T>(uid).GetGeometry();
    }

    template <typename T>
    const boost::optional<CCPACSElementMass>* ResolveMassDescription(const CTiglUIDManager& uidMgr,
                                                                     const std::string& uid)
    {
        if (!uidMgr.IsType<T>(uid)) {
            return nullptr;
        }
        return &uidMgr.ResolveObject<T>(uid).GetMass();
    }

    template <typename... Ts> struct SupportedSystemElementTypes {
        static const CCPACSElementGeometry* GetGeometry(const CTiglUIDManager& uidMgr, const std::string& uid)
        {
            const CCPACSElementGeometry* geometry = nullptr;
            ((geometry = geometry ? geometry : ResolveGeometry<Ts>(uidMgr, uid)), ...);
            return geometry;
        }

        static const boost::optional<CCPACSElementMass>* GetMassDescription(const CTiglUIDManager& uidMgr,
                                                                            const std::string& uid)
        {
            const boost::optional<CCPACSElementMass>* mass = nullptr;
            ((mass = mass ? mass : ResolveMassDescription<Ts>(uidMgr, uid)), ...);
            return mass;
        }
    };

    using SystemElementTypes = SupportedSystemElementTypes<
        CCPACSSysElemBattery, CCPACSSysElemCable, CCPACSSysElemCompressor, CCPACSSysElemConverter,
        CCPACSSysElemDCDCConverter, CCPACSSysElemElectricMachine, CCPACSSysElemElectricMotor, CCPACSSysElemFuelCell,
        CCPACSSysElemGearBox, CCPACSSysElemGenerator, CCPACSSysElemHeatExchanger, CCPACSSysElemInverter,
        CCPACSSysElemPowerDistributionUnit, CCPACSSysElemPowerElectronic, CCPACSSysElemPump, CCPACSSysElemRectifier,
        CCPACSSysElemReservoir, CCPACSSysElemSwitchgear, CCPACSSysElemTurboGenerator, CCPACSVehicleElementBase>;

    const CCPACSElementGeometry* GetGeometry(const CTiglUIDManager& uidMgr, const std::string& uid)
    {
        return SystemElementTypes::GetGeometry(uidMgr, uid);
    }

    const boost::optional<CCPACSElementMass>* GetMassDescription(const CTiglUIDManager& uidMgr, const std::string& uid)
    {
        return SystemElementTypes::GetMassDescription(uidMgr, uid);
    }

} // namespace

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
    const char* representation = ::tiglGeometryRepresentationToString(
        static_cast<TiglGeometryRepresentationFlags>(GetComponentRepresentation()));

    if (!representation) {
        throw CTiglError("Invalid geometry representation for component with uID \"" +
                         GetObjectUID().get_value_or("unnamed") + "\".");
    }

    return representation;
}

void CCPACSComponent::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath)
{
    Reset();
    generated::CPACSComponent::ReadCPACS(tixiHandle, objectXPath);

    char* cCPACSPath = NULL;
    tixiGetDocumentPath(tixiHandle, &cCPACSPath);
    _cpacsDocPath = cCPACSPath ? std::string(cCPACSPath) : std::string();
}

TiglGeometricComponentType CCPACSComponent::GetComponentType() const
{
    return TIGL_COMPONENT_SYSTEM_COMPONENT;
}

TiglGeometricComponentIntent CCPACSComponent::GetComponentIntent() const
{
    return TIGL_INTENT_PHYSICAL;
}

CTiglPoint CCPACSComponent::GetCentroidLocal() const
{
    const PNamedShape loft = BuildLocalLoft();
    if (!loft) {
        std::string uid = "unknown";
        if (const auto* parent = GetElementGeometry().GetNextUIDParent()) {
            uid = parent->GetObjectUID().get_value_or(uid);
        }
        throw CTiglError("No geometry primitives defined for uID=\"" + uid + "\"");
    }
    const TopoDS_Shape shape = loft->Shape();

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
    if (!IsPositioned()) {
        LOG(WARNING) << "Global center of gravity of component with uID=\"" << GetObjectUID().get_value_or("unnamed")
                     << "\" is only available if <transformation> is defined.";
        return boost::none;
    }

    const auto cogLocal = GetCenterOfGravityLocal();
    if (!cogLocal) {
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

    const PNamedShape loft   = BuildLocalLoft();
    const TopoDS_Shape shape = loft ? loft->Shape() : TopoDS_Shape();

    CTiglElementMassBuilder builder(massDef, GetSystemElementUID(), shape);

    const auto result  = builder.EvaluateMass();
    cache.mass         = result.mass;
    cache.cogLocal     = result.cogLocal;
    cache.inertiaLocal = result.inertiaLocal;
}

} // namespace tigl