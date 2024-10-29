/*
* Copyright (C) 2021 German Aerospace Center (DLR/SC)
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

#include "test.h"

#include <tigletaxsifunctions.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSConfiguration.h>

TEST(EtaXsiFunctions, interpolateXsi_onSegment)
{
    TiglHandleWrapper handle("TestData/simpletest.cpacs.xml", "");
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(handle);
    const auto& uidMgr = config.GetUIDManager();
    tigl::CCPACSWing& wing = config.GetWing(1);
    const auto& compSegmentUID = wing.GetComponentSegment(1).GetUID();

    double eta = 1.;
    double xsi = 0.;
    double errorDist = 0.;
    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         "Cpacs2Test_Wing_Seg_1_2", eta, uidMgr,
                         xsi, errorDist);
    EXPECT_NEAR(0.5, xsi, 1e-6);

    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 1.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 0.0),
                         "Cpacs2Test_Wing_Seg_1_2", eta, uidMgr,
                         xsi, errorDist);
    EXPECT_NEAR(0.75, xsi, 1e-6);

    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         "Cpacs2Test_Wing_Seg_1_2", 0.0, uidMgr,
                         xsi, errorDist);
    EXPECT_NEAR(0.0, xsi, 1e-6);

    ASSERT_THROW(
        tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.1, 0.1),
                             compSegmentUID, tigl::EtaXsi(0.9, 0.1),
                             "Cpacs2Test_Wing_Seg_2_3", eta, uidMgr,
                             xsi, errorDist);
        , tigl::CTiglError);
}

TEST(EtaXsiFunctions, InterpolateOnLine_onComponentSegment)
{
    TiglHandleWrapper handle("TestData/simpletest.cpacs.xml", "");
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(handle);
    const auto& uidMgr = config.GetUIDManager();
    tigl::CCPACSWing& wing = config.GetWing(1);
    const auto& compSegmentUID = wing.GetComponentSegment(1).GetUID();

    double xsi = 0;
    double error = 0.;

    // check trivial borders for validity
    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, 0.0, uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.0, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, 1.0, uidMgr,
                         xsi, error);
    EXPECT_NEAR(1.0, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    // check cases in first segment
    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, 0.5 / (1. + sqrt(17./16.)), uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.25, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, 0.8 / (1. + sqrt(17./16.)), uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.4, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    // now check the not so trivial cases in second segment
    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, 1.0 / (1. + sqrt(17./16.)), uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.5, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, (1 + 0.5*sqrt(17./16.)) / (1. + sqrt(17./16.)), uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.5/0.75, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, (1 + 0.2*sqrt(17./16.)) / (1. + sqrt(17./16.)), uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.5/0.9, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    tigl::InterpolateXsi(compSegmentUID, tigl::EtaXsi(0.0, 0.0),
                         compSegmentUID, tigl::EtaXsi(1.0, 1.0),
                         compSegmentUID, (1 + 0.8*sqrt(17./16.)) / (1. + sqrt(17./16.)), uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.5/0.6, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);
}

TEST(EtaXsiFunctions, InterpolateOnLine_onSegmentsOnly)
{
    TiglHandleWrapper handle("TestData/simpletest.cpacs.xml", "");
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(handle);
    const auto& uidMgr = config.GetUIDManager();
    double xsi = 0;
    double error = 0.;


    tigl::InterpolateXsi("Cpacs2Test_Wing_Seg_1_2", tigl::EtaXsi(0.0, 0.0),
                         "Cpacs2Test_Wing_Seg_1_2", tigl::EtaXsi(1.0, 1.0),
                         "Cpacs2Test_Wing_Seg_1_2", 0.5, uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.5, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    tigl::InterpolateXsi("Cpacs2Test_Wing_Seg_1_2", tigl::EtaXsi(0.0, 0.0),
                         "Cpacs2Test_Wing_Seg_2_3", tigl::EtaXsi(1.0, 1.0),
                         "Cpacs2Test_Wing_Seg_1_2", 1.0, uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.5, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    tigl::InterpolateXsi("Cpacs2Test_Wing_Seg_1_2", tigl::EtaXsi(0.0, 0.0),
                         "Cpacs2Test_Wing_Seg_2_3", tigl::EtaXsi(1.0, 0.0),
                         "Cpacs2Test_Wing_Seg_1_2", 1.0, uidMgr,
                         xsi, error);
    EXPECT_NEAR(0.25, xsi, 1e-6);
    EXPECT_NEAR(0.0, error, 1e-6);

    // the eta point is outside the line, the function must throw
    ASSERT_THROW(
        tigl::InterpolateXsi("Cpacs2Test_Wing_Seg_1_2", tigl::EtaXsi(0.0, 0.0),
                             "Cpacs2Test_Wing_Seg_1_2", tigl::EtaXsi(1.0, 1.0),
                             "Cpacs2Test_Wing_Seg_2_3", 0.2, uidMgr,
                             xsi, error),
        tigl::CTiglError);
}


TEST(EtaXsiFunctions, InterpolateOnLine_onDifferentWings)
{
    TiglHandleWrapper handle("TestData/CPACS_30_D150.xml", "");
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(handle);
    const auto& uidMgr = config.GetUIDManager();
    double xsi = 0;
    double error = 0.;

    // the eta point is outside the line, the function must throw
    ASSERT_THROW(
        tigl::InterpolateXsi("D150_VAMP_W1_Seg1", tigl::EtaXsi(0.0, 0.0),
                             "D150_VAMP_W1_Seg1", tigl::EtaXsi(1.0, 1.0),
                             "D150_VAMP_HL1_Seg1", 0.2, uidMgr,
                             xsi, error),
        tigl::CTiglError);
}
