/* 
* Copyright (C) 1018 German Aerospace Center (DLR/SC)
*
* Created: 2018-01-11 Martin Siggel <martin.siggel@dlr.de>
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

#include "test.h" // Brings in the GTest framework
#include "tigl.h"
#include <string.h>


/******************************************************************************/

/**
 * @brief This tests checks, whether the cpacs transformation properties (absglobal, abslocal)
 * are interpreted correctly.
 */
class TiglTransformationBenchmark : public ::testing::Test 
{
protected:
    static void SetUpTestCase() 
    {
        const char* filename = "TestData/transformation-benchmark.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase() 
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() OVERRIDE {}
    void TearDown() OVERRIDE {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle  TiglTransformationBenchmark::tixiHandle = 0;
TiglCPACSConfigurationHandle  TiglTransformationBenchmark::tiglHandle = 0;

TEST_F(TiglTransformationBenchmark, checkProfilePositions)
{
    double px, py, pz;

    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 1, 1, 0., 0., &px, &py, &pz));
    EXPECT_NEAR(1.0, px, 1e-10);
    EXPECT_NEAR(0.0, py, 1e-10);
    EXPECT_NEAR(0.0, pz, 1e-10);

    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 2, 1, 0., 0., &px, &py, &pz));
    EXPECT_NEAR(1.0, px, 1e-10);
    EXPECT_NEAR(1.0, py, 1e-10);
    EXPECT_NEAR(0.0, pz, 1e-10);

    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 3, 1, 0., 0., &px, &py, &pz));
    EXPECT_NEAR(1.0, px, 1e-10);
    EXPECT_NEAR(2.0, py, 1e-10);
    EXPECT_NEAR(0.0, pz, 1e-10);

    ASSERT_EQ(TIGL_SUCCESS, tiglWingGetChordPoint(tiglHandle, 4, 1, 0., 0., &px, &py, &pz));
    EXPECT_NEAR(1.0, px, 1e-10);
    EXPECT_NEAR(3.0, py, 1e-10);
    EXPECT_NEAR(0.0, pz, 1e-10);
}
