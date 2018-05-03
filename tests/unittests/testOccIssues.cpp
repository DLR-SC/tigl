/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-03-05 Martin Siggel <Martin.Siggel@dlr.de>
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


#include "test.h" // Brings in the GTest framework

#include "tiglcommonfunctions.h"

#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <Geom_Curve.hxx>

/**
 * @brief This test case checks a bug in OpenCASCADE < 6.9.0
 * TiGl requires accurate functionality. If this function fails,
 * OpenCASCADE has to be patched with the patch under
 * patches/oce-0.17/fix-geomapi_extremacurvecurve.patch
 */
TEST(OccIssues, GeomAPI_ExtremaCurveCurve)
{
    BRep_Builder b;
    TopoDS_Shape curveShape;
    BRepTools::Read(curveShape, "TestData/bugs/418/occ-intersection-bug.brep", b);

    TopoDS_Edge c1Edge = GetEdge(curveShape, 0);
    TopoDS_Edge c2Edge = GetEdge(curveShape, 1);

    double umin, umax;
    Handle(Geom_Curve) c1 = BRep_Tool::Curve(c1Edge, umin, umax);
    Handle(Geom_Curve) c2 = BRep_Tool::Curve(c2Edge, umin, umax);
    
    GeomAPI_ExtremaCurveCurve extrema(c1, c2);
    // There are two intersections. One at the start of c1, the other at the end of c1
    EXPECT_EQ(2, extrema.NbExtrema());

    gp_Pnt p1, p2;
    extrema.Points(1, p1, p2);
    EXPECT_NEAR(c1->FirstParameter(), p1.Distance(p2), 1e-4);
    double u1, u2;
    extrema.Parameters(1, u1, u2);
    EXPECT_NEAR(0.0, u1, 1e-6);
    EXPECT_NEAR(0.215758486, u2, 1e-6);

    extrema.Points(2, p1, p2);
    EXPECT_NEAR(0., p1.Distance(p2), 1e-4);
    extrema.Parameters(2, u1, u2);
    EXPECT_NEAR(c1->LastParameter(), u1, 1e-6);
    EXPECT_NEAR(0.215758486, u2, 1e-6);
}
