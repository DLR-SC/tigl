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
    for (double xsi = 0; xsi <= 1.0; xsi += 0.01) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.0, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.0, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }

    // outer wing profile
    for (double xsi = 0; xsi <= 1.0; xsi += 0.01) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 1.0, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 1.0, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }

    // middle wing profile
    for (double xsi = 0; xsi <= 1.0; xsi += 0.01) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 1, 0.5, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.5, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }

    // middle wing profile, segment 2
    for (double xsi = 0; xsi <= 1.0; xsi += 0.01) {
        double xl,yl, zl;
        ASSERT_TRUE(tiglWingGetLowerPoint(tiglHandle, 1, 2, 0.5, xsi, &xl, &yl, &zl) == TIGL_SUCCESS);

        double xu,yu, zu;
        ASSERT_TRUE(tiglWingGetUpperPoint(tiglHandle, 1, 2, 0.5, xsi, &xu, &yu, &zu) == TIGL_SUCCESS);

        ASSERT_NEAR(0., zu+zl, 1e-9);
    }
}

TEST_F(WingGetPointSimple, getPointDirection)
{
    double px, py, pz, distance;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.5, 0.5, 0., 0., 1., &px, &py, &pz, &distance));
    ASSERT_NEAR(0.5, px, 1e-9);
    ASSERT_NEAR(0.5, py, 1e-9);
    ASSERT_NEAR(0.0529403, pz, 1e-6);
    ASSERT_LE(distance, 1e-10);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetLowerPointAtDirection(tiglHandle, 1, 1, 0.5, 0.5, 0., 0., 1., &px, &py, &pz, &distance));
    ASSERT_NEAR(0.5, px, 1e-9);
    ASSERT_NEAR(0.5, py, 1e-9);
    ASSERT_NEAR(-0.0529403, pz, 1e-6);
    ASSERT_LE(distance, 1e-10);
    
    ASSERT_EQ(TIGL_SUCCESS,    tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.5, 0.9, 0., 1., 0., &px, &py, &pz, &distance));
    ASSERT_GT(distance, 1e-3);
    ASSERT_EQ(TIGL_SUCCESS,    tiglWingGetLowerPointAtDirection(tiglHandle, 1, 1, 0.5, 0.9, 0., 1., 0., &px, &py, &pz, &distance));
    ASSERT_GT(distance, 1e-3);
    ASSERT_EQ(TIGL_MATH_ERROR, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.5, 0.5, 0., 0., 0., &px, &py, &pz, &distance));
    ASSERT_EQ(TIGL_MATH_ERROR, tiglWingGetLowerPointAtDirection(tiglHandle, 1, 1, 0.5, 0.5, 0., 0., 0., &px, &py, &pz, &distance));
    
    ASSERT_EQ(TIGL_NULL_POINTER, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.5, 0.5, 0., 0., 1., NULL, &py, &pz, &distance));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.5, 0.5, 0., 0., 1., &px, NULL, &pz, &distance));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.5, 0.5, 0., 0., 1., &px, &py, NULL, &distance));
    
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglWingGetUpperPointAtDirection(tiglHandle, 0, 1, 0.5, 0.5, 0., 0., 1., &px, &py, &pz, &distance));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 0, 0.5, 0.5, 0., 0., 1., &px, &py, &pz, &distance));
}

TEST_F(WingGetPointSimple, getPointDirection_smallmiss)
{
    double px, py, pz, distance;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.0, 0.5, 0., -0.01, 1., &px, &py, &pz, &distance));
    ASSERT_NEAR(0.5, px, 1e-6);
    ASSERT_NEAR(0.0, py, 1e-6);
    ASSERT_LT(distance, 1e-3);
    ASSERT_GT(distance, 1e-10);
}

TEST_F(WingGetPointSimple, getChordPoint_success)
{
    double px, py, pz;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 1, 1, 0.0, 0.0, &px, &py, &pz));
    ASSERT_NEAR(0.0, px, 1e-7);
    ASSERT_NEAR(0.0, py, 1e-7);
    ASSERT_NEAR(0.0, pz, 1e-7);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 1, 1, 1.0, 0.0, &px, &py, &pz));
    ASSERT_NEAR(0.0, px, 1e-7);
    ASSERT_NEAR(1.0, py, 1e-7);
    ASSERT_NEAR(0.0, pz, 1e-7);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 1, 1, 0.0, 1.0, &px, &py, &pz));
    ASSERT_NEAR(1.0, px, 1e-7);
    ASSERT_NEAR(0.0, py, 1e-7);
    ASSERT_NEAR(0.0, pz, 1e-7);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 1, 1, 1.0, 1.0, &px, &py, &pz));
    ASSERT_NEAR(1.0, px, 1e-7);
    ASSERT_NEAR(1.0, py, 1e-7);
    ASSERT_NEAR(0.0, pz, 1e-7);
}

TEST_F(WingGetPointSimple, getChordPoint_invalidArgs)
{
    double px, py, pz;
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglWingGetChordPoint(tiglHandle, 1, 0, 0.0, 0.0, &px, &py, &pz));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglWingGetChordPoint(tiglHandle, 1, 3, 0.0, 0.0, &px, &py, &pz));
    ASSERT_EQ(TIGL_NOT_FOUND  , tiglWingGetChordPoint(-1, 1, 1, 0.0, 0.0, &px, &py, &pz));
    ASSERT_EQ(TIGL_NULL_POINTER,tiglWingGetChordPoint(tiglHandle, 1, 1, 0.0, 0.0, NULL, &py, &pz));
    ASSERT_EQ(TIGL_NULL_POINTER,tiglWingGetChordPoint(tiglHandle, 1, 1, 0.0, 0.0, &px, NULL, &pz));
    ASSERT_EQ(TIGL_NULL_POINTER,tiglWingGetChordPoint(tiglHandle, 1, 1, 0.0, 0.0, &px, &py, NULL));
}

TEST_F(WingGetPointSimple, getChordNormal_success)
{
    double nx, ny, nz;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordNormal(tiglHandle, 1, 1, 0.0, 0.0, &nx, &ny, &nz));
    ASSERT_NEAR(0.0, nx, 1e-7);
    ASSERT_NEAR(0.0, ny, 1e-7);
    ASSERT_NEAR(1.0, nz, 1e-7);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordNormal(tiglHandle, 1, 1, 1.0, 0.0, &nx, &ny, &nz));
    ASSERT_NEAR(0.0, nx, 1e-7);
    ASSERT_NEAR(0.0, ny, 1e-7);
    ASSERT_NEAR(1.0, nz, 1e-7);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordNormal(tiglHandle, 1, 1, 0.0, 1.0, &nx, &ny, &nz));
    ASSERT_NEAR(0.0, nx, 1e-7);
    ASSERT_NEAR(0.0, ny, 1e-7);
    ASSERT_NEAR(1.0, nz, 1e-7);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordNormal(tiglHandle, 1, 1, 1.0, 1.0, &nx, &ny, &nz));
    ASSERT_NEAR(0.0, nx, 1e-7);
    ASSERT_NEAR(0.0, ny, 1e-7);
    ASSERT_NEAR(1.0, nz, 1e-7);
    
    // now on the second segment
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordNormal(tiglHandle, 1, 2, 0.5, 0.5, &nx, &ny, &nz));
    ASSERT_NEAR(0.0, nx, 1e-7);
    ASSERT_NEAR(0.0, ny, 1e-7);
    ASSERT_NEAR(1.0, nz, 1e-7);
}

TEST_F(WingGetPointSimple, getChordNormal_invalidArgs)
{
    double nx, ny, nz;
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglWingGetChordNormal(tiglHandle, 1, 0, 0.0, 0.0, &nx, &ny, &nz));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglWingGetChordNormal(tiglHandle, 1, 3, 0.0, 0.0, &nx, &ny, &nz));
    ASSERT_EQ(TIGL_NOT_FOUND  , tiglWingGetChordNormal(-1, 1, 1, 0.0, 0.0, &nx, &ny, &nz));
    ASSERT_EQ(TIGL_NULL_POINTER,tiglWingGetChordNormal(tiglHandle, 1, 1, 0.0, 0.0, NULL, &ny, &nz));
    ASSERT_EQ(TIGL_NULL_POINTER,tiglWingGetChordNormal(tiglHandle, 1, 1, 0.0, 0.0, &nx, NULL, &nz));
    ASSERT_EQ(TIGL_NULL_POINTER,tiglWingGetChordNormal(tiglHandle, 1, 1, 0.0, 0.0, &nx, &ny, NULL));
}

TEST(WingGetPointBugs, getPointDirection_Fuehrer)
{
    const char* filename = "TestData/WingGetPointBug1.xml";

    TixiDocumentHandle tiglHandle = -1;
    TiglCPACSConfigurationHandle tixiHandle = -1;

    ReturnCode tixiRet = tixiOpenDocument(filename, &tixiHandle);
    ASSERT_TRUE (tixiRet == SUCCESS);
    TiglReturnCode tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
    ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    
    double px, py, pz;
    double dirx, diry, dirz, distance;
    double tolerance = 1e-10;
    dirx = 0.0; diry =  -0.069756473744125316; dirz = 0.99756405025982431;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.04, 0.0001, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.06, 0.0001, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.08, 0.0001, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.088, 0.004, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.0, 0.0001, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.0, 0.004, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.0, 0.036, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.0, 0.07568, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.0, 0.16569, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPointAtDirection(tiglHandle, 1, 1, 0.0, 1.0, dirx, diry, dirz, &px, &py, &pz, &distance));
    ASSERT_LT(distance, tolerance);
}
