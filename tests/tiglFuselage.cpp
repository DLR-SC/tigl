/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2015-01-07 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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
* @brief Tests for testing non classified fuselage functions.
*/

#define _USE_MATH_DEFINES
#include <cmath>

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include <string.h>


/******************************************************************************/

class TiglFuselage : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/CPACS_21_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    virtual void SetUp() {}
    virtual void TearDown() {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};


TixiDocumentHandle TiglFuselage::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglFuselage::tiglHandle = 0;



/**
* Tests for tiglFuselageGetIndex
*/
TEST_F(TiglFuselage, tiglFuselageGetIndex_success)
{
    int fuselageIndex = 0;
    ASSERT_TRUE(tiglFuselageGetIndex(tiglHandle, "D150_VAMP_FL1", &fuselageIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(fuselageIndex == 1);

}

TEST_F(TiglFuselage, tiglFuselageGetIndex_wrongUID)
{
    int fuselageIndex = 0;
    ASSERT_TRUE(tiglFuselageGetIndex(tiglHandle, "invalid_uid", &fuselageIndex) == TIGL_UID_ERROR);
    ASSERT_TRUE(fuselageIndex == -1);
}

TEST_F(TiglFuselage, tiglFuselageGetIndex_nullPtr)
{
    int fuselageIndex = 0;
    ASSERT_TRUE(tiglFuselageGetIndex(tiglHandle, NULL , &fuselageIndex) == TIGL_NULL_POINTER);
}


TEST_F(TiglFuselage, tiglFuselageGetIndex_indexNullPtr)
{
    int fuselageIndex = 0;
    ASSERT_TRUE(tiglFuselageGetIndex(tiglHandle, "D150_VAMP_FL1" , NULL) == TIGL_NULL_POINTER);
}

TEST_F(TiglFuselage, tiglFuselageGetIndex_wrongHandle)
{
    TiglCPACSConfigurationHandle myWrongHandle = -1234;
    int fuselageIndex = 0;
    ASSERT_TRUE(tiglFuselageGetIndex(myWrongHandle, "D150_VAMP_FL1" , &fuselageIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetSegmentIndex
*/
TEST_F(TiglFuselage, tiglFuselageGetSegmentIndex_success)
{
    int segmentIndex = 0;
    int fuselageIndex = 0;
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSegmentIndex(tiglHandle, "D150_VAMP_FL1_Seg1", &segmentIndex, &fuselageIndex));
    ASSERT_EQ(1, segmentIndex);
    ASSERT_EQ(1, fuselageIndex);

    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSegmentIndex(tiglHandle, "D150_VAMP_FL1_Seg2", &segmentIndex, &fuselageIndex));
    ASSERT_EQ(2, segmentIndex);
    ASSERT_EQ(1, fuselageIndex);

    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSegmentIndex(tiglHandle, "D150_VAMP_FL1_Seg3", &segmentIndex, &fuselageIndex));
    ASSERT_EQ(3, segmentIndex);
    ASSERT_EQ(1, fuselageIndex);
}

TEST_F(TiglFuselage, tiglFuselageGetSegmentIndex_wrongUID)
{
    int segmentIndex = 0;
    int fuselageIndex    = 0;
    ASSERT_TRUE(tiglFuselageGetSegmentIndex(tiglHandle, "invalid_seg_name", &segmentIndex, &fuselageIndex) == TIGL_UID_ERROR);
}

TEST_F(TiglFuselage, tiglFuselageGetSegmentIndex_nullPtr)
{
    int segmentIndex = 0;
    int fuselageIndex = 0;
    ASSERT_TRUE(tiglFuselageGetSegmentIndex(tiglHandle, NULL, &segmentIndex, &fuselageIndex) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetSegmentIndex(tiglHandle, "D150_VAMP_FL1_Seg1", NULL, &fuselageIndex) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetSegmentIndex(tiglHandle, "D150_VAMP_FL1_Seg1", &segmentIndex, NULL) == TIGL_NULL_POINTER);
}

TEST_F(TiglFuselage, tiglFuselageGetSegmentIndex_wrongHandle)
{
    TiglCPACSConfigurationHandle myWrongHandle = -1234;
    int segmentIndex = 0;
    int fuselageIndex = 0;
    ASSERT_TRUE(tiglFuselageGetSegmentIndex(myWrongHandle, "D150_VAMP_FL1_Seg1", &segmentIndex, &fuselageIndex) == TIGL_NOT_FOUND);
}

TEST(TiglSimpleFuselage, getSurfaceArea_FullModel)
{
    const char* filename = "TestData/simpletest.cpacs.xml";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    ASSERT_EQ(SUCCESS, tixiOpenDocument(filename, &tixiHandle));
    ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

    double area = 0.;
    tiglFuselageGetSurfaceArea(tiglHandle, 1, &area);

    ASSERT_NEAR(2.*M_PI, area, 2.*M_PI * 0.1);

    tiglCloseCPACSConfiguration(tiglHandle);
    tixiCloseDocument(tixiHandle);
}

TEST(TiglSimpleFuselage, getSurfaceArea_HalfModel)
{
    const char* filename = "TestData/simpletest-halfmodel.cpacs.xml";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    ASSERT_EQ(SUCCESS, tixiOpenDocument(filename, &tixiHandle));
    ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));

    double area = 0.;
    tiglFuselageGetSurfaceArea(tiglHandle, 1, &area);

    ASSERT_NEAR(M_PI, area, M_PI * 0.1);

    tiglCloseCPACSConfiguration(tiglHandle);
    tixiCloseDocument(tixiHandle);
}
