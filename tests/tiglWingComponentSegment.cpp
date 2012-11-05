/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2012-10-17 Martin Siggel <Martin.Siggel@dlr.de>
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
* @brief Tests for testing behavior of wing component segment functions.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#include "CCPACSConfigurationManager.h"
#include "CCPACSWing.h"
#include "CCPACSWingComponentSegment.h"

/******************************************************************************/

class WingComponentSegment : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
        char* filename = "TestData/CPACS_21_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
  }

  static void TearDownTestCase() {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
  }
  
  virtual void SetUp() {}
  virtual void TearDown() {}
  

  static TixiDocumentHandle           tixiHandle;
  static TiglCPACSConfigurationHandle tiglHandle;
};


TixiDocumentHandle WingComponentSegment::tixiHandle = 0;
TiglCPACSConfigurationHandle WingComponentSegment::tiglHandle = 0;

/******************************************************************************/


class WingComponentSegmentSimple : public ::testing::Test {
 protected:
  virtual void SetUp() {
        char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "Cpacs2Test", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
  }

  virtual void TearDown() {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
  }

  TixiDocumentHandle           tixiHandle;
  TiglCPACSConfigurationHandle tiglHandle;
};

TEST_F(WingComponentSegment, tiglWingGetComponentSegmentCount_success)
{
	int numCompSeg = 0;
	ASSERT_TRUE(tiglWingGetComponentSegmentCount(tiglHandle, 1, &numCompSeg) == TIGL_SUCCESS);
	ASSERT_TRUE(numCompSeg == 1);
}

TEST_F(WingComponentSegment, tiglWingGetComponentSegmentCount_wrongWing)
{
	int numCompSeg = 0;
	// this example contains only 3 wings
	ASSERT_TRUE(tiglWingGetComponentSegmentCount(tiglHandle, 4, &numCompSeg) == TIGL_INDEX_ERROR);
}

TEST_F(WingComponentSegment, tiglWingGetComponentSegmentCount_nullPtr)
{
	// this example contains only 3 wings
	ASSERT_TRUE(tiglWingGetComponentSegmentCount(tiglHandle, 1, NULL) == TIGL_NULL_POINTER);
}

TEST_F(WingComponentSegment, tiglWingGetComponentSegmentCount_wrongHandle)
{
	int numCompSeg = 0;
	TiglCPACSConfigurationHandle myWrongHandle = -1234;
	ASSERT_TRUE(tiglWingGetComponentSegmentCount(myWrongHandle, 1, &numCompSeg) == TIGL_NOT_FOUND);
}

TEST_F(WingComponentSegment, tiglWingGetComponentSegmentUID_success)
{
    char * uid = NULL;
    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle, 1, 1, &uid) == TIGL_SUCCESS);
    ASSERT_STREQ("D150_VAMP_W1_CompSeg1", uid);

    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle, 2, 1, &uid) == TIGL_SUCCESS);
    ASSERT_STREQ("D150_VAMP_HL1_CompSeg1", uid);

    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle, 3, 1, &uid) == TIGL_SUCCESS);
    ASSERT_STREQ("D150_VAMP_SL1_CompSeg1", uid);
}

TEST_F(WingComponentSegment, tiglWingGetComponentSegmentUID_indexFail)
{
    char * uid = NULL;
    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle, 1, 2, &uid) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle,-1, 1, &uid) == TIGL_INDEX_ERROR);
    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle, 4, 1, &uid) == TIGL_INDEX_ERROR);
}

TEST_F(WingComponentSegment, tiglWingComponentSegmentIndex_success)
{
    int segment = 0;
    ASSERT_TRUE(tiglWingGetComponentSegmentIndex(tiglHandle, 1, "D150_VAMP_W1_CompSeg1", &segment) == TIGL_SUCCESS);
    ASSERT_TRUE(segment == 1);

    segment = 0;
    ASSERT_TRUE(tiglWingGetComponentSegmentIndex(tiglHandle, 2, "D150_VAMP_HL1_CompSeg1", &segment) == TIGL_SUCCESS);
    ASSERT_TRUE(segment == 1);

    segment = 0;
    ASSERT_TRUE(tiglWingGetComponentSegmentIndex(tiglHandle, 3, "D150_VAMP_SL1_CompSeg1", &segment) == TIGL_SUCCESS);
    ASSERT_TRUE(segment == 1);
}

TEST_F(WingComponentSegment, tiglWingComponentSegmentIndex_wrongUID)
{
    int segment = 0;
    // component segment UID exist, but on wing 1
    ASSERT_TRUE(tiglWingGetComponentSegmentIndex(tiglHandle, 2, "D150_VAMP_W1_CompSeg1", &segment) == TIGL_UID_ERROR);

    ASSERT_TRUE(tiglWingGetComponentSegmentIndex(tiglHandle, 1, "invalid_uid", &segment) == TIGL_UID_ERROR);
}


TEST_F(WingComponentSegment, tiglWingComponentGetEtaXsi_success)
{
	double eta = 0.1103;
	double xsi = 0.812922;
	char * wingUID = NULL;
	char * segmentUID = NULL;
	double segmentEta = 0., segmentXsi = 0.;

	TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_VAMP_W1_CompSeg1", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	ASSERT_TRUE( ret == TIGL_SUCCESS);
	ASSERT_STREQ("D150_VAMP_W1", wingUID);
	free(wingUID); wingUID = NULL;
	free(segmentUID); segmentUID = NULL;

	// test for a bug  in tigl 2.0.2, occurs when component segment does not lie on first wing
	eta = 0.16;
	xsi = 0.577506;
	ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_VAMP_HL1_CompSeg1", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	ASSERT_TRUE(ret == TIGL_SUCCESS);
	ASSERT_STREQ("D150_VAMP_HL1", wingUID);
	free(wingUID); wingUID = NULL;
	free(segmentUID); segmentUID = NULL;
}

TEST_F(WingComponentSegment, tiglWingComponentGetEtaXsi_wrongUID){
	double eta = 0.1103;
	double xsi = 0.812922;
	char * wingUID = NULL;
	char * segmentUID = NULL;
	double segmentEta = 0., segmentXsi = 0.;

	TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "invalid_comp_seg", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	ASSERT_TRUE(ret == TIGL_UID_ERROR);
	free(wingUID); wingUID = NULL;
	free(segmentUID); segmentUID = NULL;
}

TEST_F(WingComponentSegmentSimple, getPointInternal_accuracy)
{
    int compseg = 1;
    // now we have do use the internal interface as we currently have no public api for this
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(compseg);

    gp_Pnt point = segment.GetPoint(0.25, 0.5);
    ASSERT_NEAR(point.X(), 0.5, 1e-7);
    ASSERT_NEAR(point.Y(), 0.5, 1e-7);

    point = segment.GetPoint(0.5, 0.5);
    ASSERT_NEAR(point.X(), 0.5, 1e-7);
    ASSERT_NEAR(point.Y(), 1.0, 1e-7);

    point = segment.GetPoint(0.75, 0.5);
    ASSERT_NEAR(point.X(), 0.625, 1e-7);
    ASSERT_NEAR(point.Y(), 1.5, 1e-7);
}

TEST_F(WingComponentSegmentSimple, tiglWingComponentSegmentPointGetSegmentEtaXsi_success1){
        // now the tests
        double csEta = 0., csXsi = 0.;
        char *wingUID = NULL, *segmentUID = NULL;
        tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "WING_CS1", 0.25, 0.5, &wingUID, &segmentUID, &csEta, &csXsi);
        ASSERT_STREQ("Wing",wingUID);
        ASSERT_STREQ("Cpacs2Test_Wing_Seg_1_2", segmentUID);
        ASSERT_NEAR(csEta, 0.5, 1e-7);
        ASSERT_NEAR(csXsi, 0.5, 1e-7);
        free(wingUID); wingUID = NULL;
        free(segmentUID); segmentUID = NULL;

        tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "WING_CS1", 0.49, 0.5, &wingUID, &segmentUID, &csEta, &csXsi);
        ASSERT_STREQ("Wing",wingUID);
        ASSERT_STREQ("Cpacs2Test_Wing_Seg_1_2", segmentUID);
        ASSERT_NEAR(csEta, 0.98, 1e-7);
        ASSERT_NEAR(csXsi, 0.5, 1e-7);
        free(wingUID); wingUID = NULL;
        free(segmentUID); segmentUID = NULL;

        tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "WING_CS1", 0.75, 0.5, &wingUID, &segmentUID, &csEta, &csXsi);
        ASSERT_STREQ("Wing",wingUID);
        ASSERT_STREQ("Cpacs2Test_Wing_Seg_2_3", segmentUID);
        ASSERT_NEAR(csEta, 0.5, 1e-7);
        ASSERT_NEAR(csXsi, 0.5, 1e-7);
        free(wingUID); wingUID = NULL;
        free(segmentUID); segmentUID = NULL;
}