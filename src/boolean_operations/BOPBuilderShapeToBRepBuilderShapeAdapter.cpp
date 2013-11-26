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

#include "BOPBuilderShapeToBRepBuilderShapeAdapter.h"

#include <BOPAlgo_BuilderShape.hxx>

BOPBuilderShapeToBRepBuilderShapeAdapter::BOPBuilderShapeToBRepBuilderShapeAdapter(BOPAlgo_BuilderShape& adaptee)
    : _adaptee(adaptee) {
}

const TopoDS_Shape &BOPBuilderShapeToBRepBuilderShapeAdapter::Shape() const {
    return _adaptee.Shape();
}

BOPBuilderShapeToBRepBuilderShapeAdapter::operator TopoDS_Shape() const {
    return Shape();
}


const TopTools_ListOfShape& BOPBuilderShapeToBRepBuilderShapeAdapter::Generated(const TopoDS_Shape &S) {
    return _adaptee.Generated(S);
}

const TopTools_ListOfShape& BOPBuilderShapeToBRepBuilderShapeAdapter::Modified(const TopoDS_Shape &S) {
    return _adaptee.Modified(S);
}

Standard_Boolean BOPBuilderShapeToBRepBuilderShapeAdapter::IsDeleted(const TopoDS_Shape &S) {
    return _adaptee.IsDeleted(S);
}
