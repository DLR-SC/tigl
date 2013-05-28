/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
#include "CCPACSMaterial.h"

/******************************************************************************/

class WingComponentSegment : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
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

  static void TearDownTestCase() {
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

class WingComponentSegment2 : public ::testing::Test {
 protected:
  virtual void SetUp() {
        const char* filename = "TestData/CPACS_20_D250_10.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ (SUCCESS, tixiRet);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D250_VAMP", &tiglHandle);
        ASSERT_EQ(TIGL_SUCCESS, tiglRet);
  }

  virtual void TearDown() {
        ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
        tiglHandle = -1;
        tixiHandle = -1;
  }

  TixiDocumentHandle           tixiHandle;
  TiglCPACSConfigurationHandle tiglHandle;
};


class WingComponentSegment3 : public ::testing::Test {
 protected:
  virtual void SetUp() {
        const char* filename = "TestData/D150_v201.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ (SUCCESS, tixiRet);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150modelID", &tiglHandle);
        ASSERT_EQ(TIGL_SUCCESS, tiglRet);
  }

  virtual void TearDown() {
        ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
        tiglHandle = -1;
        tixiHandle = -1;
  }

  TixiDocumentHandle           tixiHandle;
  TiglCPACSConfigurationHandle tiglHandle;
};


TixiDocumentHandle WingComponentSegment::tixiHandle = 0;
TiglCPACSConfigurationHandle WingComponentSegment::tiglHandle = 0;

/******************************************************************************/


class WingComponentSegmentSimple : public ::testing::Test {
 protected:
  virtual void SetUp() {
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

  virtual void TearDown() {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
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


TEST_F(WingComponentSegment2, tiglWingComponentGetEtaXsi_success)
{
	double eta = 0.3336;
	double xsi = 0.;
	char * wingUID = NULL;
	char * segmentUID = NULL;
	double segmentEta = 0., segmentXsi = 0.;

	TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D250_wing_CS", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	ASSERT_EQ(TIGL_SUCCESS, ret);
	ASSERT_STREQ("D250_wing", wingUID);
	free(wingUID); wingUID = NULL;
	free(segmentUID); segmentUID = NULL;

	eta = 0.;
	xsi = 0.5;

	ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D250_wing_CS", eta, xsi, &wingUID, &segmentUID, &segmentEta, &segmentXsi);
	ASSERT_EQ(TIGL_SUCCESS, ret);
	ASSERT_STREQ("D250_wing", wingUID);
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

TEST_F(WingComponentSegmentSimple, tiglWingComponentSegmentGetPoint_success){
    double accuracy = 1e-7;
    double x, y, z;
    ASSERT_EQ(TIGL_SUCCESS, tiglWingComponentSegmentGetPoint(tiglHandle, "WING_CS1", 0.5, 0.0, &x, &y, &z));
    ASSERT_NEAR(0.0, x, accuracy);
    ASSERT_NEAR(1.0, y, accuracy);
    ASSERT_NEAR(0.0, z, accuracy);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingComponentSegmentGetPoint(tiglHandle, "WING_CS1", 0.75, 0.0, &x, &y, &z));
    ASSERT_NEAR(0.25, x, accuracy);
    ASSERT_NEAR(1.5, y, accuracy);
    ASSERT_NEAR(0.0, z, accuracy);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglWingComponentSegmentGetPoint(tiglHandle, "WING_CS1", 1.0, 0.0, &x, &y, &z));
    ASSERT_NEAR(0.5, x, accuracy);
    ASSERT_NEAR(2.0, y, accuracy);
    ASSERT_NEAR(0.0, z, accuracy);
}

TEST_F(WingComponentSegmentSimple, GetMaterials){
    int compseg = 1;
    // now we have do use the internal interface as we currently have no public api for this
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(compseg);
    
    // test point in cell
    tigl::MaterialList list = segment.GetMaterials(0.25, 0.9, tigl::UPPER_SHELL);
    ASSERT_EQ(2, list.size());
    ASSERT_STREQ("MyCellMat", list[0]->GetUID().c_str());
    ASSERT_STREQ("MySkinMat", list[1]->GetUID().c_str());
    
    // test point outside cell
    list = segment.GetMaterials(0.6, 0.9, tigl::UPPER_SHELL);
    ASSERT_EQ(1, list.size());
    ASSERT_STREQ("MySkinMat", list[0]->GetUID().c_str());
    
    // no materials defined for lower shell
    list = segment.GetMaterials(0.6, 0.9, tigl::LOWER_SHELL);
    ASSERT_EQ(0, list.size());
}

TEST_F(WingComponentSegmentSimple, determine_segments){
    int compseg = 1;
    // now we have do use the internal interface as we currently have no public api for this
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(compseg);
    
    std::vector<int> list = segment.GetSegmentList(segment.GetFromElementUID(), segment.GetToElementUID());
    ASSERT_EQ(2, list.size());
    ASSERT_EQ(1, list.at(0));
    ASSERT_EQ(2, list.at(1));
    
    list = segment.GetSegmentList(segment.GetToElementUID(), segment.GetFromElementUID());
    ASSERT_EQ(2, list.size());
    ASSERT_EQ(1, list.at(0));
    ASSERT_EQ(2, list.at(1));
    
    list = segment.GetSegmentList("Cpacs2Test_Wing_Sec1_El1", "Cpacs2Test_Wing_Sec2_El1");
    ASSERT_EQ(1, list.size());
    ASSERT_EQ(1, list.at(0));
    
    list = segment.GetSegmentList("Cpacs2Test_Wing_Sec2_El1", "Cpacs2Test_Wing_Sec1_El1");
    ASSERT_EQ(1, list.size());
    ASSERT_EQ(1, list.at(0));
    
    list = segment.GetSegmentList("Cpacs2Test_Wing_Sec2_El1", "Cpacs2Test_Wing_Sec3_El1");
    ASSERT_EQ(1, list.size());
    ASSERT_EQ(2, list.at(0));
    
}

TEST_F(WingComponentSegmentSimple, determine_segments_invalidUids){
    int compseg = 1;
    // now we have do use the internal interface as we currently have no public api for this
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(compseg);
    
    std::vector<int> list = segment.GetSegmentList("my_fake_UID", segment.GetFromElementUID());
    ASSERT_EQ(0, list.size());
}

TEST_F(WingComponentSegment3, tiglWingComponentSegmentPointGetSegmentEtaXsi_BUG1){
    // now the tests
    double sEta = 0., sXsi = 0.;
    char *wingUID = NULL, *segmentUID = NULL;
    TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_wing_CS", 0.0, 0.0, &wingUID, &segmentUID, &sEta, &sXsi);
    ASSERT_EQ(TIGL_SUCCESS, ret);
    ASSERT_STREQ("D150_wing_1ID", wingUID);
    ASSERT_STREQ("D150_wing_1Segment2ID", segmentUID);
    cout << "eta_s / xsi_s: " << sEta << "/" << sXsi << endl;
}

TEST(WingComponentSegment4, tiglWingComponentSegmentPointGetSegmentEtaXsi_BUG2){
    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle = -1;
    
    const char* filename = "TestData/simple_rectangle_compseg.xml";
    
    ReturnCode tixiRet = tixiOpenDocument(filename, &tixiHandle);
    ASSERT_TRUE (tixiRet == SUCCESS);
    TiglReturnCode tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150modelID", &tiglHandle);
    ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    
    double sEta = 0., sXsi = 0.;
    char *wingUID = NULL, *segmentUID = NULL;
    tiglRet = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_wing_CS", 0.5, 0.10142, &wingUID, &segmentUID, &sEta, &sXsi);
    ASSERT_EQ(TIGL_SUCCESS, tiglRet);
    ASSERT_STREQ("D150_wing_1Segment3ID", segmentUID);
    ASSERT_NEAR(0.5, sEta, 0.0001);
    
    
    tiglCloseCPACSConfiguration(tiglHandle);
    tixiCloseDocument(tixiHandle);
}

TEST_F(WingComponentSegment3, DISABLED_tiglWingComponentSegmentPointGetSegmentEtaXsi_BUG3){
    double sEta = 0., sXsi = 0.;
    char *wingUID = NULL, *segmentUID = NULL;
    TiglReturnCode ret = tiglWingComponentSegmentPointGetSegmentEtaXsi(tiglHandle, "D150_VTP_CS", 0.0, 1.0, &wingUID, &segmentUID, &sEta, &sXsi);
    ASSERT_EQ(TIGL_SUCCESS, ret);
    //ASSERT_STREQ("D150_wing_1ID", wingUID);
    //ASSERT_STREQ("D150_wing_1Segment2ID", segmentUID);
    cout << "eta_s / xsi_s: " << sEta << "/" << sXsi << endl;
}

