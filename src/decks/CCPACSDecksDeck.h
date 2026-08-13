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

#include "generated/CPACSDecksDeck.h"
#include "CTiglRelativelyPositionedComponent.h"

namespace tigl
{

class CCPACSConfiguration;

class CCPACSDecksDeck : public generated::CPACSDecksDeck, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT CCPACSDecksDeck(CCPACSDecks* parent, CTiglUIDManager* uidMgr);

    // Virtual destructor
    TIGL_EXPORT virtual ~CCPACSDecksDeck();

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration const& GetConfiguration() const;

    // Returns the Component Type TIGL_COMPONENT_GENERICSYSTEM.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;

    /// @brief Returns the TiGL geometric component intent.
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

protected:
    // Build the shape of the system
    PNamedShape BuildLoft() const override;

private:
    // get short name for loft
    std::string GetShortShapeName() const;
};

} // end namespace tigl
