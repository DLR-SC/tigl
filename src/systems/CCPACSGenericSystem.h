/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-21 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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

#include "generated/CPACSGenericSystem.h"
#include "CTiglRelativelyPositionedComponent.h"

namespace tigl
{

class CCPACSConfiguration;

class CCPACSGenericSystem : public generated::CPACSGenericSystem, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT CCPACSGenericSystem(CCPACSGenericSystems* parent, CTiglUIDManager* uidMgr);

    // Virtual destructor
    TIGL_EXPORT virtual ~CCPACSGenericSystem();

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    // Returns the Component Type TIGL_COMPONENT_GENERICSYSTEM.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override
    {
        return TIGL_COMPONENT_GENERICSYSTEM;
    }
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override
    {
        return TIGL_INTENT_PHYSICAL;
    }

    /**
     * @brief Returns the total mass of all components in the system.
     *
     * The mass is computed as the sum of the masses of all contained
     * @c CCPACSComponent instances.
     *
     * Components without a valid mass definition contribute zero mass.
     * Positioning of the components is not required.
     *
     * @return Total mass of all components in the system.
     */
    TIGL_EXPORT double GetMassAllComponents() const;

    /**
     * @brief Returns the total mass of all explicitly positioned components.
     *
     * The mass is computed as the sum of the masses of all contained
     * @c CCPACSComponent instances for which IsPositioned() returns true.
     *
     * Components without a valid mass definition contribute zero mass.
     *
     * @return Total mass of all positioned components in the system.
     */
    TIGL_EXPORT double GetMassPositionedComponents() const;

    /**
     * @brief Returns the center of gravity of the system.
     *
     * The system CoG is computed as the mass-weighted average of the global
     * centers of gravity of all contained @c CCPACSComponent instances.
     *
     * Only components that satisfy all of the following conditions are considered:
     * - The component is explicitly positioned (see CCPACSComponent::IsPositioned()).
     * - A valid positive mass is available.
     * - A global center of gravity can be determined.
     *
     * If no contributing mass is available, boost::none is returned.
     *
     * @return Global system center of gravity, or boost::none if undefined.
     */
    TIGL_EXPORT boost::optional<CTiglPoint> GetCenterOfGravity() const;

protected:
    // Build the shape of the system
    PNamedShape BuildLoft() const override;

private:
    // get short name for loft
    std::string GetShortShapeName() const;
};

} // end namespace tigl
