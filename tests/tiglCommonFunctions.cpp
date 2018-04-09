/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-08 Martin Siggel <Martin.Siggel@dlr.de>
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
#include "tiglcommonfunctions.h"
#include "test.h"

#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>

TEST(TiglCommonFunctions, isPathRelative)
{
    ASSERT_TRUE(IsPathRelative("./test.txt"));

    ASSERT_TRUE(IsPathRelative("test.txt"));

    ASSERT_TRUE(IsPathRelative("../test.txt"));

    // check absolute paths
#ifdef WIN32
    ASSERT_FALSE(IsPathRelative("d:/data/test.txt"));
#else
    ASSERT_FALSE(IsPathRelative("/usr/bin/test.txt"));
#endif
}

TEST(TiglCommonFunctions, isFileReadable)
{
    ASSERT_TRUE(IsFileReadable("TestData/nacelle.stp"));
    ASSERT_FALSE(IsFileReadable("invalidfile.txt"));
}

TEST(TiglCommonFunctions, projectPointOnPlane)
{
    gp_Pnt p(0,0,1);
    gp_Pln pln(gp_Pnt(0,0,0), gp_Dir(0,0,1));

    gp_Pnt2d res;
    res = ProjectPointOnPlane(pln, p);
    ASSERT_NEAR(0, res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);

    p = gp_Pnt(2,0,3);
    res = ProjectPointOnPlane(pln, p);
    ASSERT_NEAR(2, res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);

    pln = gp_Pln(gp_Pnt(0,0,0), gp_Dir(0,1,0));
    res = ProjectPointOnPlane(pln, p);
    ASSERT_NEAR(3, res.X(), 1e-10);
    ASSERT_NEAR(2, res.Y(), 1e-10);

    pln = gp_Pln(gp_Pnt(0,0,0), gp_Dir(-1,1,0));
    p = gp_Pnt(1,0,0);
    res = ProjectPointOnPlane(pln, p);
    ASSERT_NEAR(sqrt(0.5), res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);

    pln = gp_Pln(gp_Pnt(1,0,0), gp_Dir(-1,1,0));
    p = gp_Pnt(1,0,0);
    res = ProjectPointOnPlane(pln, p);
    ASSERT_NEAR(0, res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);
}

TEST(TiglCommonFunctions, projectVecOnPlane)
{
    gp_Vec v(0,0,1);
    gp_Pln pln(gp_Pnt(0,0,0), gp_Dir(0,0,1));

    gp_Vec2d res;
    res = ProjectVecOnPlane(pln, v);
    ASSERT_NEAR(0, res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);

    v = gp_Vec(2,0,3);
    res = ProjectVecOnPlane(pln, v);
    ASSERT_NEAR(2, res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);

    pln = gp_Pln(gp_Pnt(0,0,0), gp_Dir(0,1,0));
    res = ProjectVecOnPlane(pln, v);
    ASSERT_NEAR(3, res.X(), 1e-10);
    ASSERT_NEAR(2, res.Y(), 1e-10);

    pln = gp_Pln(gp_Pnt(0,0,0), gp_Dir(-1,1,0));
    v = gp_Vec(1,0,0);
    res = ProjectVecOnPlane(pln, v);
    ASSERT_NEAR(sqrt(0.5), res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);

    pln = gp_Pln(gp_Pnt(1,0,0), gp_Dir(-1,1,0));
    v = gp_Vec(1,0,0);
    res = ProjectVecOnPlane(pln, v);
    ASSERT_NEAR(sqrt(0.5), res.X(), 1e-10);
    ASSERT_NEAR(0, res.Y(), 1e-10);
}

TEST(TiglCommonFunctions, IntersectLinePlane)
{
    gp_Pln plane(gp_Pnt(10., 2., 0.), gp_Dir(0., 1., 0));

    gp_Pnt result;
    ASSERT_EQ(BetweenPoints, IntersectLinePlane(gp_Pnt(0., 0., 0.), gp_Pnt(0., 4., 0.), plane, result));
    ASSERT_NEAR(0., result.Distance(gp_Pnt(0., 2., 0)), 1e-10);

    ASSERT_EQ(OutsideBefore, IntersectLinePlane(gp_Pnt(1., 3., 0.), gp_Pnt(1., 4., 0.), plane, result));
    ASSERT_NEAR(0., result.Distance(gp_Pnt(1., 2., 0)), 1e-10);

    ASSERT_EQ(OutsideAfter, IntersectLinePlane(gp_Pnt(1., 0., 0.), gp_Pnt(1., 1., 0.), plane, result));
    ASSERT_NEAR(0., result.Distance(gp_Pnt(1., 2., 0)), 1e-10);

    ASSERT_EQ(NoIntersection, IntersectLinePlane(gp_Pnt(1., 3., 0.), gp_Pnt(10., 3., 0.), plane, result));
}
