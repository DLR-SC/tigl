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

#include "CTiglEngineNacelleBuilder.h"
#include "generated/CPACSNacelleSections.h"
#include "CCPACSNacelleSection.h"
#include "UniquePtr.h"

namespace tigl {

CTiglEngineNacelleBuilder::CTiglEngineNacelleBuilder(const generated::CPACSEngineNacelle& nacelle)
    : m_nacelle(nacelle)
{};

PNamedShape CTiglEngineNacelleBuilder::BuildShape()
{
    TopoDS_Shape fanCowlShape = BuildNacelleCowl(m_nacelle.GetFanCowl());
    //TODO
    // If coreCowl exists, build coreCowl
    // Add both to PNamedShape();
    return PNamedShape();

};

TopoDS_Shape CTiglEngineNacelleBuilder::BuildNacelleCowl(const CCPACSNacelleCowl& cowl)
{
    // get profile curves
    std::vector<TopoDS_Wire> profiles;
    for(size_t i = 1; i <= cowl.GetSections().GetSectionCount(); ++i ) {
        CCPACSNacelleSection& section = cowl.GetSections().GetSection(i);
        profiles.push_back(section.GetTransformedWire());
    }

//    // get guide curves
//    CCPACSNacelleGuideCurves& guides = cowl.GetGuideCurves();
//    if(!guides) {
//        // make custom guide curves
//    }

    return TopoDS_Shape();
}

CTiglEngineNacelleBuilder::operator PNamedShape()
{
    return BuildShape();
};

} //namespace tigl
