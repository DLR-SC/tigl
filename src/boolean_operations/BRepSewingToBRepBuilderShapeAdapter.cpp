/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-11-11 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "BRepSewingToBRepBuilderShapeAdapter.h"

#include <BRepBuilderAPI_Sewing.hxx>
#include <TopExp.hxx>

BRepSewingToBRepBuilderShapeAdapter::BRepSewingToBRepBuilderShapeAdapter(BRepBuilderAPI_Sewing& adaptee)
    : _adaptee(adaptee)
{
}

const TopoDS_Shape &BRepSewingToBRepBuilderShapeAdapter::Shape() const
{
    return _adaptee.SewedShape();
}

BRepSewingToBRepBuilderShapeAdapter::operator TopoDS_Shape() const
{
    return Shape();
}

const TopTools_ListOfShape& BRepSewingToBRepBuilderShapeAdapter::Modified(const TopoDS_Shape &S)
{
    _modified.Clear();
    TopoDS_Shape modshape = _adaptee.Modified(S);
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(modshape, TopAbs_FACE, map);
    for (int iface = 1; iface <= map.Extent(); ++iface) {
        _modified.Append(map(iface));
    }

    return _modified;
}
