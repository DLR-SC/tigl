/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief Tests for testing behavior of the routines for retrieving fuselage surface points.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"


/******************************************************************************/

class TiglFuselageGetPoint : public ::testing::Test
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
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Cpacs2Test", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};


TixiDocumentHandle TiglFuselageGetPoint::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglFuselageGetPoint::tiglHandle = 0;

/**
* Tests tiglFuselageGetPoint with invalid CPACS handle.
*/
TEST_F(TiglFuselageGetPoint, invalidHandle)
{   
    double x, y, z;
    ASSERT_TRUE(tiglFuselageGetPoint(-1, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
    ASSERT_TRUE(tiglFuselageGetPoint(-2, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
    ASSERT_TRUE(tiglFuselageGetPoint(-3, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
}

/**
* Tests tiglFuselageGetPoint with invalid fuselage indices.
*/
TEST_F(TiglFuselageGetPoint, invalidFuselage)
{
    double x, y, z;
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle,      0, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 100000, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetPoint with invalid segment indices.
*/
TEST_F(TiglFuselageGetPoint, invalidSegment)
{
    double x, y, z;
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1,     0, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 10000, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglFuselageGetPoint with invalid eta.
*/
TEST_F(TiglFuselageGetPoint, invalidEta)
{
    double x, y, z;
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, -0.1, 0.0, &x, &y, &z) == TIGL_ERROR);
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1,  1.1, 0.0, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglFuselageGetPoint with invalid zeta.
*/
TEST_F(TiglFuselageGetPoint, invalidZeta)
{
    double x, y, z;
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, -0.1, &x, &y, &z) == TIGL_ERROR);
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0,  1.1, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglFuselageGetPoint with null pointer arguments.
*/
TEST_F(TiglFuselageGetPoint, nullPointerArgument) 
{
    double x, y, z;
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, NULL, &y, &z) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, NULL, &z) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call to tiglFuselageGetPoint.
*/
TEST_F(TiglFuselageGetPoint, success)
{
    double x, y, z;
    double accuracy = 1e-7;
    
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_SUCCESS);
    ASSERT_NEAR(-0.5, x, accuracy);
    
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 0.5, 0.5, &x, &y, &z) == TIGL_SUCCESS);
    ASSERT_NEAR( 0.0, x, accuracy);
    
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 1, 1.0, 1.0, &x, &y, &z) == TIGL_SUCCESS);
    ASSERT_NEAR( 0.5, x, accuracy);
    
    ASSERT_TRUE(tiglFuselageGetPoint(tiglHandle, 1, 2, 1.0, 1.0, &x, &y, &z) == TIGL_SUCCESS);
    ASSERT_NEAR( 1.5, x, accuracy);
}

/**
* Testing a bug in getPointAtAngle.
*/
TEST(TiglFuselageGetPointBugs, getPointAngle)
{
    const char* filename = "TestData/CPACS_30_D150.xml";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    ReturnCode tixiRet = tixiOpenDocument(filename, &tixiHandle);
    ASSERT_TRUE (tixiRet == SUCCESS);
    TiglReturnCode tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
    ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

    double x, y, z;

    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetPointAngle(tiglHandle, 1, 24, 0.5, 0.0, &x, &y, &z));
}

/**
* Testing a bug in getPointAtAngle that provokes an unstable OCCT algorithm if not
* used correctly
*/
TEST(TiglFuselageGetPointBugs, getPointAngleTranslated)
{
    const char* filename = "TestData/D150_v30.xml";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;

    ReturnCode tixiRet = tixiOpenDocument(filename, &tixiHandle);
    ASSERT_TRUE (tixiRet == SUCCESS);
    TiglReturnCode tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
    ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

    double x, y, z;

    // this always worked
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetPointAngleTranslated(tiglHandle, 1, 29, 0.5,  8.1795,  0.19097, 0.027451, &x, &y, &z));
    ASSERT_NEAR(-0.088661, y, 1e-5);

    // this was buggy
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetPointAngleTranslated(tiglHandle, 1, 29, 0.5, -8.1795, -0.19097, 0.027451, &x, &y, &z));
    ASSERT_NEAR(0.088661, y, 1e-5);
}

