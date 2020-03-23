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
/**
* @file
* @brief  Implementation of CPACS wing handling routines.
*/

#ifndef CCPACSGENERICSYSTEM_H
#define CCPACSGENERICSYSTEM_H

#include <string>
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
    TIGL_EXPORT CCPACSConfiguration & GetConfiguration() const;

    // Returns the Component Type TIGL_COMPONENT_GENERICSYSTEM.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override {return TIGL_COMPONENT_GENERICSYSTEM;}
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override {return TIGL_INTENT_PHYSICAL;}

protected:
    // Build the shape of the system
    PNamedShape BuildLoft() const override;

private:
    // get short name for loft
    std::string GetShortShapeName() const;

    // Copy constructor
    CCPACSGenericSystem(const CCPACSGenericSystem & );

    // Assignment operator
    void operator=(const CCPACSGenericSystem & );
};

} // end namespace tigl

#endif // CCPACSGENERICSYSTEM_H
