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
#include <Geom_BSplineCurve.hxx>

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
