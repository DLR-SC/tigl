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

#include "CControlSurfaceBoarderBuilder.h"

#include <TopoDS_Shape.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>

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

    tigl::CSCoordSystem coordsystem(gp_Pnt(lex, 0.5, 0), gp_Pnt(1, 0.5, 0), gp_Vec(0, 0, 1.));
    tigl::CControlSurfaceBoarderBuilder builder(coordsystem, wingShape);

    builder.boarderSimple();

    double upperXsi = 0.8;
    double lowerXsi = 0.8;
    double relHeight = 0.5;
    builder.boarderWithLEShape(relHeight, upperXsi, lowerXsi);

    ASSERT_NEAR((upperXsi - 1.)*(lex - 1.), builder.upperPoint().X(), 1e-8);
    ASSERT_TRUE(builder.upperPoint().Y() > 0.);

    ASSERT_NEAR((lowerXsi - 1.)*(lex - 1.), builder.lowerPoint().X(), 1e-8);
    ASSERT_TRUE(builder.lowerPoint().Y() < 0.);
}
