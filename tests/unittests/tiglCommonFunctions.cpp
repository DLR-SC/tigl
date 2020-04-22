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
#include "tigl.h"
#include "tiglcommonfunctions.h"
#include "test.h"
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

#include <gp_Pln.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Standard_Failure.hxx>

#include <list>

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

TEST(TiglCommonFunctions, getEdgeContinuity)
{
    TopoDS_Edge edge1;
    TopoDS_Edge edge2;
    TopoDS_Edge edge3;
    BRep_Builder b;
    BRepTools::Read(edge1, "TestData/checkEdgeContinuity_edge1.brep", b);
    BRepTools::Read(edge2, "TestData/checkEdgeContinuity_edge2.brep", b);
    BRepTools::Read(edge3, "TestData/checkEdgeContinuity_edge3.brep", b);
    ASSERT_EQ(C2, getEdgeContinuity(edge1, edge2));
    ASSERT_EQ(C0, getEdgeContinuity(edge2, edge3));
    ASSERT_EQ(C0, getEdgeContinuity(edge3, edge1));
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

TEST(TiglCommonFunctions, IsPointInsideShape)
{
    TopoDS_Shape box = BRepPrimAPI_MakeBox(1., 1., 1).Solid();

    EXPECT_TRUE(IsPointInsideShape(box, gp_Pnt(0.5, 0.5, 0.5)));
    EXPECT_TRUE(IsPointInsideShape(box, gp_Pnt(1.0, 0.5, 0.5)));

    EXPECT_FALSE(IsPointInsideShape(box, gp_Pnt(1.5, 0.5, 0.5)));

    TopoDS_Shape boxrev = box.Reversed();

    EXPECT_TRUE(IsPointInsideShape(boxrev, gp_Pnt(0.5, 0.5, 0.5)));
    EXPECT_TRUE(IsPointInsideShape(boxrev, gp_Pnt(1.0, 0.5, 0.5)));

    EXPECT_FALSE(IsPointInsideShape(boxrev, gp_Pnt(1.5, 0.5, 0.5)));

    // check tolerance of 1e-3
    EXPECT_TRUE(IsPointInsideShape(boxrev, gp_Pnt(1.0009, 0.5, 0.5)));
    EXPECT_FALSE(IsPointInsideShape(boxrev, gp_Pnt(1.0011, 0.5, 0.5)));

    // The function requires a solid, else it will throw
    TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(gp_Pnt(10., 10., 10.));
    EXPECT_THROW(IsPointInsideShape(v, gp_Pnt(0., 0., 0.)), tigl::CTiglError);
}

TEST(TiglCommonFunctions, tiglCheckPointInside_api)
{
    TixiDocumentHandle tixiSimpleWingHandle = -1;
    TiglCPACSConfigurationHandle tiglSimpleWingHandle = -1;

    ReturnCode tixiRet = tixiOpenDocument("TestData/simpletest.cpacs.xml", &tixiSimpleWingHandle);
    ASSERT_EQ (SUCCESS, tixiRet);

    TiglReturnCode tiglRet = tiglOpenCPACSConfiguration(tixiSimpleWingHandle, "Cpacs2Test", &tiglSimpleWingHandle);
    ASSERT_EQ(TIGL_SUCCESS, tiglRet);

    TiglBoolean isInside = TIGL_FALSE;

    ASSERT_EQ(TIGL_SUCCESS, tiglCheckPointInside(tiglSimpleWingHandle, 0., 0., 0., "segmentD150_Fuselage_1Segment2ID", &isInside));
    EXPECT_EQ(TIGL_TRUE, isInside);

    ASSERT_EQ(TIGL_SUCCESS, tiglCheckPointInside(tiglSimpleWingHandle, 0., 0., 2., "segmentD150_Fuselage_1Segment2ID", &isInside));
    EXPECT_EQ(TIGL_FALSE, isInside);

    // test errors
    EXPECT_EQ(TIGL_UID_ERROR, tiglCheckPointInside(tiglSimpleWingHandle, 0., 0., 0., "wrongUID", &isInside));
    EXPECT_EQ(TIGL_NOT_FOUND, tiglCheckPointInside(-1, 0., 0., 0., "wrongUID", &isInside));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglCheckPointInside(tiglSimpleWingHandle, 0., 0., 0., nullptr, &isInside));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglCheckPointInside(tiglSimpleWingHandle, 0., 0., 0., "wrongUID", nullptr));


}

TEST(TiglCommonFunctions, LinspaceWithBreaks)
{
    std::vector<double> breaks;
    breaks.push_back(0.25);
    breaks.push_back(0.49);
    breaks.push_back(0.62);
    breaks.push_back(0.);
    breaks.push_back(1.0);
    std::vector<double> res = LinspaceWithBreaks(0., 1., 11, breaks);
    ASSERT_EQ(12, res.size());
    EXPECT_NEAR(0.00, res[0], 1e-10);
    EXPECT_NEAR(0.10, res[1], 1e-10);
    EXPECT_NEAR(0.20, res[2], 1e-10);
    EXPECT_NEAR(0.25, res[3], 1e-10);
    EXPECT_NEAR(0.30, res[4], 1e-10);
    EXPECT_NEAR(0.40, res[5], 1e-10);
    EXPECT_NEAR(0.49, res[6], 1e-10);
    EXPECT_NEAR(0.62, res[7], 1e-10);
    EXPECT_NEAR(0.70, res[8], 1e-10);
    EXPECT_NEAR(0.80, res[9], 1e-10);
    EXPECT_NEAR(0.90, res[10], 1e-10);
    EXPECT_NEAR(1.00, res[11], 1e-10);
}

TEST(TiglCommonFunctions, ReplaceAdjacentWith)
{
    auto is_adjacent = [](int v1, int v2) {
        return v1 + 1 == v2;
    };
    
    auto merged = [](int /* v1 */, int v2) {
        return v2;
    };
    
    std::list<int> a = {1, 2, 3, 10};
    ReplaceAdjacentWithMerged(a, is_adjacent, merged);
    EXPECT_TRUE(ArraysMatch({3, 10}, a));
    
    a = {1, 2, 5, 6, 7, 9, 11, 23, 24};
    ReplaceAdjacentWithMerged(a, is_adjacent, merged);
    EXPECT_TRUE(ArraysMatch({2, 7, 9, 11, 24}, a));
    
    a = {5};
    ReplaceAdjacentWithMerged(a, is_adjacent, merged);
    EXPECT_TRUE(ArraysMatch({5}, a));
    
    a = {5, 6};
    ReplaceAdjacentWithMerged(a, is_adjacent, merged);
    EXPECT_TRUE(ArraysMatch({6}, a));
    
    a = {};
    ReplaceAdjacentWithMerged(a, is_adjacent, merged);
    EXPECT_TRUE(ArraysMatch({}, a));
}

TEST(TiglCommonFunctions, FaceBetweenPoints)
{
    gp_Pln plane(gp_Pnt(0., 0., 0.), gp_Dir(0, 1., 0));
    TopoDS_Face face = BRepBuilderAPI_MakeFace(plane);

    EXPECT_TRUE(IsFaceBetweenPoints(face, gp_Pnt(0, -1, 0), gp_Pnt(0, 1, 0)));
    EXPECT_FALSE(IsFaceBetweenPoints(face, gp_Pnt(0, -1, 0), gp_Pnt(0, -1, 0)));
    EXPECT_FALSE(IsFaceBetweenPoints(face, gp_Pnt(0, 1, 0), gp_Pnt(0, 1, 0)));
    
    EXPECT_TRUE(IsFaceBetweenPoints(face, gp_Pnt(5, -1, 20), gp_Pnt(3, 1, -20)));
    EXPECT_FALSE(IsFaceBetweenPoints(face, gp_Pnt(5, -1, 20), gp_Pnt(3, -10, -20)));
}

TEST(TiglCommonFunctions, IsPointInsideFace)
{
    TopoDS_Edge left = BRepBuilderAPI_MakeEdge(gp_Pnt(-1,-1,0), gp_Pnt(-1,1,0)).Edge();
    TopoDS_Edge right = BRepBuilderAPI_MakeEdge(gp_Pnt(1,-1,0), gp_Pnt(1,1,0)).Edge();
    TopoDS_Edge top = BRepBuilderAPI_MakeEdge(gp_Pnt(-1,1,0), gp_Pnt(1,1,0)).Edge();
    TopoDS_Edge bottom = BRepBuilderAPI_MakeEdge(gp_Pnt(-1,-1,0), gp_Pnt(1,-1,0)).Edge();
    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(left, bottom, right, top).Wire();
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire, false).Face();

    EXPECT_TRUE(IsPointInsideFace(face, gp_Pnt(0,0,0)));
    EXPECT_TRUE(IsPointInsideFace(face, gp_Pnt(-1,-1,0)));
    EXPECT_TRUE(IsPointInsideFace(face, gp_Pnt(-1,1,0)));
    EXPECT_TRUE(IsPointInsideFace(face, gp_Pnt(1,1,0)));
    EXPECT_TRUE(IsPointInsideFace(face, gp_Pnt(1,-1,0)));

    EXPECT_TRUE(IsPointInsideFace(face, gp_Pnt(1,0,0)));

    EXPECT_FALSE(IsPointInsideFace(face, gp_Pnt(1,-2,0)));
    EXPECT_FALSE(IsPointInsideFace(face, gp_Pnt(0,0,-1)));
    EXPECT_FALSE(IsPointInsideFace(face, gp_Pnt(0,0, 1)));

}

TEST(TiglCommonFunctions, IsPointAbovePlane)
{
    gp_Pln xy = gp_Pln(gp_Ax3(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.), gp_Dir(1., 0., 0.)));

    EXPECT_TRUE (IsPointAbovePlane(xy, gp_Pnt(0., 0.,  1.)));
    EXPECT_TRUE (IsPointAbovePlane(xy, gp_Pnt(1., 1.,  1.)));
    EXPECT_FALSE(IsPointAbovePlane(xy, gp_Pnt(1., 1., -1.)));
    EXPECT_FALSE(IsPointAbovePlane(xy, gp_Pnt(0., 0., -1.)));
    EXPECT_FALSE(IsPointAbovePlane(xy, gp_Pnt(0., 0.,  0.)));
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
