/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#include "CCPACSDoorAssemblyPosition.h"

#include "CCPACSFuselageStructure.h"
#include "CNamedShape.h"

namespace tigl
{
CCPACSDoorAssemblyPosition::CCPACSDoorAssemblyPosition(CCPACSCargoDoorsAssembly* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDoorAssemblyPosition(parent, uidMgr)
    , CTiglStringerFrameBorderedObject(*uidMgr, *parent->GetParent()->GetParent(), generated::CPACSDoorAssemblyPosition::m_startFrameUID, generated::CPACSDoorAssemblyPosition::m_endFrameUID, generated::CPACSDoorAssemblyPosition::m_startStringerUID, generated::CPACSDoorAssemblyPosition::m_endStringerUID)
{
}

std::string CCPACSDoorAssemblyPosition::GetDefaultedUID() const
{
    return GetUID();
}

PNamedShape CCPACSDoorAssemblyPosition::GetLoft() const
{
    return PNamedShape(new CNamedShape(GetGeometry(), GetUID()));
}

TiglGeometricComponentType CCPACSDoorAssemblyPosition::GetComponentType() const
{
    return TIGL_COMPONENT_CARGO_DOOR | TIGL_COMPONENT_PHYSICAL;
}

} // namespace tigl
