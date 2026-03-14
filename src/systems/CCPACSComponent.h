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

#pragma once

#include "generated/CPACSComponent.h"
#include "generated/CPACSMassInertia.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CTiglPoint.h"
#include "CTiglMassInertia.h"

namespace tigl
{
class CCPACSConfiguration;

/**
 * @brief Geometric component representing a CPACS <component> within systems.
 *
 * A CCPACSComponent references a system element via @c systemElementUID and provides
 * geometric and mass properties derived from that referenced element.
 *
 * Mass properties:
 * - The mass can be given explicitly or computed from a density and the component volume.
 * - The center of gravity (CoG) can be given explicitly via CPACS @c location.
 *   If not provided, it is derived from the geometric centroid of the component.
 *
 * Coordinate frames:
 * - Local values (mass, CoG local) are expressed in the component's local coordinate system.
 * - Global CoG is only available if the component is explicitly positioned via a
 *   CPACS @c <transformation> element (see IsPositioned()).
 */
class CCPACSComponent : public generated::CPACSComponent, public CTiglRelativelyPositionedComponent
{
public:
    /**
     * @brief Constructs a CCPACSComponent.
     * @param parent Parent CPACS <components> container.
     * @param uidMgr UID manager for resolving referenced system elements.
     */
    TIGL_EXPORT CCPACSComponent(CCPACSComponents* parent, CTiglUIDManager* uidMgr);

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
        return TIGL_COMPONENT_SYSTEM_COMPONENT;
    }

    /// @brief Returns the TiGL geometric component intent.
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override
    {
        return TIGL_INTENT_PHYSICAL;
    }

    /**
     * @brief Returns the geometric representation type of the referenced system element.
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
     * This value is only available if the component is explicitly positioned
     * (see IsPositioned()) and a local CoG exists.
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

    /**
     * @brief Returns whether this component is explicitly positioned in CPACS.
     *
     * This checks for the presence of the optional CPACS @c <transformation> element
     * under the component.
     *
     * @return true if an explicit transformation is present, false otherwise.
     */
    TIGL_EXPORT bool IsPositioned() const;

protected:
    virtual PNamedShape BuildLoft() const override;

private:
    std::string _cpacsDocPath;

    struct MassCache {
        boost::optional<double> mass                   = boost::none;
        boost::optional<CTiglPoint> cogLocal           = boost::none;
        boost::optional<CTiglMassInertia> inertiaLocal = boost::none;
    };

    const CCPACSElementGeometry& GetElementGeometry() const;

    void BuildMass(MassCache& cache) const;
    Cache<MassCache, CCPACSComponent> m_mass;
};

} // namespace tigl
