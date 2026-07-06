/*
* Copyright (C) 2025 German Aerospace Center (DLR/SC)
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

#include "CBopCommon.h"
#include "CTrimShape.h"
#include "CCutShape.h"
#include "CNamedShape.h"
#include "CTiglError.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Trsf.hxx>


namespace {

PNamedShape BoxAsNamedShape(const TopoDS_Shape& box, const char* name)
{
    return PNamedShape(new CNamedShape(box, name));
}

TopoDS_Shape MakeBox(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return BRepPrimAPI_MakeBox(gp_Pnt(x1, y1, z1), gp_Pnt(x2, y2, z2)).Solid();
}

PNamedShape MakeNullShape()
{
    return PNamedShape();
}

} // namespace


TEST(CBopCommon, IntersectingBoxesReturnsValidShape)
{
    TopoDS_Shape box1 = MakeBox(0, 0, 0, 1, 1, 1);
    TopoDS_Shape box2 = MakeBox(0.5, 0.5, 0.5, 1.5, 1.5, 1.5);

    CBopCommon op(BoxAsNamedShape(box1, "box1"), BoxAsNamedShape(box2, "box2"));
    PNamedShape result = op;

    ASSERT_TRUE(result != nullptr);
    ASSERT_FALSE(result->Shape().IsNull());
    EXPECT_TRUE(BRepCheck_Analyzer(result->Shape()).IsValid());
    EXPECT_GT(TopExp_Explorer(result->Shape(), TopAbs_FACE).More(), 0);
}

TEST(CBopCommon, FuzzyValueSetter)
{
    CBopCommon op(MakeNullShape(), MakeNullShape());
    EXPECT_NO_THROW(op.SetFuzzyValue(1e-4));
}

TEST(CBopCommon, FuzzyValueSetterDefault)
{
    CBopCommon op(MakeNullShape(), MakeNullShape());
    EXPECT_NO_THROW(op.SetFuzzyValue(Precision::Confusion()));
}

TEST(CTrimShape, IntersectingBoxesExcludeReturnsValidShape)
{
    TopoDS_Shape box1 = MakeBox(0, 0, 0, 2, 2, 2);
    TopoDS_Shape box2 = MakeBox(1, 1, 1, 3, 3, 3);

    CTrimShape op(BoxAsNamedShape(box1, "box1"), BoxAsNamedShape(box2, "box2"), EXCLUDE);
    PNamedShape result = op;

    ASSERT_TRUE(result != nullptr);
    ASSERT_FALSE(result->Shape().IsNull());
    EXPECT_TRUE(BRepCheck_Analyzer(result->Shape()).IsValid());
}

TEST(CTrimShape, IntersectingBoxesIncludeReturnsValidShape)
{
    TopoDS_Shape box1 = MakeBox(0, 0, 0, 2, 2, 2);
    TopoDS_Shape box2 = MakeBox(1, 1, 1, 3, 3, 3);

    CTrimShape op(BoxAsNamedShape(box1, "box1"), BoxAsNamedShape(box2, "box2"), INCLUDE);
    PNamedShape result = op;

    ASSERT_TRUE(result != nullptr);
    ASSERT_FALSE(result->Shape().IsNull());
    EXPECT_TRUE(BRepCheck_Analyzer(result->Shape()).IsValid());
}

TEST(CTrimShape, FuzzyValueSetter)
{
    CTrimShape op(MakeNullShape(), MakeNullShape(), EXCLUDE);
    EXPECT_NO_THROW(op.SetFuzzyValue(1e-4));
}

TEST(CTrimShape, FuzzyValueSetterDefault)
{
    CTrimShape op(MakeNullShape(), MakeNullShape(), EXCLUDE);
    EXPECT_NO_THROW(op.SetFuzzyValue(Precision::Confusion()));
}

TEST(CCutShape, IntersectingBoxesReturnsValidShape)
{
    TopoDS_Shape box1 = MakeBox(0, 0, 0, 2, 2, 2);
    TopoDS_Shape box2 = MakeBox(1, 1, 1, 3, 3, 3);

    CCutShape op(BoxAsNamedShape(box1, "box1"), BoxAsNamedShape(box2, "box2"));
    PNamedShape result = op;

    ASSERT_TRUE(result != nullptr);
    ASSERT_FALSE(result->Shape().IsNull());
    EXPECT_GT(TopExp_Explorer(result->Shape(), TopAbs_FACE).More(), 0);
}

TEST(CCutShape, FuzzyValueSetter)
{
    CCutShape op(MakeNullShape(), MakeNullShape());
    EXPECT_NO_THROW(op.SetFuzzyValue(1e-4));
}

TEST(CCutShape, FuzzyValueSetterDefault)
{
    CCutShape op(MakeNullShape(), MakeNullShape());
    EXPECT_NO_THROW(op.SetFuzzyValue(Precision::Confusion()));
}