/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-17 Marko Alder <marko.alder@dlr.de>
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

#include "CCPACSDeckComponent2DBase.h"
#include "CCPACSDeck.h"
#include "generated/CPACSVehicleElementBase.h"
#include "generated/CPACSSeatElement.h"

#include "CCPACSConfiguration.h"
#include "CTiglUIDManager.h"
#include "CTiglElementGeometryBuilder.h"
#include "CTiglElementMassBuilder.h"
#include "tiglcommonfunctions.h"
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
    return GetGeomFromTypes<CCPACSVehicleElementBase, CCPACSSeatElement>(uidMgr, uid);
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
    return GetMassDescriptionFromTypes<CCPACSVehicleElementBase, CCPACSSeatElement>(uidMgr, uid);
}

CCPACSDeckComponent2DBase::CCPACSDeckComponent2DBase(CCPACSClassDividers* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponent2DBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponent2DBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}

CCPACSDeckComponent2DBase::CCPACSDeckComponent2DBase(CCPACSGalleys* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponent2DBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponent2DBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}
CCPACSDeckComponent2DBase::CCPACSDeckComponent2DBase(CCPACSGenericFloorModules* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponent2DBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponent2DBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}

CCPACSDeckComponent2DBase::CCPACSDeckComponent2DBase(CCPACSLavatories* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponent2DBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponent2DBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}

CCPACSDeckComponent2DBase::CCPACSDeckComponent2DBase(CCPACSSeatModules* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponent2DBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponent2DBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}

std::string CCPACSDeckComponent2DBase::GetDefaultedUID() const
{
    return generated::CPACSDeckComponent2DBase::GetUID();
}

CCPACSConfiguration const& CCPACSDeckComponent2DBase::GetConfiguration() const
{
    return m_parentDeck->GetConfiguration();
}

TiglGeometryRepresentation CCPACSDeckComponent2DBase::GetComponentRepresentation() const
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

std::string CCPACSDeckComponent2DBase::GetComponentRepresentationAsString() const
{
    switch (GetComponentRepresentation()) {
    case TIGL_GEOMREP_PHYSICAL:
        return "physical";
    case TIGL_GEOMREP_ENVELOPE:
        return "envelope";
    }
}

void CCPACSDeckComponent2DBase::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath)
{
    Reset();
    generated::CPACSDeckComponent2DBase::ReadCPACS(tixiHandle, objectXPath);

    char* cCPACSPath = NULL;
    tixiGetDocumentPath(tixiHandle, &cCPACSPath);
    _cpacsDocPath = cCPACSPath ? std::string(cCPACSPath) : std::string();
}

boost::optional<double> CCPACSDeckComponent2DBase::GetMass() const
{
    return m_mass->mass;
}

boost::optional<CTiglPoint> CCPACSDeckComponent2DBase::GetCenterOfGravityLocal() const
{
    return m_mass->cogLocal;
}

boost::optional<CTiglPoint> CCPACSDeckComponent2DBase::GetCenterOfGravityGlobal() const
{
    const auto cogLocal = m_mass->cogLocal;
    if (!IsPositioned()) {
        LOG(WARNING) << "Global center of gravity of component \"" << GetObjectUID().get_value_or("unnamed")
                     << "\" is only available if <transformation> is defined.";
        return boost::none;
    }
    return GetTransformationMatrix() * (*cogLocal);
}

boost::optional<CTiglMassInertia> CCPACSDeckComponent2DBase::GetMassInertiaLocal() const
{
    return m_mass->inertiaLocal;
}

bool CCPACSDeckComponent2DBase::IsPositioned() const
{
    return GetTransformation().is_initialized();
}

const CCPACSElementGeometry& CCPACSDeckComponent2DBase::GetElementGeometry() const
{
    const std::string deckElementUID = m_deckElementUID;
    const CCPACSElementGeometry* const geom = GetGeometry(*m_uidMgr, deckElementUID);

    if (!geom) {
        throw CTiglError("Unsupported system element for uID \"" + deckElementUID + "\".");
    }

    return *geom;
}

PNamedShape CCPACSDeckComponent2DBase::BuildLoft() const
{
    const CCPACSElementGeometry& geom = GetElementGeometry();

    // Use component UID as shape name
    const std::string compName = this->GetObjectUID().get_value_or("unnamed");

    // The builder works on the generic CTiglRelativelyPositionedComponent,
    // therefore the CCPACSDeckComponent2DBase-specific information (configuration, geometry, uID) needs to be determined here
    CTiglElementGeometryBuilder builder(*this, this->GetConfiguration(), geom, compName, _cpacsDocPath);
    const PNamedShape shape = builder.BuildShape();

    // Apply the resolved component transformation to the generated shape and return
    return GetTransformationMatrix().Transform(shape);
}

void CCPACSDeckComponent2DBase::BuildMass(MassCache& cache) const
{
    const std::string uid = m_deckElementUID;

    const auto* massPtr = GetMassDescription(*m_uidMgr, uid);
    if (!massPtr || !*massPtr) {
        LOG(WARNING) << "No mass definition for uID \"" + uid + "\"!";
        return;
    }

    const CCPACSElementMass& massDef = massPtr->get();

    CTiglElementMassBuilder builder(massDef, m_deckElementUID, GetLoft()->Shape());

    const auto result  = builder.EvaluateMass();
    cache.mass         = result.mass;
    cache.cogLocal     = result.cogLocal; //ToDo: is it really local?
    cache.inertiaLocal = result.inertiaLocal;
}

} //namespace tigl
