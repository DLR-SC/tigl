/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017 Martin Siggel <Martin.Siggel@dlr.de>
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

#include <Geom_Curve.hxx>
#include "CTiglCurveNetworkSorter.h"
#include <vector>
#include <math_Matrix.hxx>

TEST(CurveNetwork, alreadyOrdered)
{
    // create some fake profiles / guides
    std::vector<Handle(Geom_Curve)> profiles(3);
    std::vector<Handle(Geom_Curve)> guides(3);

    math_Matrix u(0, 2, 0, 2);
    math_Matrix v(0, 2, 0, 2);

    u(0, 0) = 0.0; u(0, 1) = 0.2; u(0, 2) = 1.0;
    u(1, 0) = 0.0; u(1, 1) = 0.6; u(1, 2) = 1.0;
    u(2, 0) = 0.0; u(2, 1) = 0.7; u(2, 2) = 1.0;

    v(0, 0) = 0.0; v(0, 1) = 0.0; v(0, 2) = 0.0;
    v(1, 0) = 0.3; v(1, 1) = 0.4; v(1, 2) = 0.5;
    v(2, 0) = 1.0; v(2, 1) = 1.0; v(2, 2) = 1.0;

    tigl::CTiglCurveNetworkSorter sorter(profiles, guides, u, v);

    size_t ustart, vstart;
    bool guidesReversed = false;
    sorter.GetStartCurveIndices(ustart, vstart, guidesReversed);
    EXPECT_EQ(0, ustart);
    EXPECT_EQ(0, vstart);
    EXPECT_FALSE(guidesReversed);

    sorter.Perform();

    std::vector<std::string> pidx = sorter.ProfileIndices();
    EXPECT_STREQ("0", pidx[0].c_str());
    EXPECT_STREQ("1", pidx[1].c_str());
    EXPECT_STREQ("2", pidx[2].c_str());

    std::vector<std::string> gidx = sorter.GuideIndices();
    EXPECT_STREQ("0", gidx[0].c_str());
    EXPECT_STREQ("1", gidx[1].c_str());
    EXPECT_STREQ("2", gidx[2].c_str());

    // check parameters
    math_Matrix us = sorter.ProfileIntersectionParms();
    EXPECT_NEAR(0.0, us(0, 0), 1e-10);
    EXPECT_NEAR(0.2, us(0, 1), 1e-10);
    EXPECT_NEAR(1.0, us(0, 2), 1e-10);

    EXPECT_NEAR(0.0, us(1, 0), 1e-10);
    EXPECT_NEAR(0.6, us(1, 1), 1e-10);
    EXPECT_NEAR(1.0, us(1, 2), 1e-10);

    EXPECT_NEAR(0.0, us(2, 0), 1e-10);
    EXPECT_NEAR(0.7, us(2, 1), 1e-10);
    EXPECT_NEAR(1.0, us(2, 2), 1e-10);

    math_Matrix vs = sorter.GuideIntersectionParms();
    EXPECT_NEAR(0.0, vs(0, 0), 1e-10);
    EXPECT_NEAR(0.3, vs(1, 0), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 0), 1e-10);

    EXPECT_NEAR(0.0, vs(0, 1), 1e-10);
    EXPECT_NEAR(0.4, vs(1, 1), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 1), 1e-10);

    EXPECT_NEAR(0.0, vs(0, 2), 1e-10);
    EXPECT_NEAR(0.5, vs(1, 2), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 2), 1e-10);
}

TEST(CurveNetwork, unorderedAndReversed)
{
    // create some fake profiles / guides
    std::vector<Handle(Geom_Curve)> profiles(3);
    std::vector<Handle(Geom_Curve)> guides(3);

    math_Matrix u(0, 2, 0, 2);
    math_Matrix v(0, 2, 0, 2);

    u(0, 0) = 0.0; u(0, 1) = 0.4; u(0, 2) = 1.0;
    u(1, 0) = 1.0; u(1, 1) = 0.2; u(1, 2) = 0.0;
    u(2, 0) = 1.0; u(2, 1) = 0.7; u(2, 2) = 0.0;

    v(0, 0) = 0.5; v(0, 1) = 0.4; v(0, 2) = 0.3;
    v(1, 0) = 1.0; v(1, 1) = 0.0; v(1, 2) = 0.0;
    v(2, 0) = 0.0; v(2, 1) = 1.0; v(2, 2) = 1.0;

    tigl::CTiglCurveNetworkSorter sorter(profiles, guides, u, v);

    size_t ustart, vstart;
    bool guidesReversed = false;
    sorter.GetStartCurveIndices(ustart, vstart, guidesReversed);
    EXPECT_EQ(1, ustart);
    EXPECT_EQ(2, vstart);
    EXPECT_FALSE(guidesReversed);

    sorter.Perform();

    std::vector<std::string> pidx = sorter.ProfileIndices();
    EXPECT_STREQ("1", pidx[0].c_str());
    EXPECT_STREQ("-0", pidx[1].c_str());
    EXPECT_STREQ("2", pidx[2].c_str());

    std::vector<std::string> gidx = sorter.GuideIndices();
    EXPECT_STREQ("2", gidx[0].c_str());
    EXPECT_STREQ("1", gidx[1].c_str());
    EXPECT_STREQ("-0", gidx[2].c_str());

    // check parameters
    math_Matrix us = sorter.ProfileIntersectionParms();
    EXPECT_NEAR(0.0, us(0, 0), 1e-10);
    EXPECT_NEAR(0.2, us(0, 1), 1e-10);
    EXPECT_NEAR(1.0, us(0, 2), 1e-10);

    EXPECT_NEAR(0.0, us(1, 0), 1e-10);
    EXPECT_NEAR(0.6, us(1, 1), 1e-10);
    EXPECT_NEAR(1.0, us(1, 2), 1e-10);

    EXPECT_NEAR(0.0, us(2, 0), 1e-10);
    EXPECT_NEAR(0.7, us(2, 1), 1e-10);
    EXPECT_NEAR(1.0, us(2, 2), 1e-10);

    math_Matrix vs = sorter.GuideIntersectionParms();
    EXPECT_NEAR(0.0, vs(0, 0), 1e-10);
    EXPECT_NEAR(0.3, vs(1, 0), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 0), 1e-10);

    EXPECT_NEAR(0.0, vs(0, 1), 1e-10);
    EXPECT_NEAR(0.4, vs(1, 1), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 1), 1e-10);

    EXPECT_NEAR(0.0, vs(0, 2), 1e-10);
    EXPECT_NEAR(0.5, vs(1, 2), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 2), 1e-10);
}

TEST(CurveNetwork, unorderedCircular)
{
    // create some fake profiles / guides
    std::vector<Handle(Geom_Curve)> profiles(3);
    std::vector<Handle(Geom_Curve)> guides(2);

    math_Matrix u(0, 2, 0, 1);
    math_Matrix v(0, 2, 0, 1);

    u(0, 0) = 1.0; u(0, 1) = 0.0;
    u(1, 0) = 0.0; u(1, 1) = 1.0;
    u(2, 0) = 0.0; u(2, 1) = 1.0;

    v(0, 0) = 0.0; v(0, 1) = 1.0;
    v(1, 0) = 1.0; v(1, 1) = 0.0;
    v(2, 0) = 0.5; v(2, 1) = 0.6;

    tigl::CTiglCurveNetworkSorter sorter(profiles, guides, u, v);

    size_t ustart, vstart;
    bool guidesReversed = false;
    sorter.GetStartCurveIndices(ustart, vstart, guidesReversed);
    EXPECT_EQ(0, ustart);
    EXPECT_EQ(1, vstart);
    EXPECT_TRUE(guidesReversed);

    sorter.Perform();

    std::vector<std::string> pidx = sorter.ProfileIndices();
    ASSERT_EQ(3, pidx.size());
    EXPECT_STREQ("0", pidx[0].c_str());
    EXPECT_STREQ("-2", pidx[1].c_str());
    EXPECT_STREQ("-1", pidx[2].c_str());

    std::vector<std::string> gidx = sorter.GuideIndices();
    ASSERT_EQ(2, gidx.size());
    EXPECT_STREQ("-1", gidx[0].c_str());
    EXPECT_STREQ("0", gidx[1].c_str());

    // check parameters
    math_Matrix us = sorter.ProfileIntersectionParms();
    EXPECT_NEAR(0.0, us(0, 0), 1e-10);
    EXPECT_NEAR(1.0, us(0, 1), 1e-10);

    EXPECT_NEAR(0.0, us(1, 0), 1e-10);
    EXPECT_NEAR(1.0, us(1, 1), 1e-10);

    EXPECT_NEAR(0.0, us(2, 0), 1e-10);
    EXPECT_NEAR(1.0, us(2, 1), 1e-10);

    math_Matrix vs = sorter.GuideIntersectionParms();
    EXPECT_NEAR(0.0, vs(0, 0), 1e-10);
    EXPECT_NEAR(0.4, vs(1, 0), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 0), 1e-10);

    EXPECT_NEAR(0.0, vs(0, 1), 1e-10);
    EXPECT_NEAR(0.5, vs(1, 1), 1e-10);
    EXPECT_NEAR(1.0, vs(2, 1), 1e-10);
}
