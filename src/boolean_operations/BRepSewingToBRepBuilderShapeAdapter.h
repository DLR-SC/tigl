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

#ifndef BREPSEWINGTOBREPBUILDERSHAPEADAPTER_H
#define BREPSEWINGTOBREPBUILDERSHAPEADAPTER_H

#include <BRepBuilderAPI_MakeShape.hxx>
#include "tigl_internal.h"

class BRepBuilderAPI_Sewing;

class BRepSewingToBRepBuilderShapeAdapter : public BRepBuilderAPI_MakeShape
{
public:
    TIGL_EXPORT BRepSewingToBRepBuilderShapeAdapter(BRepBuilderAPI_Sewing& adaptee);

    TIGL_EXPORT const TopoDS_Shape& Shape() const;
    TIGL_EXPORT operator TopoDS_Shape() const;

    TIGL_EXPORT const TopTools_ListOfShape& Modified(const TopoDS_Shape& S) OVERRIDE;

private:
    BRepBuilderAPI_Sewing& _adaptee;
    TopTools_ListOfShape _modified;
};

#endif // BREPSEWINGTOBREPBUILDERSHAPEADAPTER_H
