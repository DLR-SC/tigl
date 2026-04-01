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

#pragma once

#include "generated/CPACSDeckComponentBase.h"
#include "generated/CPACSMassInertia.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CTiglPoint.h"
#include "CTiglMassInertia.h"

namespace tigl
{
class CCPACSConfiguration;

/**
 * @brief Representing a CPACS <deck> element.
 *
 * A CCPACSDeckComponent2DBase references a deck element via @c deckElementUID and provides
 * geometric and mass properties derived from that referenced element.
 *
 * Mass properties:
 * - The mass can be given explicitly or computed from a density and the component volume.
 * - The center of gravity (CoG) can be given explicitly via CPACS @c location.
 *   If not provided, it is derived from the geometric centroid of the component.
 *
 * Coordinate frames:
 * - Local values (mass, CoG local) are expressed in the component's local coordinate system.
 */
class CCPACSDeckComponentBase : public generated::CPACSDeckComponentBase, public CTiglRelativelyPositionedComponent
{
public:
    /**
     * @brief Constructs a CCPACSDeckComponent2DBase.
     * @param parent Parent CPACS container owning this component.
     * @param uidMgr UID manager for resolving referenced deck elements.
     */
    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSCeilingPanels* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSClassDividers* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSGalleys* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSGenericFloorModules* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSLavatories* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSLuggageCompartments* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSSeatModules* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSSidewallPanels* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSDeckComponentBase(CCPACSCargoContainers* parent, CTiglUIDManager* uidMgr);

    /**
     * @brief Returns the component UID (defaulted if required).
     */
    TIGL_EXPORT std::string GetDefaultedUID() const override;

    TIGL_EXPORT CCPACSConfiguration const& GetConfiguration() const;

    /**
     * @brief Reads the CPACS subtree of this component.
     */
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath) override;

    /// @brief Returns the TiGL geometric component type.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override
    {
        return TIGL_COMPONENT_DECK_COMPONENT;
    }

    /// @brief Returns the TiGL geometric component intent.
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override
    {
        return TIGL_INTENT_PHYSICAL;
    }

    /**
     * @brief Returns the geometric representation type of the referenced deck element.
     *
     * The representation is obtained from the referenced element geometry definition.
     * It is returned as one of the TiGL constants
     * @c TIGL_GEOMREP_PHYSICAL (1) or @c TIGL_GEOMREP_ENVELOPE (2).
     * If no explicit representation is given in CPACS,
     * @c TIGL_GEOMREP_PHYSICAL (1) is returned.
     *
     * @return Geometry representation type of the component.
     */
    TIGL_EXPORT TiglGeometryRepresentation GetComponentRepresentation() const;

    /**
     * @brief Returns the geometry representation of the referenced system element as string.
     *
     * Possible return values are:
     * - @c "physical"
     * - @c "envelope"
     *
     * If no explicit representation is given in CPACS, @c "physical" is returned.
     *
     * @return Geometry representation as human-readable string.
     */
    TIGL_EXPORT std::string GetComponentRepresentationAsString() const;

    /**
     * @brief Returns the component mass.
     *
     * The mass is obtained from the referenced system element:
     * - If an explicit mass is provided in CPACS, it is returned.
     * - Otherwise, if a density is provided, the mass is computed as density * volume.
     * - If neither mass nor density is available, no mass value is returned.
     *
     * @return Component mass, or boost::none if the mass cannot be determined.
     */
    TIGL_EXPORT boost::optional<double> GetMass() const;

    /**
     * @brief Returns the center of gravity in the component's local coordinate system.
     *
     * If a CPACS @c location is provided, that value is used.
     * Otherwise, the geometric centroid of the component volume is computed.
     *
     * If the center of gravity cannot be determined (e.g., missing mass definition or
     * zero-volume geometry), boost::none is returned.
     *
     * @return Local CoG (x, y, z), or boost::none if not available.
     */
    TIGL_EXPORT boost::optional<CTiglPoint> GetCenterOfGravityLocal() const;

    /**
     * @brief Returns the center of gravity in the global coordinate system.
     *
     * The global CoG is obtained by applying the component transformation to the local CoG.
     *
     * @return Global CoG, or boost::none if unavailable.
     */
    TIGL_EXPORT boost::optional<CTiglPoint> GetCenterOfGravityGlobal() const;

    /**
     * @brief Returns the mass inertia tensor components in the local coordinate system.
     *
     * If a CPACS @c massInertia definition is present, its values are returned.
     * Otherwise, boost::none is returned. Cross terms (Jxy, Jxz, Jyz) are optional
     * as in the CPACS schema.
     *
     * @return Local mass inertia components, or boost::none if not defined.
     */
    TIGL_EXPORT boost::optional<CTiglMassInertia> GetMassInertiaLocal() const;

protected:
    virtual PNamedShape BuildLoft() const override;

private:
    PNamedShape BuildLocalLoft() const;

    std::string _cpacsDocPath;

    struct MassCache {
        boost::optional<double> mass                   = boost::none;
        boost::optional<CTiglPoint> cogLocal           = boost::none;
        boost::optional<CTiglMassInertia> inertiaLocal = boost::none;
    };

    const CCPACSElementGeometry& GetElementGeometry() const;

    void BuildMass(MassCache& cache) const;
    Cache<MassCache, CCPACSDeckComponentBase> m_mass;

    const CCPACSDeck* m_parentDeck;
};

} // namespace tigl
