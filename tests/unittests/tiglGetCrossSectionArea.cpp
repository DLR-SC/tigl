/*
* Copyright (C) 2022 German Aerospace Center
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
#include "tigl.h"

class GetCrossSectionAreaSimple : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};


TixiDocumentHandle  GetCrossSectionAreaSimple::tixiHandle = 0;
TiglCPACSConfigurationHandle  GetCrossSectionAreaSimple::tiglHandle = 0;

// TODO: Test if uID is valid: Does it exist and is it in case of existence assigned to a geometric component?

TEST_F(GetCrossSectionAreaSimple, area_computations_fused_airplane)
{
    double area;

    // In the following line both wings and the fuselage are cut simultaneously.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Cpacs2Test", 0., 0., 0., 0., 0., 1., &area), TIGL_SUCCESS);

    double precision = 1.E-5;

    ASSERT_NEAR(area, 4.5, precision);

    // Now let the cutting plane given by the x-z-plane

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Cpacs2Test", 0., 0., 0., 0., 1., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 2., precision);

    // Choose a cutting plane, parallel to the x-z-plane, that doesn't intersect the airplane

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Cpacs2Test", 0., 2.5, 0., 0., 1., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 0., precision);

    // In the following line the cutting plane is chosen in such a way, that it is expected to be "tangent" to the fuselage and not
    // cut any of the wings at the same time. Hence the cross section area is expected to be 0.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Cpacs2Test", 0., 0., 0.5, 0., 0., 1., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 0., precision);

    // In the following line the cutting plane is positioned in such a way that only the fuselage is cut.
    // Note that the fuselage in simpletest.cpacs.xml is not a perfect cylinder since the profiles are no perfect circles.
    // For the following test, a low precision (precisionLow) has been chosen, since the cross section area of the cutting plane
    // with the non-cylindrical fuselage has been computet in an approximative way (visually in the GUI).

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Cpacs2Test", 0., 0., 0.25, 0., 0., 1., &area), TIGL_SUCCESS);

    double precisionLow = 5.E-2;

    ASSERT_NEAR(area, 1.72, precisionLow);
}

TEST_F(GetCrossSectionAreaSimple, area_computations_wing)
{
    double area;

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Wing", 0., 0., 0., 0., 0., 1., &area), TIGL_SUCCESS);

    double precision = 1.E-5;

    ASSERT_NEAR(area, 1.75, precision);

    // Position the cutting plane in such a way that it doesn't intersect the wing

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Wing", 0., 0., 0.25, 0., 0., 1., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 0., precision);
}

TEST_F(GetCrossSectionAreaSimple, area_computations_fuselage)
{
    double area;

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "SimpleFuselage", 0., 0., 0., 0., 0., 1., &area), TIGL_SUCCESS);

    double precision = 1.E-5;

    ASSERT_NEAR(area, 2., precision);

    // Choose three different cutting planes in such a way, that they are "tangent" to the fuselage.
    // Hence the cross section area is expected to be 0.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "SimpleFuselage", 0., 0., 0.5, 0., 0., 1., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 0., precision);

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "SimpleFuselage", 0., 0.5, 0., 0., 1., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 0., precision);

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "SimpleFuselage", 0., - 0.5, 0., 0., 1., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 0., precision);

    // Compute the cross section of the non-perfectly cylindrical fuselage with the y-z-plane.
    // It is expected to be less than the area of a disc with radius 0.5.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "SimpleFuselage", 0., 0., 0., 1., 0., 0., &area), TIGL_SUCCESS);

    double precisionLow = 5.E-2;

    ASSERT_NEAR(area, 0.76, precisionLow);
}

TEST_F(GetCrossSectionAreaSimple, error_handle_invalid)
{
    double area;
    EXPECT_EQ(tiglGetCrossSectionArea(-1, "Cpacs2Test", 0., 0., 0., 1., 0., 0., &area), TIGL_NOT_FOUND);
}

TEST_F(GetCrossSectionAreaSimple, error_zero_normal)
{
    double area;
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle,"Cpacs2Test", 0., 0., 0., 0., 0., 0., &area), TIGL_MATH_ERROR);
}
