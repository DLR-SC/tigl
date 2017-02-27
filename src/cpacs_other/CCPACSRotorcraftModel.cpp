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

namespace tigl
{
CCPACSRotorcraftModel::CCPACSRotorcraftModel(CCPACSConfiguration* config)
    : config(config) {}

const std::string& CCPACSRotorcraftModel::GetUID() const {
    return generated::CPACSRotorcraftModel::GetUID();
}

void CCPACSRotorcraftModel::SetUID(const std::string& uid) {
    generated::CPACSRotorcraftModel::SetUID(uid);
}

// Returns the Geometric type of this component, e.g. Wing or Fuselage
TiglGeometricComponentType CCPACSRotorcraftModel::GetComponentType()
{
    return (TIGL_COMPONENT_PHYSICAL | TIGL_COMPONENT_PLANE);
}

PNamedShape CCPACSRotorcraftModel::BuildLoft()
{
    // return empty loft
    return loft;
}

void CCPACSRotorcraftModel::Invalidate() {
    if (m_wings)
        m_wings->Invalidate();
    if (m_fuselages)
        m_fuselages;
    if (m_rotors)
        m_rotors->Invalidate();
}

CCPACSConfiguration& CCPACSRotorcraftModel::GetConfiguration() const {
    return *config;
}

} // end namespace tigl
