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
* @brief Tests for testing behavior of the TIGL intersection calculation routines.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#include "CTiglIntersectionCalculation.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWing.h"
#include "CCPACSFuselage.h"
#include "CNamedShape.h"

class TiglIntersectionCalculation : public ::testing::Test
{
protected:
    void SetUp() OVERRIDE
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

    void TearDown() OVERRIDE
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    TixiDocumentHandle           tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};


/**
* Tests 
*/
TEST_F(TiglIntersectionCalculation, tiglIntersection_FuselageWingIntersects)
{
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSFuselage& fuselage = config.GetFuselage(1);

    const TopoDS_Shape& wingShape = wing.GetLoft()->Shape();
    const TopoDS_Shape& fuselageShape = fuselage.GetLoft()->Shape();

    tigl::CTiglIntersectionCalculation iCalc(&config.GetShapeCache(), fuselage.GetUID(), wing.GetUID(), fuselageShape, wingShape);

    ASSERT_EQ(1, iCalc.GetCountIntersectionLines());
}

TEST_F(TiglIntersectionCalculation, tiglIntersectComponents)
{
    char* id = NULL;
    int count = 0;
    double px, py, pz;
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectComponents(tiglHandle, "Wing", "SimpleFuselage", &id));
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetLineCount(tiglHandle, id, &count));
    ASSERT_EQ(1, count);
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetPoint(tiglHandle, id, 1, 0., &px, &py, &pz));
    
    // tests errors of tiglIntersectGetPoint since we already have a result
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglIntersectGetPoint(-1, id, 1, 0., &px, &py, &pz));
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglIntersectGetPoint(tiglHandle, "1234567890", 1, 0., &px, &py, &pz));
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglIntersectGetPoint(tiglHandle, id, 0, 0., &px, &py, &pz));
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglIntersectGetPoint(tiglHandle, id, 2, 0., &px, &py, &pz));
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglIntersectGetPoint(tiglHandle, id, 1, -0.5, &px, &py, &pz));
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglIntersectGetPoint(tiglHandle, id, 1,  1.5, &px, &py, &pz));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectGetPoint(tiglHandle, id, 1, 0.5, NULL, &py, &pz));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectGetPoint(tiglHandle, id, 1, 0.5, &px, NULL, &pz));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectGetPoint(tiglHandle, id, 1, 0.5, &px, &py, NULL));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectGetPoint(tiglHandle, NULL, 1, 0.5, &px, &py, &pz));
}

TEST_F(TiglIntersectionCalculation, tiglIntersectWithPlane)
{
    char* id = NULL;
    int count = 0;
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlane(tiglHandle, "SimpleFuselage", 0., 0., 0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetLineCount(tiglHandle, id, &count));
    ASSERT_EQ(1, count);

    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlane(tiglHandle, "SimpleFuselage", 0., 0., 2., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetLineCount(tiglHandle, id, &count));
    ASSERT_EQ(0, count);

    ASSERT_EQ(TIGL_NOT_FOUND,    tiglIntersectWithPlane(-1, "SimpleFuselage", 0., 0., 0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_UID_ERROR,    tiglIntersectWithPlane(tiglHandle, "UNKNOWN_UID", 0., 0., 0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectWithPlane(tiglHandle, NULL, 0., 0., 0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectWithPlane(tiglHandle, "SimpleFuselage", 0., 0., 0., 0., 0., 1., NULL));
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglIntersectWithPlane(tiglHandle, "SimpleFuselage", 0., 0., 0., 0., 0., 0., &id));
}

TEST_F(TiglIntersectionCalculation, tiglIntersectWithPlaneSegment)
{
    char* id1 = NULL;
    int count1 = -1;
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage", 0.5, -.25, 0., 0.5, .25, 0., 0., 0., 1., &id1));
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetLineCount(tiglHandle, id1, &count1));
    ASSERT_EQ(2, count1);

    char* id2 = NULL;
    int count2 = -1;
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage", -1., -.25, 0., -1., .25, 0., 0., 0., 1., &id2));
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetLineCount(tiglHandle, id2, &count2));
    ASSERT_EQ(0, count2);

    char* id = NULL;
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglIntersectWithPlaneSegment(-1, "SimpleFuselage", 0.5, -5., 0., 0.5, 5., 0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_UID_ERROR,    tiglIntersectWithPlaneSegment(tiglHandle, "UNKNOWN_UID", 0.5, -5., 0., 0.5, 5., 0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectWithPlaneSegment(tiglHandle, NULL,  0.5, -5., 0., 0.5, 5.,0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage",  0.5, -5., 0., 0.5, 5.,0., 0., 0., 1., NULL));
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage",  0.5, -5., 0., 0.5, 5.,0., 0., 0., 0., &id));
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage",  0.5,  5., 0., 0.5, 5.,0., 0., 0., 1., &id));
}

TEST_F(TiglIntersectionCalculation, tiglGetCurveIntersection)
{
    // first, calculate two intersections

    char* id1 = NULL;
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlane(tiglHandle, "SimpleFuselage", 0., 0., 0., 0., 1., 0., &id1));

    char* id2 = NULL;
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage", 0.5, -1.25, 0., 0.5, 1.25, 0., 0., 0., 1., &id2));

    // next, calculate the intersection points of the two intersection lines

    double tol = 1e-4;
    char* id_result = NULL;
    ASSERT_EQ(TIGL_SUCCESS, tiglGetCurveIntersection(tiglHandle, id1, 1, id2, 1, tol, &id_result) );

    // query the number of points
    int numPoints;
    ASSERT_EQ(TIGL_SUCCESS, tiglGetCurveIntersectionCount(tiglHandle, id_result, &numPoints) );
    EXPECT_EQ( 2, numPoints );

    // query a point by its index
    double p1x, p1y, p1z;
    ASSERT_EQ(TIGL_SUCCESS, tiglGetCurveIntersectionPoint(tiglHandle, id_result, 1,  &p1x, &p1y, &p1z) );
    ASSERT_NEAR(  0.5, p1x, tol);
    ASSERT_NEAR(  0.0, p1y, tol);
    ASSERT_NEAR( -0.5, p1z, tol);

    double p2x, p2y, p2z;
    ASSERT_EQ(TIGL_SUCCESS, tiglGetCurveIntersectionPoint(tiglHandle, id_result, 2,  &p2x, &p2y, &p2z) );
    ASSERT_NEAR(  0.5, p2x, tol);
    ASSERT_NEAR(  0.0, p2y, tol);
    ASSERT_NEAR(  0.5, p2z, tol);

    // get the parameter of an intersection point
    double eta1;
    ASSERT_EQ(TIGL_SUCCESS, tiglGetCurveParameter (tiglHandle, id1, 1, p1x, p1y, p1z, &eta1) );

    double eta2;
    ASSERT_EQ(TIGL_SUCCESS, tiglGetCurveParameter (tiglHandle, id2, 1, p1x, p1y, p1z,  &eta2) );

    double eta3;
    ASSERT_EQ(TIGL_SUCCESS, tiglGetCurveParameter (tiglHandle, id2, 1, 0., 0., 0.,  &eta3) );


    // check errorcodes of tiglGetCurveIntersection
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglGetCurveIntersection(-1, id1, 1, id2, 1, tol, &id_result) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersection(tiglHandle, NULL, 1, id2, 1, tol, &id_result) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersection(tiglHandle, id1, 1, NULL, 1, tol, &id_result) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersection(tiglHandle, id1, 1, id2, 1, tol, NULL) );
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglGetCurveIntersection(tiglHandle, id1, 2, id2, 1, tol, &id_result) );
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglGetCurveIntersection(tiglHandle, id1, 1, id2, 2, tol, &id_result) );
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglGetCurveIntersection(tiglHandle, id1, 1, id2, 1, -1, &id_result) );

    // check errorcodes of tiglGetCurveIntersectionCount
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglGetCurveIntersectionCount(-1, id_result, &numPoints) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersectionCount(tiglHandle, NULL, &numPoints) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersectionCount(tiglHandle, id_result, NULL) );

    // check errorcodes of tiglGetCurveIntersectionPoint
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglGetCurveIntersectionPoint(-1, id_result, 2,  &p2x, &p2y, &p2z) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersectionPoint(tiglHandle, NULL, 2,  &p2x, &p2y, &p2z) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersectionPoint(tiglHandle, id_result, 2,  NULL, &p2y, &p2z) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersectionPoint(tiglHandle, id_result, 2,  &p2x, NULL, &p2z) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveIntersectionPoint(tiglHandle, id_result, 2,  &p2x, &p2y, NULL) );
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglGetCurveIntersectionPoint(tiglHandle, id_result, 3,  &p2x, &p2y, &p2z) );

    // check errorcodes of tiglGetCurveParameter
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglGetCurveParameter (-1, id1, 1, p1x, p1y, p1z, &eta1) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveParameter (tiglHandle, NULL, 1, p1x, p1y, p1z, &eta1) );
    ASSERT_EQ(TIGL_NULL_POINTER, tiglGetCurveParameter (tiglHandle, id1, 1, p1x, p1y, p1z, NULL) );
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglGetCurveParameter (tiglHandle, id1, 2, p1x, p1y, p1z, &eta1) );

}

TEST_F(TiglIntersectionCalculation, tiglIntersectComponents_Errors)
{
    char* id = NULL;
    ASSERT_EQ(TIGL_UID_ERROR,    tiglIntersectComponents(tiglHandle, "UNKNOWN_UID", "SimpleFuselage", &id));
    ASSERT_EQ(TIGL_UID_ERROR,    tiglIntersectComponents(tiglHandle, "Wing", "UNKNOWN_UID", &id));
    ASSERT_EQ(TIGL_NOT_FOUND,    tiglIntersectComponents(-1, "Wing", "SimpleFuselage", &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectComponents(tiglHandle, NULL, "SimpleFuselage", &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectComponents(tiglHandle, "Wing", NULL, &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectComponents(tiglHandle, "Wing", "SimpleFuselage", NULL));
}

TEST_F(TiglIntersectionCalculation, tiglIntersectGetLineCount_Errors)
{
    char* id = NULL;
    int count = 0;
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectGetLineCount(-1, id, &count));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectGetLineCount(tiglHandle, "myid", NULL));
    // lets hope the id is invalid
    ASSERT_EQ(TIGL_NOT_FOUND, tiglIntersectGetLineCount(tiglHandle, "1234567890", &count));
}

