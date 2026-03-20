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

#include "generated/CPACSDeck.h"
#include "CTiglRelativelyPositionedComponent.h"

namespace tigl
{

class CCPACSConfiguration;

class CCPACSDeck : public generated::CPACSDeck, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT CCPACSDeck(CCPACSDecks* parent, CTiglUIDManager* uidMgr);

    // Virtual destructor
    TIGL_EXPORT virtual ~CCPACSDeck();

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    // Returns the Component Type TIGL_COMPONENT_GENERICSYSTEM.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override
    {
        return TIGL_COMPONENT_DECK;
    }
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override
    {
        return TIGL_INTENT_PHYSICAL;
    }

protected:
    // Build the shape of the system
    PNamedShape BuildLoft() const override;
};

} // end namespace tigl
