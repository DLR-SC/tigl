/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-12-23 Martin Siggel <Martin.Siggel@dlr.de>
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
#include "CTiglPoint.h"
#include "CTiglPointTranslator.h"

#include <string.h>
#include <ctime>


/******************************************************************************/

class TestPerformance : public ::testing::Test
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

TixiDocumentHandle TestPerformance::tixiHandle = 0;
TiglCPACSConfigurationHandle TestPerformance::tiglHandle = 0;

TEST_F(TestPerformance, wingGetPoint)
{
    int nruns = 500;
    double number = 0.;

    clock_t start = clock();

    for (int irun = 0; irun < nruns; ++irun) {
        int segIndex = rand() % 3 + 1;
        double px, py, pz;
        double eta = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
        double xsi = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);

        ASSERT_EQ(TIGL_SUCCESS, tiglWingGetUpperPoint(tiglHandle, 1, segIndex, eta, xsi, &px, &py, &pz));
        ASSERT_EQ(TIGL_SUCCESS, tiglWingGetLowerPoint(tiglHandle, 1, segIndex, eta, xsi, &px, &py, &pz));

        // prevent compiler optimization
        number += 1.0;
    }

    clock_t stop = clock();
    if (fabs(number - nruns) < 1e-10) {
        double time_elapsed = (double)(stop - start)/(double)CLOCKS_PER_SEC/(2.*number) * 1000000.;
        std::cout << "Time wingGetPoint [us]: " << time_elapsed << std::endl;
    }
}

TEST_F(TestPerformance, fuselageGetPoint)
{
    int nruns = 100;
    double number = 0.;

    int nsegs = 0;
    ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetSegmentCount(tiglHandle, 1, &nsegs));

    clock_t start = clock();

    for (int irun = 0; irun < nruns; ++irun) {
        int segIndex = rand() % nsegs + 1;
        double px, py, pz;
        double eta = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
        double xsi = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);

        ASSERT_EQ(TIGL_SUCCESS, tiglFuselageGetPoint(tiglHandle, 1, segIndex, eta, xsi, &px, &py, &pz));

        // prevent compiler optimization
        number += 1.0;
    }

    clock_t stop = clock();
    if (fabs(number - nruns) < 1e-10) {
        double time_elapsed = (double)(stop - start)/(double)CLOCKS_PER_SEC/number * 1000000.;
        std::cout << "Time fuselageGetPoint [us]: " << time_elapsed << std::endl;
    }
}

TEST_F(TestPerformance, pointTranslator)
{
    int nruns = 10000;
    double abs_error = 1e-6;

    tigl::CTiglPoint x1(0,4,0);
    tigl::CTiglPoint x2(8,4,0);
    tigl::CTiglPoint x3(0,0,0);
    tigl::CTiglPoint x4(4,0,0);

    tigl::CTiglPoint p(3,2,1);

    double eta, xsi;
    double x = 0.;

    tigl::CTiglPointTranslator trans(x1, x2, x3, x4 );

    clock_t start = clock();

    for(int i = 0; i < nruns; ++i){
        trans.translate(p, &eta, &xsi) ;
        //just some dummy to prevent compiler optimization
        x = x + 1.0;
    }

    clock_t stop = clock();

    ASSERT_EQ((double)nruns, x);
    ASSERT_NEAR(0.5, eta, abs_error);
    ASSERT_NEAR(0.5, xsi, abs_error);

    double time_elapsed = (double)(stop - start)/(double)CLOCKS_PER_SEC/(double)nruns * 1000000.;
    std::cout << "Time PointTranslator [us]: " << time_elapsed << std::endl;

    ASSERT_TRUE(true);
}
