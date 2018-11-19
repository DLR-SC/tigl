/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleCowl.h"
#include "CCPACSNacelleSection.h"

namespace tigl
{

CCPACSNacelleCowl::CCPACSNacelleCowl(CCPACSEngineNacelle* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSNacelleCowl(parent, uidMgr)
    , CTiglAbstractGeometricComponent()
{}

std::string CCPACSNacelleCowl::GetDefaultedUID() const
{
    return generated::CPACSNacelleCowl::GetUID();
}

PNamedShape CCPACSNacelleCowl::BuildLoft() const
{
    // get profile curves
    std::vector<TopoDS_Wire> profiles;
    for(size_t i = 1; i <= m_sections.GetSectionCount(); ++i ) {
        CCPACSNacelleSection& section = m_sections.GetSection(i);
        profiles.push_back(section.GetTransformedWire());
    }

//    // get guide curves
//    CCPACSNacelleGuideCurves& guides = cowl.GetGuideCurves();
//    if(!guides) {
//        // make custom guide curves
//    }

    // remove blending part for rotationally symmetric interior

    // get rotation curve and generate rotationally symmetric interior

    // blend the surfaces

    // interpolate curve network

    return PNamedShape();
}

} //namespace tigl
