/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-04-06 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Implementation of CPACS ducts handling routines.
*/

#pragma once

#include "generated/CPACSDucts.h"
#include "PNamedShape.h"
#include <functional>
#include "CCPACSDuctAssembly.h"

namespace tigl {

class CCPACSDucts: public generated::CPACSDucts
{
public:

    TIGL_EXPORT CCPACSDucts(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr);

    // Given an input loft, create a new loft where all ducts have been cut away.
    TIGL_EXPORT PNamedShape LoftWithDuctCutouts(PNamedShape const&, std::string const &) const;

    TIGL_EXPORT CCPACSDuct const& GetDuct(std::string const& uid) const;
    TIGL_EXPORT CCPACSDuct& GetDuct(std::string const& uid);

    TIGL_EXPORT bool IsEnabled()  const;
    TIGL_EXPORT void SetEnabled(bool val=true);

    //Any AbstractGeometricComponent, that shall be cut with ducts, can register its Invalidation
    //as a callback.
    TIGL_EXPORT void RegisterInvalidationCallback(std::function<void()> const&);

private:
    bool enabled;

    std::vector<std::function<void()>> invalidationCallbacks;
};

} //namespace tigl
