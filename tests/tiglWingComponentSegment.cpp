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

/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

class WingComponentSegment : public ::testing::Test {
 protected:
  virtual void SetUp() {
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

  virtual void TearDown() {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
  }
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
    ASSERT_TRUE(strcmp(uid, "D150_VAMP_W1_CompSeg1") == 0);

    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle, 2, 1, &uid) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(uid, "D150_VAMP_HL1_CompSeg1") == 0);

    ASSERT_TRUE(tiglWingGetComponentSegmentUID(tiglHandle, 3, 1, &uid) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(uid, "D150_VAMP_SL1_CompSeg1") == 0);
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
	ASSERT_TRUE(strcmp(wingUID, "D150_VAMP_W1") == 0);

	// test for a bug  in tigl 2.0.2, occurs when component segment does not lie on first wing
	eta = 0.16;
	xsi = 0.577506;
	ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_VAMP_HL1_CompSeg1", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	ASSERT_TRUE(ret == TIGL_SUCCESS);
	ASSERT_TRUE(strcmp(wingUID, "D150_VAMP_HL1") == 0);
}

TEST_F(WingComponentSegment, tiglWingComponentGetEtaXsi_wrongUID){
	double eta = 0.1103;
	double xsi = 0.812922;
	char * wingUID = NULL;
	char * segmentUID = NULL;
	double segmentEta = 0., segmentXsi = 0.;

	TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "invalid_comp_seg", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	ASSERT_TRUE(ret == TIGL_UID_ERROR);
}