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

#include "CCPACSDeckComponentBase.h"
#include "CCPACSDeck.h"
#include "generated/CPACSVehicleElementBase.h"

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
    return GetGeomFromTypes<CCPACSVehicleElementBase>(uidMgr, uid);
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
    return GetMassDescriptionFromTypes<CCPACSVehicleElementBase>(uidMgr, uid);
}

CCPACSDeckComponentBase::CCPACSDeckComponentBase(CCPACSCeilingPanels* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponentBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponentBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}

CCPACSDeckComponentBase::CCPACSDeckComponentBase(CCPACSLuggageCompartments* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponentBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponentBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}

CCPACSDeckComponentBase::CCPACSDeckComponentBase(CCPACSSidewallPanels* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeckComponentBase(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(parent->GetParent(), &m_transformation)
    , m_mass(*this, &CCPACSDeckComponentBase::BuildMass)
    , m_parentDeck(parent->GetParent())
{
}

std::string CCPACSDeckComponentBase::GetDefaultedUID() const
{
    return generated::CPACSDeckComponentBase::GetUID();
}

CCPACSConfiguration const& CCPACSDeckComponentBase::GetConfiguration() const
{
    return m_parentDeck->GetConfiguration();
}

TiglGeometryRepresentation CCPACSDeckComponentBase::GetComponentRepresentation() const
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

std::string CCPACSDeckComponentBase::GetComponentRepresentationAsString() const
{
    switch (GetComponentRepresentation()) {
    case TIGL_GEOMREP_PHYSICAL:
        return "physical";
    case TIGL_GEOMREP_ENVELOPE:
        return "envelope";
    }
}

void CCPACSDeckComponentBase::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath)
{
    Reset();
    generated::CPACSDeckComponentBase::ReadCPACS(tixiHandle, objectXPath);

    char* cCPACSPath = NULL;
    tixiGetDocumentPath(tixiHandle, &cCPACSPath);
    _cpacsDocPath = cCPACSPath ? std::string(cCPACSPath) : std::string();
}

boost::optional<double> CCPACSDeckComponentBase::GetMass() const
{
    return m_mass->mass;
}

boost::optional<CTiglPoint> CCPACSDeckComponentBase::GetCenterOfGravityLocal() const
{
    return m_mass->cogLocal;
}

boost::optional<CTiglPoint> CCPACSDeckComponentBase::GetCenterOfGravityGlobal() const
{
    const auto cogLocal = m_mass->cogLocal;
    if (!IsPositioned()) {
        LOG(WARNING) << "Global center of gravity of component \"" << GetObjectUID().get_value_or("unnamed")
                     << "\" is only available if <transformation> is defined.";
        return boost::none;
    }
    return GetTransformationMatrix() * (*cogLocal);
}

boost::optional<CTiglMassInertia> CCPACSDeckComponentBase::GetMassInertiaLocal() const
{
    return m_mass->inertiaLocal;
}

bool CCPACSDeckComponentBase::IsPositioned() const
{
    return GetTransformation().is_initialized();
}

const CCPACSElementGeometry& CCPACSDeckComponentBase::GetElementGeometry() const
{
    const std::string deckElementUID        = m_deckElementUID;
    const CCPACSElementGeometry* const geom = GetGeometry(*m_uidMgr, deckElementUID);

    if (!geom) {
        throw CTiglError("Unsupported system element for uID \"" + deckElementUID + "\".");
    }

    return *geom;
}

PNamedShape CCPACSDeckComponentBase::BuildLoft() const
{
    const CCPACSElementGeometry& geom = GetElementGeometry();

    // Use component UID as shape name
    const std::string compName = this->GetObjectUID().get_value_or("unnamed");

    // The builder works on the generic CTiglRelativelyPositionedComponent,
    // therefore the CCPACSComponent-specific information (configuration, geometry, uID) needs to be extracted at this level
    CTiglElementGeometryBuilder builder(*this, this->GetConfiguration(), geom, compName, _cpacsDocPath);
    const PNamedShape shape = builder.BuildShape();

    // Apply local transformation and return shape
    return GetTransformationMatrix().Transform(shape);
}

void CCPACSDeckComponentBase::BuildMass(MassCache& cache) const
{
    const std::string uid = m_deckElementUID;

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
