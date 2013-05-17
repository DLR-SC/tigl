/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief Tests some fuselage fuctions like circumreference, volumen, etc...
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"

#include <cmath>

/******************************************************************************/

class TiglFuselageHelper : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
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


TixiDocumentHandle TiglFuselageHelper::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglFuselageHelper::tiglHandle = 0;

//#########################################################################################################

/**
* Tests tiglFuselageGetCircumference with success
*/
TEST_F(TiglFuselageHelper, tiglFuselageGetCircumference_success)
{
    double circumferencePtr;
    ASSERT_TRUE(tiglFuselageGetCircumference(tiglHandle, 1, 1, 0.5, &circumferencePtr) == TIGL_SUCCESS);
    // the profile is not really a circle, thats why we allow a large deviation of 3%l
    ASSERT_NEAR(circumferencePtr, M_PI, M_PI*0.03);
}


/**
* Tests tiglFuselageGetCircumference with null pointer to circumferencePtr
*/
TEST_F(TiglFuselageHelper, tiglFuselageGetProfileName_nullPointer)
{
    ASSERT_TRUE(tiglFuselageGetCircumference(tiglHandle, 1, 1, 0.5, NULL) == TIGL_NULL_POINTER);
}



/**
* Tests tiglFuselageGetProfileName with success
*/
TEST_F(TiglFuselageHelper, tiglFuselageGetProfileName_success)
{
    char* profileName; 
    ASSERT_TRUE(tiglFuselageGetProfileName(tiglHandle, 1, 1, 1, &profileName) == TIGL_SUCCESS);
    ASSERT_STREQ(profileName, "Circle");
}


/**
* Tests tiglFuselageGetUID with success
*/
TEST_F(TiglFuselageHelper, tiglFuselageGetUID_success)
{
    char* uid; 
    ASSERT_TRUE(tiglFuselageGetUID(tiglHandle, 1, &uid) == TIGL_SUCCESS);
    ASSERT_STREQ(uid, "SimpleFuselage");
}

TEST_F(TiglFuselageHelper, tiglFuselageGetVolume){
    double volume = 0.;
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetVolume(tiglHandle, 1, &volume));
    double theoVol = 0.5*0.5*M_PI * 2.; // only approximation
    ASSERT_NEAR(theoVol, volume, theoVol*0.05);
}


