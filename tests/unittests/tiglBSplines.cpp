/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-06-01 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CPointsToLinearBSpline.h"
#include "CTiglSymetricSplineBuilder.h"
#include "CTiglError.h"
#include <Geom_BSplineCurve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include "CTiglBSplineApproxInterp.h"
#include "stringtools.h"
#include "UniquePtr.h"
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <cmath>

TEST(BSplines, pointsToLinear)
{
    std::vector<gp_Pnt> points;
    points.push_back(gp_Pnt(0,0,0));
    points.push_back(gp_Pnt(2,0,0));
    points.push_back(gp_Pnt(2,1,0));
    points.push_back(gp_Pnt(1,1,0));
    
    Handle(Geom_BSplineCurve) curve;
    ASSERT_NO_THROW(curve = tigl::CPointsToLinearBSpline(points));
    
    ASSERT_NEAR(0, curve->Value(0.  ).Distance(gp_Pnt(0,0,0)), 1e-10);
    ASSERT_NEAR(0, curve->Value(0.5 ).Distance(gp_Pnt(2,0,0)), 1e-10);
    ASSERT_NEAR(0, curve->Value(0.75).Distance(gp_Pnt(2,1,0)), 1e-10);
    ASSERT_NEAR(0, curve->Value(1.  ).Distance(gp_Pnt(1,1,0)), 1e-10);
    
    gp_Pnt p; gp_Vec v;
    curve->D1(0.25, p, v);
    ASSERT_NEAR(v.Magnitude(), v*gp_Vec(1,0,0), 1e-10);
    
    curve->D1(0.6, p, v);
    ASSERT_NEAR(v.Magnitude(), v*gp_Vec(0,1,0), 1e-10);
    
    curve->D1(0.8, p, v);
    ASSERT_NEAR(v.Magnitude(), v*gp_Vec(-1,0,0), 1e-10);
}


/// This tests checks, if the bspline
/// has symmetric properties
TEST(BSplines, symmetricBSpline)
{
    // symmetric input points wrt x-y plane
    std::vector<gp_Pnt> points;
    points.push_back(gp_Pnt(0,0,1));
    points.push_back(gp_Pnt(0,-sqrt(0.5),sqrt(0.5)));
    points.push_back(gp_Pnt(0,-1,0));
    points.push_back(gp_Pnt(0,-sqrt(0.5),-sqrt(0.5)));
    points.push_back(gp_Pnt(0,0,-1));

    Handle(Geom_BSplineCurve) curve;
    tigl::CTiglSymetricSplineBuilder builder(points);

    ASSERT_NO_THROW(curve = builder.GetBSpline());

    double umin = curve->FirstParameter();
    double umax = curve->LastParameter();

    // check, that curve start is perpendicular to x-z plane
    gp_Pnt p; gp_Vec v;
    curve->D1(umin, p, v);
    ASSERT_NEAR(0., v * gp_Vec(1,0,0), 1e-10);
    ASSERT_NEAR(0., v * gp_Vec(0,0,1), 1e-10);

    // check interpolation at start
    ASSERT_NEAR(0., p.Distance(gp_Pnt(0,0,1)), 1e-10);

    // check, that curve end is perpendicular to x-z plane
    curve->D1(umax, p, v);
    ASSERT_NEAR(0., v * gp_Vec(1,0,0), 1e-10);
    ASSERT_NEAR(0., v * gp_Vec(0,0,1), 1e-10);

    // check interpolation at end
    ASSERT_NEAR(0., p.Distance(points[points.size()-1]), 1e-10);
    
    // check symmetry wrt x-y plane
    curve->D1((umin+umax)/2., p, v);
    ASSERT_NEAR(0., p.Distance(gp_Pnt(0,-1,0)), 1e-10);
    ASSERT_NEAR(0., v * gp_Vec(0,1,0), 1e-10);
    ASSERT_NEAR(0., v * gp_Vec(1,0,0), 1e-10);
}

/// This tests checks, if the bspline
/// is really going through the points
TEST(BSplines, symmetricBSpline_interp)
{
    std::vector<gp_Pnt> points;
    points.push_back(gp_Pnt(0,0,1));
    points.push_back(gp_Pnt(0,-0.2, 0.8));
    points.push_back(gp_Pnt(0,-sqrt(0.5),sqrt(0.5)));
    points.push_back(gp_Pnt(0,-1,0));
    points.push_back(gp_Pnt(0,-sqrt(0.5),-sqrt(0.5)));
    points.push_back(gp_Pnt(0,0,-1));

    Handle(Geom_BSplineCurve) curve;
    tigl::CTiglSymetricSplineBuilder builder(points);

    ASSERT_NO_THROW(curve = builder.GetBSpline());

    GeomAPI_ProjectPointOnCurve proj;
    proj.Init(curve, curve->FirstParameter(), curve->LastParameter());
    for (unsigned int i = 0; i < points.size(); ++i) {
        gp_Pnt p = points[i];
        proj.Perform(p);
        ASSERT_GE(proj.NbPoints(), 1);
        ASSERT_LT(proj.LowerDistance(), 1e-10);
    }
}

/// The CTiglSymetricSplineBuilder algorithm requires
/// that the first and second point must have
/// a zero y-coordinate. Test, these cases
TEST(BSplines, symmetricBSpline_invalidInput)
{
    std::vector<gp_Pnt> points;
    // the first point should have y == 0
    points.push_back(gp_Pnt(0,-0.1 ,1));
    points.push_back(gp_Pnt(0,-sqrt(0.5),sqrt(0.5)));
    points.push_back(gp_Pnt(0,-1,0));
    points.push_back(gp_Pnt(0,-sqrt(0.5),-sqrt(0.5)));
    points.push_back(gp_Pnt(0,0,-1));

    Handle(Geom_BSplineCurve) curve;
    tigl::CTiglSymetricSplineBuilder builder(points);

    ASSERT_THROW(curve = builder.GetBSpline(), tigl::CTiglError);
}

TEST(BSplines, symmetricBSpline_invalidInput2)
{
    std::vector<gp_Pnt> points;
    points.push_back(gp_Pnt(0, 0 ,1));
    points.push_back(gp_Pnt(0,-sqrt(0.5),sqrt(0.5)));
    points.push_back(gp_Pnt(0,-1,0));
    points.push_back(gp_Pnt(0,-sqrt(0.5),-sqrt(0.5)));
    // the last point should have y == 0
    points.push_back(gp_Pnt(0,-0.1,-1));

    Handle(Geom_BSplineCurve) curve;
    tigl::CTiglSymetricSplineBuilder builder(points);

    ASSERT_THROW(curve = builder.GetBSpline(), tigl::CTiglError);
}

class BSplineInterpolation : public ::testing::Test
{
protected:
    BSplineInterpolation()
        : pnts(1, 101)
    {}

    void SetUp() OVERRIDE
    {
        // load in the airfoil point example
        std::string line;
        std::ifstream file ("TestData/airfoil_points.txt");
        Standard_Integer irow = 1;
        while(!file.eof()) {
            std::getline(file, line);
            std::vector<std::string> cols = tigl::split_string(line, ' ');
            if (cols.size() != 8) {
                continue;
            }
            double px, py, pz, t;
            tigl::from_string<>(cols[0], px);
            tigl::from_string<>(cols[2], py);
            tigl::from_string<>(cols[4], pz);
            tigl::from_string<>(cols[7], t);
            pnts.SetValue(irow, gp_Pnt(px, py, pz));
            parms.push_back(t);
            irow++;
        }
        ASSERT_EQ(parms.size(), pnts.Length());
    }

    void TearDown() OVERRIDE
    {
    }

    void StoreResult(const std::string& filename, const Handle(Geom_BSplineCurve)& curve, const TColgp_Array1OfPnt& pt)
    {
        TopoDS_Compound c;
        BRep_Builder b;
        b.MakeCompound(c);

        TopoDS_Shape e = BRepBuilderAPI_MakeEdge(curve);
        b.Add(c, e);

        for (Standard_Integer i = pt.Lower(); i <= pt.Upper(); ++i) {
            const gp_Pnt& p = pt.Value(i);
            TopoDS_Shape v = BRepBuilderAPI_MakeVertex(p);
            b.Add(c, v);
        }

        BRepTools::Write(c, filename.c_str());
    }

    std::vector<double> parms;
    TColgp_Array1OfPnt pnts;
};

TEST_F(BSplineInterpolation, approxAndInterpolate)
{
    tigl::CTiglBSplineApproxInterp app(pnts, 30, 3);
    app.InterpolatePoint(0);
    app.InterpolatePoint(50);
    app.InterpolatePoint(100);
    tigl::CTiglApproxResult result = app.FitCurve(parms);
    // Value from different splinelib implementation
    EXPECT_NEAR(0.01317089, result.error, 1e-5);
    EXPECT_NEAR(0.0, result.curve->Value(parms[0]).Distance(pnts.Value(1)), 1e-10);
    EXPECT_NEAR(0.0, result.curve->Value(parms[50]).Distance(pnts.Value(51)), 1e-10);
    EXPECT_NEAR(0.0, result.curve->Value(parms[100]).Distance(pnts.Value(101)), 1e-10);

    result = app.FitCurveOptimal(parms);
    EXPECT_NEAR(0.000393704, result.error, 1e-5);
    EXPECT_NEAR(0.0, result.curve->Value(parms[0]).Distance(pnts.Value(1)), 1e-10);
    EXPECT_NEAR(0.0, result.curve->Value(parms[50]).Distance(pnts.Value(51)), 1e-10);
    EXPECT_NEAR(0.0, result.curve->Value(parms[100]).Distance(pnts.Value(101)), 1e-10);

    StoreResult("TestData/analysis/BSplineInterpolation-approxAndInterpolate.brep", result.curve, pnts);
}

TEST_F(BSplineInterpolation, approxAndInterpolateContinuous)
{
    int nPoints = 21;
    TColgp_Array1OfPnt pnt2(1, nPoints);

    for (int i = 0; i < nPoints; ++i) {
        pnt2.SetValue(i + 1, gp_Pnt(cos((i * 2.*M_PI) / static_cast<double>(nPoints - 1)),
                                    0.,
                                    sin((i * 2.*M_PI) / static_cast<double>(nPoints - 1))));
    }

    tigl::CTiglBSplineApproxInterp app(pnt2, 9, 3, true);
    app.InterpolatePoint(0);
    app.InterpolatePoint(nPoints - 1);
    tigl::CTiglApproxResult result = app.FitCurve();
    Handle(Geom_BSplineCurve) curve = result.curve;

    // tests if closed curve is C2-continuous at the boundaries:
    gp_Pnt p1;
    gp_Vec deriv1_1;
    gp_Vec deriv1_2;
    curve->D2(0., p1, deriv1_1, deriv1_2);
    gp_Pnt p2;
    gp_Vec deriv2_1;
    gp_Vec deriv2_2;
    curve->D2(1., p2, deriv2_1, deriv2_2);

    EXPECT_TRUE(p1.IsEqual(p2, 1e-10));
    EXPECT_TRUE(deriv1_1.IsEqual(deriv2_1, 1e-10, 1e-10));
    EXPECT_TRUE(deriv1_2.IsEqual(deriv2_2, 1e-10, 1e-10));

    StoreResult("TestData/analysis/BSplineInterpolation-approxAndInterpolateContinuous.brep", curve, pnt2);
}

TEST_F(BSplineInterpolation, approxOnly)
{
    tigl::CTiglBSplineApproxInterp app(pnts, 15, 3);
    tigl::CTiglApproxResult result = app.FitCurve(parms);
    // Value from different splinelib implementation
    EXPECT_NEAR(0.01898, result.error, 1e-5);

    result = app.FitCurveOptimal(parms);
    EXPECT_NEAR(0.00238, result.error, 1e-5);

    StoreResult("TestData/analysis/BSplineInterpolation-approxOnly.brep", result.curve, pnts);
}

TEST_F(BSplineInterpolation, gordonIssue)
{
    tigl::CTiglBSplineApproxInterp app(pnts, 31, 3);
    app.InterpolatePoint(0);
    app.InterpolatePoint(100);
    tigl::CTiglApproxResult result = app.FitCurve(parms);
    EXPECT_NEAR(0.0, result.curve->Value(parms[0]).Distance(pnts.Value(1)), 1e-10);
    EXPECT_NEAR(0.0, result.curve->Value(parms[100]).Distance(pnts.Value(101)), 1e-10);
    EXPECT_NEAR(0.0055298, result.error, 1e-5);

    StoreResult("TestData/analysis/BSplineInterpolation-gordonIssue.brep", result.curve, pnts);
}

TEST_F(BSplineInterpolation, ownParms)
{
    tigl::CTiglBSplineApproxInterp app(pnts, 31, 3);
    app.InterpolatePoint(0);
    app.InterpolatePoint(100);
    tigl::CTiglApproxResult result = app.FitCurve(parms);
    result = app.FitCurveOptimal();
    EXPECT_NEAR(0.0, result.curve->Value(parms[0]).Distance(pnts.Value(1)), 1e-10);
    EXPECT_NEAR(0.0, result.curve->Value(parms[100]).Distance(pnts.Value(101)), 1e-10);

    StoreResult("TestData/analysis/BSplineInterpolation-ownParms.brep", result.curve, pnts);
}

TEST_F(BSplineInterpolation, tipKink)
{
    tigl::CTiglBSplineApproxInterp app(pnts, 31, 3);
    app.InterpolatePoint(0);
    app.InterpolatePoint(50, true);
    app.InterpolatePoint(100);
    tigl::CTiglApproxResult result = app.FitCurveOptimal();
    EXPECT_NEAR(0.0, result.curve->Value(parms[0]).Distance(pnts.Value(1)), 1e-10);
    EXPECT_NEAR(0.0, result.curve->Value(parms[100]).Distance(pnts.Value(101)), 1e-10);

    StoreResult("TestData/analysis/BSplineInterpolation-tipKink.brep", result.curve, pnts);
}

TEST_F(BSplineInterpolation, tipKink2)
{
    std::vector<double> parms2;
    parms2.push_back(0. / 4.);
    parms2.push_back(0.5 / 4.);
    parms2.push_back(1.0 / 4.);
    parms2.push_back(1.5 / 4.);
    parms2.push_back(2.0 / 4.);
    parms2.push_back(2.5 / 4.);
    parms2.push_back(3.0 / 4.);
    parms2.push_back(3.5 / 4.);
    parms2.push_back(4.0 / 4.);

    TColgp_Array1OfPnt pnt2(1, 9);
    pnt2.SetValue(1, gp_Pnt(0., 0., 0.));
    pnt2.SetValue(2, gp_Pnt(0.5, 0., -0.2));
    pnt2.SetValue(3, gp_Pnt(1.0, 0., 0.));
    pnt2.SetValue(4, gp_Pnt(0.8, 0., 0.5));
    pnt2.SetValue(5, gp_Pnt(1.0, 0., 1.0));
    pnt2.SetValue(6, gp_Pnt(0.5, 0., 1.2));
    pnt2.SetValue(7, gp_Pnt(0.0, 0., 1.0));
    pnt2.SetValue(8, gp_Pnt(0.2, 0., 0.5));
    pnt2.SetValue(9, gp_Pnt(0., 0., 0.));

    tigl::CTiglBSplineApproxInterp app(pnt2, 4, 2);
    app.InterpolatePoint(0);
    app.InterpolatePoint(2, true);
    app.InterpolatePoint(4, true);
    app.InterpolatePoint(6, true);
    app.InterpolatePoint(8);
    tigl::CTiglApproxResult result = app.FitCurve(parms2);

    StoreResult("TestData/analysis/BSplineInterpolation-tipKink2.brep", result.curve, pnt2);
}
