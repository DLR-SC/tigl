/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-19 Martin Siggel <martin.siggel@dlr.de>
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

#include "CControlSurfaceBorderBuilder.h"
#include "CTiglControlSurfaceBorderCoordinateSystem.h"

#include <TopoDS_Shape.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>

#include <cmath>

class TiglControlSurfaceBorder : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        BRep_Builder b;
        BRepTools::Read(wingShape, "TestData/simplewing.brep", b);
        ASSERT_FALSE(wingShape.IsNull());
    }

    virtual void TearDown()
    {
    }

protected:
    TopoDS_Shape wingShape;
};

TEST_F(TiglControlSurfaceBorder, boarderBuilder)
{
    double lex = 0.5;

    tigl::CTiglControlSurfaceBorderCoordinateSystem coordsystem(gp_Pnt(lex, 0.5, 0), gp_Pnt(1, 0.5, 0), gp_Vec(0, 0, 1.));
    tigl::CControlSurfaceBoarderBuilder builder(coordsystem, wingShape);

    builder.boarderSimple(1.0, 1.0);

    double upperXsi = 0.8;
    double lowerXsi = 0.8;
    double relHeight = 0.5;
    builder.boarderWithLEShape(relHeight, 1.0, upperXsi, lowerXsi);

    ASSERT_NEAR((upperXsi - 1.)*(lex - 1.), builder.upperPoint().X(), 1e-8);
    ASSERT_TRUE(builder.upperPoint().Y() > 0.);

    ASSERT_NEAR((lowerXsi - 1.)*(lex - 1.), builder.lowerPoint().X(), 1e-8);
    ASSERT_TRUE(builder.lowerPoint().Y() < 0.);
}

TEST(TiglControlSurfaceBorderCoordinates, directions)
{
    tigl::CTiglControlSurfaceBorderCoordinateSystem coords(gp_Pnt(0,0,0), gp_Pnt(1,0,0), gp_Vec(0,0,1));
    
    ASSERT_NEAR(1, coords.getXDir().X(), 1e-10);
    ASSERT_NEAR(0, coords.getXDir().Y(), 1e-10);
    ASSERT_NEAR(0, coords.getXDir().Z(), 1e-10);
    
    ASSERT_NEAR(0, coords.getYDir().X(), 1e-10);
    ASSERT_NEAR(0, coords.getYDir().Y(), 1e-10);
    ASSERT_NEAR(1, coords.getYDir().Z(), 1e-10);
    
    ASSERT_NEAR(0, coords.getNormal().X(), 1e-10);
    ASSERT_NEAR(-1, coords.getNormal().Y(), 1e-10);
    ASSERT_NEAR(0, coords.getNormal().Z(), 1e-10);
}

TEST(TiglControlSurfaceBorderCoordinates, transformation)
{
    tigl::CTiglControlSurfaceBorderCoordinateSystem coords(gp_Pnt(2,0,0), gp_Pnt(4,0,0), gp_Vec(0,0,1));
    
    gp_Trsf t = coords.globalTransform();

    gp_Pnt v1(1,0,0);
    v1 = v1.Transformed(t);
    
    ASSERT_NEAR(3, v1.X(), 1e-10);
    ASSERT_NEAR(0, v1.Y(), 1e-10);
    ASSERT_NEAR(0, v1.Z(), 1e-10);
    
    gp_Pnt v2(0,1,0);
    v2 = v2.Transformed(t);
    
    ASSERT_NEAR(2, v2.X(), 1e-10);
    ASSERT_NEAR(0, v2.Y(), 1e-10);
    ASSERT_NEAR(1, v2.Z(), 1e-10);
    
    gp_Pnt v3(0,0,1);
    v3 = v3.Transformed(t);
    
    ASSERT_NEAR(2, v3.X(), 1e-10);
    ASSERT_NEAR(-1, v3.Y(), 1e-10);
    ASSERT_NEAR(0, v3.Z(), 1e-10);
}

TEST(TiglControlSurfaceBorderCoordinates, transformationAdvanced)
{
    tigl::CTiglControlSurfaceBorderCoordinateSystem coords(gp_Pnt(2,0,0), gp_Pnt(3,1,0), gp_Vec(0.1,0.1,0.8));
    
    gp_Trsf t = coords.globalTransform();

    gp_Pnt v1(1,0,0);
    v1 = v1.Transformed(t);
    
    ASSERT_NEAR(2. + sqrt(0.5), v1.X(), 1e-10);
    ASSERT_NEAR(sqrt(0.5), v1.Y(), 1e-10);
    ASSERT_NEAR(0, v1.Z(), 1e-10);
    
    gp_Pnt v2(0,1,0);
    v2 = v2.Transformed(t);
    
    ASSERT_NEAR(2, v2.X(), 1e-10);
    ASSERT_NEAR(0, v2.Y(), 1e-10);
    ASSERT_NEAR(1, v2.Z(), 1e-10);
    
    gp_Pnt v3(0,0,1);
    v3 = v3.Transformed(t);
    
    ASSERT_NEAR(2 + sqrt(0.5), v3.X(), 1e-10);
    ASSERT_NEAR(-sqrt(0.5), v3.Y(), 1e-10);
    ASSERT_NEAR(0, v3.Z(), 1e-10);
}

TEST(TiglControlSurfaceBorderCoordinates, transformationAdvanced2)
{
    gp_Trsf t;
    t.SetTransformation(gp_Ax3(gp_Pnt(0,0,0), gp_Vec(0,-1,0), gp_Vec(1,0,0)));
    
    gp_Pnt v1(3,7,2);
    v1 = v1.Transformed(t);
    
    ASSERT_NEAR(3, v1.X(), 1e-10);
    ASSERT_NEAR(2, v1.Y(), 1e-10);
    ASSERT_NEAR(-7, v1.Z(), 1e-10);

}
