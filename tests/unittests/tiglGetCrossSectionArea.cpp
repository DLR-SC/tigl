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

        // Add D150 test case

        const char* filename2 = "TestData/CPACS_30_D150.xml";
        ReturnCode tixiRet2;
        TiglReturnCode tiglRet2;

        tiglHandle2 = -1;
        tixiHandle2 = -1;

        tixiRet2 = tixiOpenDocument(filename2, &tixiHandle2);
        ASSERT_TRUE (tixiRet2 == SUCCESS);
        tiglRet2 = tiglOpenCPACSConfiguration(tixiHandle2, "", &tiglHandle2);
        ASSERT_TRUE(tiglRet2 == TIGL_SUCCESS);


    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;

        //Adding D150 test case

        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle2) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle2) == SUCCESS);
        tiglHandle2 = -1;
        tixiHandle2 = -1;
    }

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    //Add D150 test case

    static TixiDocumentHandle       	tixiHandle2;
    static TiglCPACSConfigurationHandle tiglHandle2;
};


TixiDocumentHandle  GetCrossSectionAreaSimple::tixiHandle = 0;
TiglCPACSConfigurationHandle  GetCrossSectionAreaSimple::tiglHandle = 0;

TixiDocumentHandle  GetCrossSectionAreaSimple::tixiHandle2 = 0;
TiglCPACSConfigurationHandle  GetCrossSectionAreaSimple::tiglHandle2 = 0;

TEST_F(GetCrossSectionAreaSimple, uID_check)
{
    double area;

    // Test if the uID is valid, i.e. it exists and is related to a geometric component

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "nonExistentUID", 0., 0., 0., 0., 0., 1., &area), TIGL_UID_ERROR);

    // Empty uID

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "", 0., 0., 0., 0., 0., 1., &area), TIGL_XML_ERROR);

    // For the D150 configuration:

    // Test if the uID is valid, i.e. it exists and is related to a geometric component

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle2, "nonExistentUID", 0., 0., 0., 0., 0., 1., &area), TIGL_UID_ERROR);

    // Empty uID

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle2, "", 0., 0., 0., 0., 0., 1., &area), TIGL_XML_ERROR);

}

TEST_F(GetCrossSectionAreaSimple, area_computations_fused_airplane)
{
    double area;

    // In the following line both wings and the fuselage are cut simultaneously.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle, "Cpacs2Test", 0., 0., 0., 0., 0., 1., &area), TIGL_SUCCESS);

    double precision = 1.E-5;

    ASSERT_NEAR(area, 4.5, precision);

    // Now let the cutting plane be given by the x-z-plane

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

    // For the D150 configuration:

    // The cross section area of the following plane and the configuration is expected to be zero.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle2, "D150_VAMP", 0., 0., 0., 1., 0., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 0., precision);

    // The expected area for the following intersection is verified through an approximative estimation in the GUI.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle2, "D150_VAMP", 10., 0., 0., 1., 0., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 12.84858, precision);

    // An approximation using the GUI shows that the following expected area is plausible.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle2, "D150_VAMP", 15., 0., 0., 1., 0., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 18.88991, precision);


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

    // For the D150 configuration:

    // An approximation using the GUI shows that the following expected area is plausible.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle2, "D150_VAMP_W1", 15., 0., 0., 1., 0., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 4.44927, precision);

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

    // For the D150 configuration:

    // An approximation using the GUI shows that the following expected area is plausible.

    EXPECT_EQ(tiglGetCrossSectionArea(tiglHandle2, "D150_VAMP_FL1", 15., 0., 0., 1., 0., 0., &area), TIGL_SUCCESS);

    ASSERT_NEAR(area, 12.84757, precision);
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

