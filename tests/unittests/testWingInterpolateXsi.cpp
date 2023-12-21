/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2015-01-07 Anton Reiswich <Anton.Reiswich@dlr.de>
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
/**
* @file
* @brief Tests for testing duct functions.
*/

#include "test.h" // Brings in the GTest framework

#include "tigl.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"

#include <iostream>

/******************************************************************************/




class WingInterpolateXsi : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/test_wing_segment_special_modified_component_segments.xml";
        //const char* filename = "TestData/test_wing_segment_special.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ (tixiRet, SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Aircraft1", &tiglHandle);
        ASSERT_EQ(tiglRet, TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() override{

    }
    void TearDown() override {
    }

    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    tigl::CTiglUIDManager& uidMgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(WingInterpolateXsi::tiglHandle).GetUIDManager();

};

TixiDocumentHandle WingInterpolateXsi::tixiHandle = 0;
TiglCPACSConfigurationHandle WingInterpolateXsi::tiglHandle = 0;

TEST_F(WingInterpolateXsi, tigl_wing_interpolate_xsi_segment_not_contained_in_component_segment)
{    
    double intersectionXsi = 0;
    TiglBoolean hasWarning;

    EXPECT_EQ(tiglWingInterpolateXsi(tiglHandle, "Aircraft1_Wing1_Seg2", 0., 0.0, "Aircraft1_Wing1_Seg2", 1., 0.5, "Aircraft1_Wing1_Seg2", 0.5, &intersectionXsi, &hasWarning), TIGL_SUCCESS);
}

TEST_F(WingInterpolateXsi, tigl_wing_interpolate_xsi_check_different_combinations)
{
    double intersectionXsi = 0;
    TiglBoolean hasWarning;

    EXPECT_EQ(tiglWingInterpolateXsi(tiglHandle, "Aircraft1_Wing1_Seg1", 0., 0.0, "Aircraft1_Wing1_Seg2", 1., 0.5, "Aircraft1_Wing1_Seg2", 0.5, &intersectionXsi, &hasWarning), TIGL_SUCCESS);
    // this has been checked through a geometric reconstruction in CAD
    EXPECT_NEAR(intersectionXsi, 0.32119, 1e-5);

    EXPECT_EQ(tiglWingInterpolateXsi(tiglHandle, "Aircraft1_Wing1_CompSeg1", 0., 0.0, "Aircraft1_Wing1_Seg2", 1., 0.5, "Aircraft1_Wing1_Seg2", 0.5, &intersectionXsi, &hasWarning), TIGL_SUCCESS);
    EXPECT_NEAR(intersectionXsi, 0.32119, 1e-5);

    EXPECT_EQ(tiglWingInterpolateXsi(tiglHandle, "Aircraft1_Wing1_CompSeg1", 0., 0.0, "Aircraft1_Wing1_CompSeg2", 1., 0.5, "Aircraft1_Wing1_Seg2", 0.5, &intersectionXsi, &hasWarning), TIGL_SUCCESS);
    // this has been checked through a geometric reconstruction in CAD
    EXPECT_NEAR(intersectionXsi, 0.19294, 1e-5);

    EXPECT_EQ(tiglWingInterpolateXsi(tiglHandle, "Aircraft1_Wing1_CompSeg1", 0., 0.0, "Aircraft1_Wing1_CompSeg1", 1., 0.5, "Aircraft1_Wing1_Seg3", 0.5, &intersectionXsi, &hasWarning), TIGL_MATH_ERROR);
    EXPECT_NEAR(intersectionXsi, 0.19294, 1e-5);

}

