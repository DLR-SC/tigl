/* 
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
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
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/**
* @file
* @brief Tests for testing behavior of the configuration handle management and implementation.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"


/******************************************************************************/

static TixiDocumentHandle           tixiHandle;
static TiglCPACSConfigurationHandle tiglHandle;

class tiglOpenCpacsConfiguration : public ::testing::Test {
 protected:
  virtual void SetUp() {
        char* filename = "TestData/CPACS_21_D150.xml";
        ReturnCode tixiRet;

        tiglHandle = -1;
        tixiHandle = -1;
        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE( tixiRet == SUCCESS);
  }

  virtual void TearDown() {
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
  }
};




/**
* Tests tiglOpenCPACSConfiguration with null pointer arguments.
*/
TEST_F(tiglOpenCpacsConfiguration, nullPointerArgument) 
{
	ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", NULL) == TIGL_NULL_POINTER);
}

/**
* Tests a successfull run of tiglOpenCPACSConfiguration.
*/
TEST_F(tiglOpenCpacsConfiguration, openSuccess) 
{
	ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle) == TIGL_SUCCESS);
	ASSERT_TRUE(tiglHandle > 0);
	ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
}

/**
* Tests a successfull open of tiglOpenCPACSConfiguration with specifiing the uid of the configuration.
*/
TEST_F(tiglOpenCpacsConfiguration, open_without_uid) 
{
	// Test with NULL argument
	ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, NULL, &tiglHandle) == TIGL_SUCCESS);
	ASSERT_TRUE(tiglHandle > 0);
	ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);

	// Test with empty string argument
	ASSERT_TRUE(tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle) == TIGL_SUCCESS);
	ASSERT_TRUE(tiglHandle > 0);
	ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
}


/******************************************************************************/


class TiglGetCPACSTixiHandle : public ::testing::Test {
 protected:
  virtual void SetUp() {
        char* filename = "TestData/CPACS_21_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        ASSERT_TRUE(tixiOpenDocument(filename, &tixiHandle) == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "D150_VAMP", &tiglHandle);
        ASSERT_TRUE (tiglRet == TIGL_SUCCESS );
  }

  virtual void TearDown() {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle)== SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle)== SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
  }
};


/**
* Tests tiglGetCPACSTixiHandle with null pointer arguments.
*/
TEST_F(TiglGetCPACSTixiHandle, nullPointerArgument)
{
	ASSERT_TRUE(tiglGetCPACSTixiHandle(tiglHandle, NULL) == TIGL_NULL_POINTER);
}

/**
* Tests tiglGetCPACSTixiHandle with an invalid tigl handle.
*/
TEST_F(TiglGetCPACSTixiHandle, hanlde_notFound)
{
	ASSERT_TRUE(tiglGetCPACSTixiHandle(tiglHandle + 1, &tixiHandle) == TIGL_NOT_FOUND);
}

/**
* Tests a successfull run of tiglGetCPACSTixiHandle.
*/
TEST_F(TiglGetCPACSTixiHandle, handle_success)
{
	TixiDocumentHandle tempHandle;
	ASSERT_TRUE(tiglGetCPACSTixiHandle(tiglHandle, &tempHandle) == TIGL_SUCCESS);
}

/******************************************************************************/

class tiglCPACSConfigurationHandleValid : public ::testing::Test {
 protected:
  virtual void SetUp() {
        char* filename = "TestData/CPACS_21_D150.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;
        
        ASSERT_TRUE( tixiOpenDocument(filename, &tixiHandle) == SUCCESS);

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

/**
* Tests tiglIsCPACSConfigurationHandleValid with an invalid CPACS handle.
*/
TEST_F(tiglCPACSConfigurationHandleValid, invalidHandle)
{
    TiglBoolean isValid;
    ASSERT_TRUE(tiglIsCPACSConfigurationHandleValid(-1, &isValid) == TIGL_SUCCESS);
    ASSERT_TRUE(isValid == TIGL_FALSE);
}

/**
* Tests tiglIsCPACSConfigurationHandleValid with a valid CPACS handle.
*/
TEST_F(tiglCPACSConfigurationHandleValid,validHandle)
{
    TiglBoolean isValid;
    ASSERT_TRUE(tiglIsCPACSConfigurationHandleValid (tiglHandle, &isValid) == TIGL_SUCCESS);
    ASSERT_TRUE(isValid == TIGL_TRUE);
}


/**
* Tests tiglGetVersion.
*/
TEST_F(tiglCPACSConfigurationHandleValid, version_valid)
{
    ASSERT_TRUE(!strcmp(tiglGetVersion(), TIGL_VERSION));
}

