/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-08 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CGroupShapes.h"

#include "CNamedShape.h"
#include "CBooleanOperTools.h"

#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>

namespace tigl
{

CGroupShapes::CGroupShapes(const ListPNamedShape& list)
{
    if (list.size() == 1) {
        _result = list[0];
        return;
    }

    BRep_Builder b;
    TopoDS_Compound c;
    b.MakeCompound(c);

    for (ListPNamedShape::const_iterator it=list.begin(); it != list.end(); ++it) {
        PNamedShape shape = *it;
        if (!shape) {
            continue;
        }

        b.Add(c, shape->Shape());
    }

    _result = PNamedShape(new CNamedShape(c, "ShapeGroup"));

    // apply face names from the shapes
    for (ListPNamedShape::const_iterator it=list.begin(); it != list.end(); ++it) {
        PNamedShape shape = *it;
        if (!shape) {
            continue;
        }

        CBooleanOperTools::AppendNamesToShape(shape, _result);
    }
}

CGroupShapes::~CGroupShapes()
{
}

tigl::CGroupShapes::operator PNamedShape()
{
    return NamedShape();
}

const PNamedShape CGroupShapes::NamedShape()
{
    return _result;
}

} // namespace tigl
