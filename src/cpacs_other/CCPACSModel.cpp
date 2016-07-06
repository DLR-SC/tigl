/* 
* Copyright (C) 2014 Airbus Defense and Space
*
* Created: 2014-09-19 Roland Landertshamer <roland.landertshamer@risc-software.at>
* Changed: $Id: CCPACSModel.cpp 1434 2016-04-15 15:41:53Z rlandert $ 
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

#include "CCPACSModel.h"

namespace tigl
{
CCPACSModel::CCPACSModel() : config(nullptr) {}

CCPACSModel::CCPACSModel(CCPACSConfiguration* config) : config(config) {}

CCPACSModel::~CCPACSModel(void) {}

const std::string& CCPACSModel::GetUID() const {
    static const std::string s_uid = "Model";
    return s_uid;
}

void CCPACSModel::SetUID(const std::string& uid) {}

TiglSymmetryAxis CCPACSModel::GetSymmetryAxis(void) {
    return TiglSymmetryAxis::TIGL_NO_SYMMETRY;
}

void CCPACSModel::SetSymmetryAxis(const TiglSymmetryAxis& axis) {}

// Returns the Geometric type of this component, e.g. Wing or Fuselage
TiglGeometricComponentType CCPACSModel::GetComponentType(void)
{
    return (TIGL_COMPONENT_PHYSICAL | TIGL_COMPONENT_PLANE);
}

PNamedShape CCPACSModel::BuildLoft(void)
{
    // return empty loft
    return loft;
}

void CCPACSModel::Invalidate() {
    if (m_wings.isValid())
        m_wings->Invalidate();
    if (m_fuselages.isValid())
        m_fuselages.isValid();
}

CCPACSConfiguration& CCPACSModel::GetConfiguration() const {
    return *config;
}

} // end namespace tigl
