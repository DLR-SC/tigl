/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
*
* Created: 2020 Jan Kleinert <Jan.Kleinert@dlr.de>
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
#include "CTiglPatchShell.h"
#include "CTiglError.h"
#include "CTiglPointsToBSplineInterpolation.h"
#include "CTiglCurvesToSurface.h"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRep_Builder.hxx"


TEST(CTiglPatchShell, Success)
{
    // FIRST, BUILD A SHAPE

    std::vector<Handle(Geom_Curve)> curves;

    // create points for first curve
    Handle(TColgp_HArray1OfPnt) pnt1 = new TColgp_HArray1OfPnt(1, 5);
    pnt1->SetValue(1, gp_Pnt(1., 0., 0.5));
    pnt1->SetValue(2, gp_Pnt(0., 1., -0.75));
    pnt1->SetValue(3, gp_Pnt(-1., 0., 0.25));
    pnt1->SetValue(4, gp_Pnt(0., -1., 0.));
    pnt1->SetValue(5, gp_Pnt(1., 0., -0.1));

    // interpoate points to (planar) curve
    tigl::CTiglPointsToBSplineInterpolation app1(pnt1, 3, true);
    Handle(Geom_BSplineCurve) base_curve = app1.Curve();
    curves.push_back(base_curve);

    // create points for second curve
    Handle(TColgp_HArray1OfPnt) pnt2 = new TColgp_HArray1OfPnt(1, 5);
    pnt2->SetValue(1, gp_Pnt(1., 0., 5.));
    pnt2->SetValue(2, gp_Pnt(0., 1., 5.));
    pnt2->SetValue(3, gp_Pnt(-1., 0., 5.));
    pnt2->SetValue(4, gp_Pnt(0., -1., 5.));
    pnt2->SetValue(5, gp_Pnt(1., 0., 5.));

    // interpoate points to curve
    tigl::CTiglPointsToBSplineInterpolation app2(pnt2, 3, true);
    Handle(Geom_BSplineCurve) tip_curve = app2.Curve();
    gp_Trsf T;
    T.SetScale(gp_Pnt(0., 0., 5.), 0.25);
    tip_curve->Transform(T);
    curves.push_back(tip_curve);

    // interpolate wires
    tigl::CTiglCurvesToSurface c2s(curves);
    TopoDS_Shape myShape = BRepBuilderAPI_MakeFace(c2s.Surface(), 1e-6).Face();


    // NOW PERFORM SOME TESTS

    tigl::CTiglPatchShell patcher(myShape, 1e-6);
    EXPECT_NO_THROW(TopoDS_Shape result = patcher.PatchedShape(););
}

TEST(CTiglPatchShell, emptyShape)
{
    TopoDS_Shape emptyShape;
    tigl::CTiglPatchShell patcher(emptyShape, 1e-6);
    EXPECT_THROW(patcher.PatchedShape();, tigl::CTiglError);
}

TEST(CTiglPatchShell, brokenShape)
{
    BRep_Builder b;
    TopoDS_Compound brokenShape;
    b.MakeCompound(brokenShape);
    b.Add(brokenShape, BRepBuilderAPI_MakeEdge(gp_Pnt(0., 0., 0.), gp_Pnt(0., 0., 0.1)).Edge());
    b.Add(brokenShape, BRepBuilderAPI_MakeEdge(gp_Pnt(0., 0., 0.), gp_Pnt(0., 0., -0.1)).Edge());
    tigl::CTiglPatchShell patcher(brokenShape, 1e-6);
    EXPECT_THROW(patcher.PatchedShape();, tigl::CTiglError);
}
