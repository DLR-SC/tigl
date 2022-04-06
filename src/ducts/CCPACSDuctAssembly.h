/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-24 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "generated/CPACSDuctAssembly.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "PNamedShape.h"
#include "Cache.h"

namespace tigl
{

class CCPACSDuctAssembly : public generated::CPACSDuctAssembly, public CTiglRelativelyPositionedComponent
{
public:

    TIGL_EXPORT CCPACSDuctAssembly(CCPACSDucts* parent, CTiglUIDManager* uidMgr);

    // Given an input loft, create a new loft where all ducts have been cut away.
    TIGL_EXPORT PNamedShape LoftWithDuctCutouts(PNamedShape const&) const;

    TIGL_EXPORT void Invalidate();

private:

    // create (and cache) all ducts fused together. This can be used as a cutting tool
    void FuseDucts(PNamedShape&) const;

    Cache<PNamedShape, CCPACSDuctAssembly> fusedDucts;

};

}
