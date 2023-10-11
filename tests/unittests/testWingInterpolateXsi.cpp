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
        const char* filename = "TestData/test_wing_segment_special_without_component_segments.xml";
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

TEST_F(WingInterpolateXsi, tigl_wing_interpolate_xsi_no_component_segments)
{    
    double intersectionXsi = 0;
    TiglBoolean hasWarning;

    EXPECT_EQ(tiglWingInterpolateXsi(tiglHandle, "Aircraft1_Wing1_Seg2", 0., 0.0, "Aircraft1_Wing1_Seg2", 1., 0.5, "Aircraft1_Wing1_Seg2", 0.5, &intersectionXsi, &hasWarning), TIGL_SUCCESS);

    std::cout << "intersectionXsi: " << intersectionXsi << std::endl;
}

