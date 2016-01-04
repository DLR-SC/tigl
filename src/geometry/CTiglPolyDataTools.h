/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-12 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLPOLYDATATOOLS_H
#define CTIGLPOLYDATATOOLS_H


#include "tigl_internal.h"
#include <TopoDS_Shape.hxx>
#include <CTiglPolyData.h>

#include <Poly_Triangulation.hxx>

namespace tigl 
{

class CTiglPolyDataTools
{
public:
    TIGL_EXPORT CTiglPolyDataTools();

    TIGL_EXPORT static TopoDS_Shape MakeTopoDS(CTiglPolyData& mesh);

    TIGL_EXPORT static Handle(Poly_Triangulation) MakePoly_Triangulation(CTiglPolyData& mesh);
};

} // namespace tigl

#endif // CTIGLPOLYDATATOOLS_H
