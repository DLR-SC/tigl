/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
* @brief Tests for testing behavior of the routines for retrieving surface points.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

/******************************************************************************/

class WingGetPoint : public ::testing::Test 
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

class WingGetPointSimple : public ::testing::Test 
{
protected:
    virtual void SetUp() 
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

    virtual void TearDown() 
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};



TixiDocumentHandle WingGetPoint::tixiHandle = 0;
TiglCPACSConfigurationHandle WingGetPoint::tiglHandle = 0;


/******************************************************************************/


/**
* Tests tiglWingGetUpperPoint with invalid CPACS handle.
*/
TEST_F(WingGetPoint, tiglWingGetUpperPoint_invalidHandle)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetUpperPoint(-1, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetLowerPoint with invalid CPACS handle.
*/
TEST_F(WingGetPoint, tiglWingGetLowerPoint_invalidHandle)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetLowerPoint(-1, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetUpperPoint with invalid wing indices.
*/
TEST_F(WingGetPoint, tiglWingGetUpperPoint_invalidWing)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle,      0, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 100000, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid wing indices.
*/
TEST_F(WingGetPoint, tiglWingGetLowerPoint_invalidWing)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle,      0, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 100000, 1, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with invalid segment indices.
*/
TEST_F(WingGetPoint, tiglWingGetUpperPoint_invalidSegment)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1,     0, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 10000, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid segment indices.
*/
TEST_F(WingGetPoint, tiglWingGetLowerPoint_invalidSegment)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1,     0, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 10000, 0.0, 0.0, &x, &y, &z) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with invalid eta.
*/
TEST_F(WingGetPoint, tiglWingGetUpperPoint_invalidEta)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, -1.0, 0.0, &x, &y, &z) == TIGL_ERROR);
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1,  1.1, 0.0, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid eta.
*/
TEST_F(WingGetPoint, tiglWingGetLowerPoint_invalidEta)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, -1.0, 0.0, &x, &y, &z) == TIGL_ERROR);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1,  1.1, 0.0, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with invalid xsi.
*/
TEST_F(WingGetPoint, tiglWingGetUpperPoint_invalidXsi)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, -1.0, &x, &y, &z) == TIGL_ERROR);
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0,  1.1, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetLowerPoint with invalid xsi.
*/
TEST_F(WingGetPoint, tiglWingGetLowerPoint_invalidXsi)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, -1.0, &x, &y, &z) == TIGL_ERROR);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0,  1.1, &x, &y, &z) == TIGL_ERROR);
}

/**
* Tests tiglWingGetUpperPoint with null pointer arguments.
*/
TEST_F(WingGetPoint, tiglWingGetUpperPoint_nullPointerArgument) 
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, NULL, &y, &z) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, NULL, &z) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests tiglWingGetLowerPoint with null pointer arguments.
*/
TEST_F(WingGetPoint, tiglWingGetLowerPoint_nullPointerArgument) 
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, NULL, &y, &z) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, NULL, &z) == TIGL_NULL_POINTER);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests successfull call to tiglWingGetUpperPoint.
*/
TEST_F(WingGetPoint, tiglWingGetUpperPoint_success)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_SUCCESS);   // leading root
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.5, 0.5, &x, &y, &z) == TIGL_SUCCESS);   // middle
    ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 1.0, 1.0, &x, &y, &z) == TIGL_SUCCESS);   // trailing edge
}

/**
* Tests successfull call to tiglWingGetLowerPoint.
*/
TEST_F(WingGetPoint, tiglWingGetLowerPoint_success)
{
    double x, y, z;
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, 0.0, &x, &y, &z) == TIGL_SUCCESS);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.5, 0.5, &x, &y, &z) == TIGL_SUCCESS);
    ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 1.0, 1.0, &x, &y, &z) == TIGL_SUCCESS);
}

TEST_F(WingGetPointSimple, checkCamberLine)
{
    //inner wing profile
    for (double xsi = 0; xsi <= 1.0; xsi += 0.001) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }

    // outer wing profile
    for (double xsi = 0; xsi <= 1.0; xsi += 0.001) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 1.0, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 1.0, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }

    // middle wing profile
    for (double xsi = 0; xsi <= 1.0; xsi += 0.001) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.5, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.5, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }

    // middle wing profile, segment 2
    for (double xsi = 0; xsi <= 1.0; xsi += 0.001) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 2, 0.5, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 2, 0.5, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }
}
