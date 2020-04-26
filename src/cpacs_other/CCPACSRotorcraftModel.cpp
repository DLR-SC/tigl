/* 
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
* @brief  Implementation of Model for use as root component in CTiglUIDManager
*/

#include "CCPACSRotorcraftModel.h"

#include "CCPACSConfiguration.h"

namespace tigl
{
CCPACSRotorcraftModel::CCPACSRotorcraftModel(CCPACSConfiguration* config)
    : generated::CPACSRotorcraftModel(nullptr, config ? &config->GetUIDManager() : NULL), CTiglRelativelyPositionedComponent(static_cast<std::string*>(NULL), NULL), config(config) {}

CCPACSRotorcraftModel::CCPACSRotorcraftModel(CCPACSRotorcraft* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSRotorcraftModel(parent, uidMgr), CTiglRelativelyPositionedComponent(static_cast<std::string*>(NULL), NULL), config(NULL) {}

std::string CCPACSRotorcraftModel::GetDefaultedUID() const {
    return generated::CPACSRotorcraftModel::GetUID();
}

// Returns the Geometric type of this component, e.g. Wing or Fuselage
TiglGeometricComponentType CCPACSRotorcraftModel::GetComponentType() const
{
    return TIGL_COMPONENT_PLANE;
}

TiglGeometricComponentIntent CCPACSRotorcraftModel::GetComponentIntent() const
{
    return TIGL_INTENT_PHYSICAL;
}


PNamedShape CCPACSRotorcraftModel::BuildLoft() const
{
    return PNamedShape();
}

CCPACSConfiguration& CCPACSRotorcraftModel::GetConfiguration() const {
    return *config;
}

} // end namespace tigl
