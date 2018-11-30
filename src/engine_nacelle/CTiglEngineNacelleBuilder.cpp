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

namespace tigl {

CTiglEngineNacelleBuilder::CTiglEngineNacelleBuilder(const CCPACSEngineNacelle& nacelle)
    : m_nacelle(nacelle)
{};

PNamedShape CTiglEngineNacelleBuilder::BuildShape()
{
    PNamedShape fanshape = m_nacelle.GetFanCowl().GetLoft();
/*
    TopoDS_Compound c;
    TopoDS_Builder b;
    b.MakeCompound(c);
    b.Add(c, fanshape->Shape());

    if ( m_nacelle.GetCoreCowl() ) {
        PNamedShape coreshape = m_nacelle.GetCoreCowl()->GetLoft();
        b.Add(c, coreshape->Shape());
    }

    PNamedShape compoundShape( new CNamedShape(c, m_nacelle.GetUID().c_str()) );
*/
    PNamedShape compoundShape( new CNamedShape(fanshape->Shape(), m_nacelle.GetUID().c_str()) );
    return compoundShape;
};

CTiglEngineNacelleBuilder::operator PNamedShape()
{
    return BuildShape();
};

} //namespace tigl
