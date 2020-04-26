/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
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

#include "test.h"

#include <CTiglTopoAlgorithms.h>
#include <CTiglTransformation.h>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <gp_Pnt.hxx>

TEST(CTiglTopoAlgorithms, IsDegenerated)
{
    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(gp_Pnt(0., 0., 0.), gp_Pnt(1., 0., 0.)).Edge();
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(gp_Pnt(1., 0., 0.), gp_Pnt(1., 1., 0.)).Edge();

    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(e1, e2).Wire();

    EXPECT_FALSE(tigl::CTiglTopoAlgorithms::IsDegenerated(wire));

    tigl::CTiglTransformation trafo;
    trafo.AddScaling(0., 0., 0.);

    wire = TopoDS::Wire(trafo.Transform(wire));
    EXPECT_TRUE(tigl::CTiglTopoAlgorithms::IsDegenerated(wire));
}
