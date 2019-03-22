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
#include "UniquePtr.h"
#include "CNamedShape.h"
#include "TopoDS_Compound.hxx"
#include "TopoDS_Builder.hxx"
#include "BRepBuilderAPI_Transform.hxx"

namespace tigl {

CTiglEngineNacelleBuilder::CTiglEngineNacelleBuilder(const CCPACSEngineNacelle& nacelle)
    : m_nacelle(nacelle)
{};

CTiglEngineNacelleBuilder::CTiglEngineNacelleBuilder(const CCPACSEngineNacelle& nacelle, const CTiglTransformation& transformation)
    : m_nacelle(nacelle)
    , m_transformation(&transformation)
{};

PNamedShape CTiglEngineNacelleBuilder::BuildShape()
{
    std::vector<PNamedShape> shapes;

    // add fan
    shapes.push_back(m_nacelle.GetFanCowl().BuildLoft());

    // add core cowl
    if ( m_nacelle.GetCoreCowl() ) {
        shapes.push_back(m_nacelle.GetCoreCowl()->BuildLoft());
    }

    //add center cowl
    if ( m_nacelle.GetCenterCowl() ) {
        shapes.push_back(m_nacelle.GetCenterCowl()->BuildLoft());
    }


    // add shapes to compound
    TopoDS_Compound compound;
    TopoDS_Builder builder;
    builder.MakeCompound(compound);
    for(size_t i = 0; i< shapes.size(); ++i) {
        // if we have a transformation, apply it.
        if (m_transformation) {
            builder.Add(compound, m_transformation->Transform(shapes[i]->Shape()));
        }
        else {
            builder.Add(compound, shapes[i]->Shape());
        }
    }

    PNamedShape compoundShape( new CNamedShape(compound, m_nacelle.GetUID().c_str()) );
    return compoundShape;
};

CTiglEngineNacelleBuilder::operator PNamedShape()
{
    return BuildShape();
};

} //namespace tigl
