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
* @brief Tests for testing non classified wing functions.
*/

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include <string.h>


/******************************************************************************/

class TiglWingSymmetry : public ::testing::Test 
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


TixiDocumentHandle TiglWingSymmetry::tixiHandle = 0;
TiglCPACSConfigurationHandle TiglWingSymmetry::tiglHandle = 0;

/******************************************************************************/


/**
* Tests tiglWingGetSymmetry_invalidHandle .
*/
TEST_F(TiglWingSymmetry, tiglWingGetSymmetry_success)
{
    TiglSymmetryAxis axis;
    ASSERT_TRUE(tiglWingGetSymmetry(tiglHandle, 1, &axis) == TIGL_SUCCESS);
    ASSERT_TRUE(TIGL_X_Z_PLANE == axis) << "Symmetry not correct. Was looking for TIGL_X_Z_PLANE";
}


/**
* Tests tiglWingGetSymmetry_invalidHandle .
*/
TEST_F(TiglWingSymmetry, tiglWingGetSymmetry_invalidIndex)
{
    TiglSymmetryAxis axis;
    ASSERT_TRUE(tiglWingGetSymmetry(tiglHandle, -1, &axis) == TIGL_INDEX_ERROR);
}

