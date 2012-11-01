/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
* @brief Tests for testing non classified wing functions.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include <string.h>


/******************************************************************************/

class TiglWing : public ::testing::Test {
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


TixiDocumentHandle TiglWing::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglWing::tiglHandle = 0;

/******************************************************************************/

/**
* Tests tiglWingGetProfileName with invalid CPACS handle.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidHandle)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(-1, 1, 1, 1, &namePtr) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetProfileName with invalid wing index.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidWing)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, -1, 1, 1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with invalid section index.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidSection)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, -1, 1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with invalid element index.
*/
TEST_F(TiglWing, tiglWingGetProfileName_invalidElement)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, 1, -1, &namePtr) == TIGL_INDEX_ERROR);
}

/**
* Tests tiglWingGetProfileName with null pointer argument.
*/
TEST_F(TiglWing, tiglWingGetProfileName_nullPointerArgument)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, 1, 1, 0) == TIGL_NULL_POINTER);
}

/**
* Tests tiglWingGetProfileName with null pointer argument.
*/
TEST_F(TiglWing, tiglWingGetProfileName_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetProfileName(tiglHandle, 1, 1, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(namePtr, "NameD150_VAMP_W_SupCritProf1") == 0);
}


/**
* Tests tiglWingGetUID 
*/
TEST_F(TiglWing, tiglWingGetUID_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetUID(tiglHandle, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(namePtr, "D150_VAMP_W1") == 0);
}


/**
* Tests tiglWingGetSegmentUID 
*/
TEST_F(TiglWing, tiglWingGetSegmentUID_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetSegmentUID(tiglHandle, 1, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(namePtr, "D150_VAMP_W1_Seg1") == 0);
}


/**
* Tests tiglWingGetSectionUID 
*/
TEST_F(TiglWing, tiglWingGetSectionUID_success)
{
    char* namePtr = 0;
    ASSERT_TRUE(tiglWingGetSectionUID(tiglHandle, 1, 1, &namePtr) == TIGL_SUCCESS);
    ASSERT_TRUE(strcmp(namePtr, "D150_VAMP_W1_Sec1") == 0);
}

/**
* Tests for tiglWingGetIndex
*/
TEST_F(TiglWing, tiglWingGetIndex_success){
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "D150_VAMP_W1", &wingIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(wingIndex == 1);

    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "D150_VAMP_HL1", &wingIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(wingIndex == 2);

    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "D150_VAMP_SL1", &wingIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(wingIndex == 3);
}

TEST_F(TiglWing, tiglWingGetIndex_wrongUID){
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, "invalid_uid", &wingIndex) == TIGL_UID_ERROR);
    ASSERT_TRUE(wingIndex == -1);
}

TEST_F(TiglWing, tiglWingGetIndex_nullPtr){
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(tiglHandle, NULL , &wingIndex) == TIGL_NULL_POINTER);
}

TEST_F(TiglWing, tiglWingGetIndex_wrongHandle){
    TiglCPACSConfigurationHandle myWrongHandle = -1234;
    int wingIndex = 0;
    ASSERT_TRUE(tiglWingGetIndex(myWrongHandle, "D150_VAMP_W1_Seg1" , &wingIndex) == TIGL_NOT_FOUND);
}

/**
* Tests tiglWingGetSegmentIndex
*/
TEST_F(TiglWing, tiglWingGetSegmentIndex_success){
    int segmentIndex = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, 1, "D150_VAMP_W1_Seg1", &segmentIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentIndex == 1);

    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, 1, "D150_VAMP_W1_Seg2", &segmentIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentIndex == 2);

    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, 1, "D150_VAMP_W1_Seg3", &segmentIndex) == TIGL_SUCCESS);
    ASSERT_TRUE(segmentIndex == 3);
}

TEST_F(TiglWing, tiglWingGetSegmentIndex_wrongUID){
    int segmentIndex = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, 1, "invalid_seg_name", &segmentIndex) == TIGL_UID_ERROR);
}

TEST_F(TiglWing, tiglWingGetSegmentIndex_nullPtr){
    int segmentIndex = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, 1, NULL, &segmentIndex) == TIGL_NULL_POINTER);
}

TEST_F(TiglWing, tiglWingGetSegmentIndex_wrongWing){
    int segmentIndex = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(tiglHandle, -1, "D150_VAMP_W1_Seg1", &segmentIndex) == TIGL_INDEX_ERROR);
}

TEST_F(TiglWing, tiglWingGetSegmentIndex_wrongHandle){
    TiglCPACSConfigurationHandle myWrongHandle = -1234;
    int segmentIndex = 0;
    ASSERT_TRUE(tiglWingGetSegmentIndex(myWrongHandle, -1, "D150_VAMP_W1_Seg1", &segmentIndex) == TIGL_NOT_FOUND);
}

