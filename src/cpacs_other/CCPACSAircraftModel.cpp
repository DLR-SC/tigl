/* 
* Copyright (C) 2014 Airbus Defense and Space
*
* Created: 2014-09-19 Roland Landertshamer <roland.landertshamer@risc-software.at>
* Changed: $Id: CCPACSAircraftModel.cpp 1434 2016-04-15 15:41:53Z rlandert $ 
*
* Version: $Revision: 1434 $
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
* @brief  Implementation of Model for use as root component in CTiglUIDManager
*/

#include "CCPACSAircraftModel.h"

#include "CCPACSConfiguration.h"

namespace tigl
{
CCPACSAircraftModel::CCPACSAircraftModel(CCPACSConfiguration* config)
    : CTiglRelativelyPositionedComponent(NULL, NULL), config(config) {}

void CCPACSAircraftModel::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {
    generated::CPACSAircraftModel::ReadCPACS(tixiHandle, xpath);
    if (config) {
        config->GetUIDManager().AddUID(m_uID, this);
    }
}

const std::string& CCPACSAircraftModel::GetUID() const {
    return generated::CPACSAircraftModel::GetUID();
}

// Returns the Geometric type of this component, e.g. Wing or Fuselage
TiglGeometricComponentType CCPACSAircraftModel::GetComponentType() const
{
    return (TIGL_COMPONENT_PHYSICAL | TIGL_COMPONENT_PLANE);
}

PNamedShape CCPACSAircraftModel::BuildLoft()
{
    // return empty loft
    return loft;
}

void CCPACSAircraftModel::Invalidate() {
    if (m_wings)
        m_wings->Invalidate();
    if (m_fuselages)
        m_fuselages;
}

CCPACSConfiguration& CCPACSAircraftModel::GetConfiguration() const {
    return *config;
}

} // end namespace tigl
