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

TEST(BSplines, pointsToLinear)
{
    std::vector<gp_Pnt> points;
    points.push_back(gp_Pnt(0,0,0));
    points.push_back(gp_Pnt(2,0,0));
    points.push_back(gp_Pnt(2,1,0));
    points.push_back(gp_Pnt(1,1,0));
    
    Handle_Geom_BSplineCurve curve;
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

    Handle_Geom_BSplineCurve curve;
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

    Handle_Geom_BSplineCurve curve;
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

    Handle_Geom_BSplineCurve curve;
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

    Handle_Geom_BSplineCurve curve;
    tigl::CTiglSymetricSplineBuilder builder(points);

    ASSERT_THROW(curve = builder.GetBSpline(), tigl::CTiglError);
}
