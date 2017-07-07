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
    char* id = NULL;
    int count = 0;
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage", 0.5, -5., 0.,0.5,5.,0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetLineCount(tiglHandle, id, &count));
    ASSERT_EQ(1, count);

    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage", -1., -5., 0.,-1.,5.,0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_SUCCESS, tiglIntersectGetLineCount(tiglHandle, id, &count));
    ASSERT_EQ(0, count);

    ASSERT_EQ(TIGL_NOT_FOUND,    tiglIntersectWithPlaneSegment(-1, "SimpleFuselage", 0.5, -5., 0.,0.5,5.,0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_UID_ERROR,    tiglIntersectWithPlaneSegment(tiglHandle, "UNKNOWN_UID", 0.5, -5., 0.,0.5,5.,0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectWithPlaneSegment(tiglHandle, NULL,  0.5, -5., 0.,0.5,5.,0., 0., 0., 1., &id));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage",  0.5, -5., 0.,0.5,5.,0., 0., 0., 1., NULL));
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage",  0.5, -5., 0.,0.5,5.,0., 0., 0., 0., &id));
    ASSERT_EQ(TIGL_MATH_ERROR,   tiglIntersectWithPlaneSegment(tiglHandle, "SimpleFuselage",  0.5,  5., 0.,0.5,5.,0., 0., 0., 1., &id));
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

