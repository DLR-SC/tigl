/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-15 Anton Reiswich <Anton.Reiswich@dlr.de>
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
* @brief  Implementation of CPACS duct handling routines.
*/

#include "CCPACSDuct.h"
#include "generated/CPACSDucts.h"

namespace tigl {

CCPACSDuct::CCPACSDuct(CCPACSDucts* parent, CTiglUIDManager* uidMgr)
  : generated::CPACSDuct(parent, uidMgr)
  , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
{}

CCPACSConfiguration& CCPACSDuct::GetConfiguration() const
{
    return GetParent()->GetParent()->GetConfiguration();
}

std::string CCPACSDuct::GetDefaultedUID() const
{
    return generated::CPACSDuct::GetUID();
}

TiglGeometricComponentType CCPACSDuct::GetComponentType() const
{
    return TIGL_COMPONENT_DUCT;
}

TiglGeometricComponentIntent CCPACSDuct::GetComponentIntent() const
{
    return TIGL_INTENT_LOGICAL;
}

PNamedShape CCPACSDuct::BuildLoft() const
{
    throw CTiglError("Not implemented yet");
}

} //namespace tigl
