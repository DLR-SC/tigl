/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-22 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "generated/CPACSEngineNacelle.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSEngineNacelle : public generated::CPACSEngineNacelle, public CTiglRelativelyPositionedComponent
{
public:
    // Constructor
    TIGL_EXPORT CCPACSEngineNacelle(CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE { return TIGL_COMPONENT_ENGINE_NACELLE; }
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE { return TIGL_INTENT_PHYSICAL; }

protected:
    virtual PNamedShape BuildLoft() const OVERRIDE;
};

} //namespae tigl
