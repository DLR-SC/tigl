/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2013-02-13 Martin Siggel <Martin.Siggel@dlr.de>
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
 *
 * @file tiglPolydata.cpp
 */

#include "test.h"
#include "tigl.h"
#include "CTiglPolyData.h"

using namespace tigl;

TEST(TiglPolyData, cube_export_vtk_standard)
{
    CTiglPolyData poly;
    CTiglPoint p1(0, 0, 0);
    CTiglPoint p2(0, 1, 0);
    CTiglPoint p3(0, 1, 1);
    CTiglPoint p4(0, 0, 1);
    CTiglPoint p5(-1, 0, 0);
    CTiglPoint p6(-1, 1, 0);
    CTiglPoint p7(-1, 1, 1);
    CTiglPoint p8(-1, 0, 1);

    //face one
    CTiglPolygon f1;
    f1.addPoint(p1);
    f1.addPoint(p2);
    f1.addPoint(p3);
    f1.addPoint(p4);


    poly.addPolygon(f1);

    CTiglPolygon f2;
    f2.addPoint(p6);
    f2.addPoint(p5);
    f2.addPoint(p8);
    f2.addPoint(p7);
    poly.addPolygon(f2);

    CTiglPolygon f3;
    f3.addPoint(p5);
    f3.addPoint(p1);
    f3.addPoint(p4);
    f3.addPoint(p8);
    poly.addPolygon(f3);

    CTiglPolygon f4;
    f4.addPoint(p2);
    f4.addPoint(p6);
    f4.addPoint(p7);
    f4.addPoint(p3);
    poly.addPolygon(f4);

    CTiglPolygon f5;
    f5.addPoint(p4);
    f5.addPoint(p3);
    f5.addPoint(p7);
    f5.addPoint(p8);
    poly.addPolygon(f5);

    CTiglPolygon f6;
    f6.addPoint(p5);
    f6.addPoint(p6);
    f6.addPoint(p2);
    f6.addPoint(p1);
    poly.addPolygon(f6);

    poly.writeVTK("vtk_cube_standard.vtp");

    //check polygon data
    ASSERT_EQ(6, poly.getNPolygons());

    ASSERT_EQ(4, poly.getNPointsOfPoly(0));
    ASSERT_NEAR(0., poly.getPointOfPoly(0,0).distance2(p1), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(1,0).distance2(p2), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(2,0).distance2(p3), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(3,0).distance2(p4), 1e-10);

    ASSERT_EQ(4, poly.getNPointsOfPoly(1));
    ASSERT_NEAR(0., poly.getPointOfPoly(0,1).distance2(p6), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(1,1).distance2(p5), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(2,1).distance2(p8), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(3,1).distance2(p7), 1e-10);

    ASSERT_EQ(4, poly.getNPointsOfPoly(2));
    ASSERT_NEAR(0., poly.getPointOfPoly(0,2).distance2(p5), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(1,2).distance2(p1), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(2,2).distance2(p4), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(3,2).distance2(p8), 1e-10);

    ASSERT_EQ(4, poly.getNPointsOfPoly(3));
    ASSERT_NEAR(0., poly.getPointOfPoly(0,3).distance2(p2), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(1,3).distance2(p6), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(2,3).distance2(p7), 1e-10);
    ASSERT_NEAR(0., poly.getPointOfPoly(3,3).distance2(p3), 1e-10);
}

TEST(TiglPolyData, cube_export_vtk_withnormals)
{
    CTiglPolyData poly;
    CTiglPoint p1(0, 0, 0);
    CTiglPoint p2(0, 1, 0);
    CTiglPoint p3(0, 1, 1);
    CTiglPoint p4(0, 0, 1);
    CTiglPoint p5(-1, 0, 0);
    CTiglPoint p6(-1, 1, 0);
    CTiglPoint p7(-1, 1, 1);
    CTiglPoint p8(-1, 0, 1);

    //face one
    poly.enableNormals(true);
    CTiglPolygon f1;
    f1.addPoint(p1);
    f1.addPoint(p2);
    f1.addPoint(p3);
    f1.addPoint(p4);

    for (int i = 0; i < 4; ++i)
        f1.addNormal(CTiglPoint(1, 0, 0));

    poly.addPolygon(f1);

    CTiglPolygon f2;
    f2.addPoint(p6);
    f2.addPoint(p5);
    f2.addPoint(p8);
    f2.addPoint(p7);

    for (int i = 0; i < 4; ++i)
        f2.addNormal(CTiglPoint(-1, 0, 0));

    poly.addPolygon(f2);

    CTiglPolygon f3;
    f3.addPoint(p5);
    f3.addPoint(p1);
    f3.addPoint(p4);
    f3.addPoint(p8);

    for (int i = 0; i < 4; ++i)
        f3.addNormal(CTiglPoint(0, -1, 0));

    poly.addPolygon(f3);

    CTiglPolygon f4;
    f4.addPoint(p2);
    f4.addPoint(p6);
    f4.addPoint(p7);
    f4.addPoint(p3);

    for (int i = 0; i < 4; ++i)
        f4.addNormal(CTiglPoint(0, 1, 0));

    poly.addPolygon(f4);

    CTiglPolygon f5;
    f5.addPoint(p4);
    f5.addPoint(p3);
    f5.addPoint(p7);
    f5.addPoint(p8);

    for (int i = 0; i < 4; ++i)
        f5.addNormal(CTiglPoint(0, 0, 1));

    poly.addPolygon(f5);

    CTiglPolygon f6;
    f6.addPoint(p5);
    f6.addPoint(p6);
    f6.addPoint(p2);
    f6.addPoint(p1);

    for (int i = 0; i < 4; ++i)
        f6.addNormal(CTiglPoint(0, 0, -1));

    poly.addPolygon(f6);

    poly.writeVTK("vtk_cube+normals.vtp");
}

TEST(TiglPolyData, cube_export_vtk_withpieces)
{
    CTiglPolyData poly;
    CTiglPoint p1(0, 0, 0);
    CTiglPoint p2(0, 1, 0);
    CTiglPoint p3(0, 1, 1);
    CTiglPoint p4(0, 0, 1);
    CTiglPoint p5(-1, 0, 0);
    CTiglPoint p6(-1, 1, 0);
    CTiglPoint p7(-1, 1, 1);
    CTiglPoint p8(-1, 0, 1);

    CTiglPolygon f1;
    f1.addPoint(p1);
    f1.addPoint(p2);
    f1.addPoint(p3);
    f1.addPoint(p4);
    poly.addPolygon(f1);

    poly.createNewSurface();
    CTiglPolygon f2;
    f2.addPoint(p6);
    f2.addPoint(p5);
    f2.addPoint(p8);
    f2.addPoint(p7);
    poly.addPolygon(f2);

    poly.createNewSurface();
    CTiglPolygon f3;
    f3.addPoint(p5);
    f3.addPoint(p1);
    f3.addPoint(p4);
    f3.addPoint(p8);
    poly.addPolygon(f3);

    poly.createNewSurface();
    CTiglPolygon f4;
    f4.addPoint(p2);
    f4.addPoint(p6);
    f4.addPoint(p7);
    f4.addPoint(p3);
    poly.addPolygon(f4);

    poly.createNewSurface();
    CTiglPolygon f5;
    f5.addPoint(p4);
    f5.addPoint(p3);
    f5.addPoint(p7);
    f5.addPoint(p8);
    poly.addPolygon(f5);

    poly.createNewSurface();
    CTiglPolygon f6;
    f6.addPoint(p5);
    f6.addPoint(p6);
    f6.addPoint(p2);
    f6.addPoint(p1);
    poly.addPolygon(f6);

    poly.writeVTK("vtk_cube+pieces.vtp");
}
