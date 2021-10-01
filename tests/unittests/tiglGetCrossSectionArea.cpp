/*
* Copyright (C) 2021 German Aerospace Center
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

TEST_F(GetCrossSectionAreaSimple, error_handle_invalid)
{
    double area;
    EXPECT_EQ(tiglGetCrossSectionArea(-1, 0., 0., 0., 1., 0., 0., 0, &area), TIGL_NOT_FOUND);
}

TEST_F(GetCrossSectionAreaSimple, fuse_result_mode_invalid)
{
    double area;
    EXPECT_EQ(tiglGetCrossSectionArea(-1, 0., 0., 0., 1., 0., 0., -1, &area), TIGL_INDEX_ERROR);
    EXPECT_EQ(tiglGetCrossSectionArea(-1, 0., 0., 0., 1., 0., 0.,  4, &area), TIGL_INDEX_ERROR);
}

TEST_F(GetCrossSectionAreaSimple, error_zero_normal)
{
    double area;
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, 0., 0., 0., 0., 0., 0., 0, &area), TIGL_MATH_ERROR);
}


TEST_F(GetCrossSectionAreaSimple, sanity_values)
{
    double area_half, area_full, area;

    // in x-direction at x=0.75
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, -1, 0., 0., 1., 0., 0., 0, &area), TIGL_SUCCESS);
    EXPECT_EQ(area, 0.);

    // in x-direction
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, -0.25, 0., 0., 1., 0., 0., 1, &area), TIGL_SUCCESS);
    EXPECT_NEAR(area, 3.1416*0.25, 0.05);

    // reversed normal
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, -0.25, 0., 0., -1., 0., 0., 1, &area_full), TIGL_SUCCESS);
    EXPECT_EQ(area_full, area);

    // in z-direction at z=0
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, 0., 0., 0., 0., 0., 1., 0, &area_half), TIGL_SUCCESS);
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, 0., 0., 0., 0., 0., 1., 1, &area_full), TIGL_SUCCESS);
    EXPECT_NEAR(area_half, 2 +   1.25, 0.1);  // full fuselage, one wing
    EXPECT_NEAR(area_full, 2 + 2*1.25, 0.01); // full fuselage, two wings

    // non-uniqe intersection (should not throw error and should be larger than zero
    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, 1.25, -0.5, 0., 1., -1., 0., 1, &area), TIGL_SUCCESS);
    EXPECT_GE(area, 0.);

}


