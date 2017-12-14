 /* 
* Copyright (C) 2016 Airbus Defence & Space
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
#include "tigl.h"

#include "CCPACSConfigurationManager.h"
#include "CCPACSWing.h"
#include "CCPACSWingCell.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingRibsPositioning.h"
#include "CCPACSWingSegment.h"

using namespace tigl;

typedef std::pair<double, double> DP;

/******************************************************************************/

class WingCellRibSpar2: public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/cell_rib_spar_test.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ(SUCCESS, tixiRet);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "model", &tiglHandle);
        ASSERT_EQ(TIGL_SUCCESS, tiglRet);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
        tiglHandle = -1;
        tixiHandle = -1;
    }

    virtual void SetUp() {}
    virtual void TearDown() {}

    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle WingCellRibSpar2::tixiHandle = 0;
TiglCPACSConfigurationHandle WingCellRibSpar2::tiglHandle = 0;

class DistortedWing : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/distorted_wing.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ(SUCCESS, tixiRet);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "model", &tiglHandle);
        ASSERT_EQ(TIGL_SUCCESS, tiglRet);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(TIGL_SUCCESS, tiglCloseCPACSConfiguration(tiglHandle));
        ASSERT_EQ(SUCCESS, tixiCloseDocument(tixiHandle));
        tiglHandle = -1;
        tixiHandle = -1;
    }

    virtual void SetUp() {}
    virtual void TearDown() {}

    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
};

TixiDocumentHandle DistortedWing::tixiHandle = 0;
TiglCPACSConfigurationHandle DistortedWing::tiglHandle = 0;


/******************************************************************************/

TEST_F(WingCellRibSpar2, computeSparXsi) {
    // get Component Segment
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    const tigl::CCPACSWingSparSegment& spar = componentSegment.GetStructure()->GetSparSegment(1);

    const double arr[] = { 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.41, 0.42, 0.43, 0.44, 0.45, 0.46, 0.47, 0.48, 0.49, 0.5, 0.4, 0.3 };
    std::vector<double> expectedXsis (arr, arr + sizeof(arr) / sizeof(arr[0]));
    for (size_t i = 0; i < expectedXsis.size(); ++i) {
        // minus one since we have 21 points, but i goes from 0 to 20
        double eta = (double)i / (expectedXsis.size() - 1);
        double expect = expectedXsis.at(i);
        double xsi = tigl::computeSparXsiValue(componentSegment, spar, eta);
        ASSERT_NEAR(xsi, expect, 1e-7);
    }
}

TEST_F(DistortedWing, computeSparXsi) {
    // get Component Segment
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    const tigl::CCPACSWingSparSegment& spar = componentSegment.GetStructure()->GetSparSegment(1);

    const std::pair<double, double> arr[] = { DP(0, 0.2), DP(0.33, 0.29), DP(0.46953191, 0.2), DP(0.9, 0.33) };
    std::vector< std::pair<double, double> > expectedEtaXsis (arr, arr + sizeof(arr) / sizeof(arr[0])); 
    // low precision because expected xsi values are measured from model
    const double precision = 1E-2;
    std::vector< std::pair<double, double> >::const_iterator it;
    for (it = expectedEtaXsis.begin(); it != expectedEtaXsis.end(); ++it) {
        double eta = it->first;
        double expect = it->second;
        double xsi = tigl::computeSparXsiValue(componentSegment, spar, eta);
        ASSERT_NEAR(xsi, expect, precision);
    }
}

TEST_F(WingCellRibSpar2, computeRibEta) {
    // get Component Segment
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::CCPACSWingCSStructure& structure = *componentSegment.GetStructure();
    tigl::CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(1);
    // now we change the rib definition for testing
    // TODO: create a cpacs file which already contains this modified rib definition
    ribsDefinition.GetRibsPositioning_choice1()->SetEtaEnd(0.8);
    ribsDefinition.GetRibsPositioning_choice1()->GetRibRotation().SetZ(75);

    // measured from geometry
    const std::pair<double, double> arr[] = { DP(0.3, 0.159808), DP(0.4, 0.14641), DP(0.5, 0.133013), DP(0.6, 0.119615), DP(0.7, 0.106218), DP(0.8, 0.092820), DP(0.9, 0.079423), DP(1.0, 0.066025) };
    std::vector< std::pair<double, double> > expectedEtas (arr, arr + sizeof(arr) / sizeof(arr[0]));

    std::vector< std::pair<double, double> >::const_iterator it;
    for (it = expectedEtas.begin(); it != expectedEtas.end(); ++it) {
        double xsi = it->first;
        double expectedEta = it->second;
        double eta = tigl::computeRibEtaValue(componentSegment, ribsDefinition, 1, xsi);
        ASSERT_NEAR(eta, expectedEta, 1e-6);
    }
}

TEST_F(WingCellRibSpar2, computeSparIntersectionEtaXsi) {
    // get Component Segment
    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(1));
    tigl::CCPACSWingCSStructure& structure = *componentSegment.GetStructure();
    tigl::CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(1);
    const tigl::CCPACSWingSparSegment& spar = structure.GetSparSegment(1);
    // now we change the rib definition for testing
    // TODO: create a cpacs file which already contains this modified rib definition
    ribsDefinition.GetRibsPositioning_choice1()->SetEtaEnd(0.8);
    ribsDefinition.GetRibsPositioning_choice1()->GetRibRotation().SetZ(75);

    // measured from geometry
    const tigl::EtaXsi arr[] = { tigl::EtaXsi(0.162331, 0.281166), tigl::EtaXsi(0.737370, 0.467474) };
    std::vector<tigl::EtaXsi> expectedEtaXsis (arr, arr + sizeof(arr) / sizeof(arr[0]));

    int ribIndex = 1;
    std::vector<tigl::EtaXsi>::const_iterator it;
    for (it = expectedEtaXsis.begin(); it != expectedEtaXsis.end(); ++it) {
        tigl::EtaXsi got = tigl::computeRibSparIntersectionEtaXsi(componentSegment, ribsDefinition, ribIndex, spar);
        ASSERT_NEAR(got.eta, it->eta, 1e-6);
        ASSERT_NEAR(got.xsi, it->xsi, 1e-6);
        ribIndex++;
    }
}
