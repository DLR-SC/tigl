/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 * Author: Malo Drougard
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
#include "CPACSWing.h"
#include "CTiglWingHelper.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"

class creatorWingHelper : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfigurationManager* manager = nullptr;
    tigl::CCPACSConfiguration* config         = nullptr;
    tigl::CTiglWingHelper wingHelper;

    void setVariables(std::string inFilename, std::string wingUID)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager = &(tigl::CCPACSConfigurationManager::GetInstance());
        config  = &(manager->GetConfiguration(tiglHandle));
        wingHelper.SetWing(&(config->GetWing(wingUID)));
    }

    void setWingHelper(std::string wingUID)
    {
        wingHelper.SetWing(&(config->GetWing(wingUID)));
    }

    void unsetVariables()
    {
        filename = "";

        if (tiglHandle > -1) {
            tiglCloseCPACSConfiguration(tiglHandle);
        }
        if (tixiHandle > -1) {
            tixiCloseDocument(tixiHandle);
        }

        tiglHandle = -1;
        tiglHandle = -1;

        manager = nullptr;
        config  = nullptr;
        wingHelper.SetWing(nullptr);
    }

    void TearDown()
    {
        unsetVariables();
    }
};

TEST_F(creatorWingHelper, MultipleWings_HasShape)
{
    tigl::CTiglWingHelper localWingHelper;
    EXPECT_FALSE(localWingHelper.HasShape());

    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_TRUE(wingHelper.HasShape());

    setVariables("TestData/multiple_wings.xml", "W13_EmptyWing");
    EXPECT_FALSE(wingHelper.HasShape());
}

TEST_F(creatorWingHelper, MultipleWings_GetCTiglElementOfWing)
{
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_EQ(wingHelper.GetCTiglElementOfWing("Cpacs2Test_Wing_Sec1_El1")->GetSectionElementUID(),
              "Cpacs2Test_Wing_Sec1_El1");
    EXPECT_EQ(wingHelper.GetCTiglElementOfWing("fsdafs"), nullptr);

    setVariables("TestData/multiple_wings.xml", "W7_SymX");
    EXPECT_EQ(wingHelper.GetCTiglElementOfWing("W7_SymX_Sec1_El1")->GetSectionElementUID(), "W7_SymX_Sec1_El1");

    setVariables("TestData/multiple_wings.xml", "W13_EmptyWing");
    EXPECT_EQ(wingHelper.GetCTiglElementOfWing("Cpacs2Test_Wing_Sec1_El1"), nullptr);
}

TEST_F(creatorWingHelper, MultipleWings_GetWingDirections)
{
    // symmetry x-z case
    setVariables("TestData/multiple_wings.xml", "Wing");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);

    setWingHelper("W2_RX90");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Z_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Y_AXIS);

    // symmetry x-z case
    setWingHelper("W3_RX40");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);

    // no symmetry case
    setWingHelper("W4_RX40b");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);

    // symmetry x-z
    // The symmetry prime on the heuristic (So, we get always the correct span)
    setWingHelper("W5_RX60");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);

    // no symmetry case
    setWingHelper("W6_RX60b");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Z_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Y_AXIS);

    // strange symmetry case (symmetry y-x plane)
    setWingHelper("W7_SymX");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Z_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Y_AXIS);

    // strange symmetry case (symmetry y-x plane)
    setWingHelper("W8_SBW");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);

    // case of a empty wing (no sections, no segments, no positionings)
    setWingHelper("W13_EmptyWing");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);

    // case of 45 angle -> most common chose primee
    setWingHelper("W14_ChaDih");
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);

    // case when the wing is not set
    // In this case, we expect to log a warning and to return the default direction
    wingHelper.SetWing(nullptr);
    EXPECT_EQ(wingHelper.GetMajorDirection(), TIGL_Y_AXIS);
    EXPECT_EQ(wingHelper.GetDeepDirection(), TIGL_X_AXIS);
    EXPECT_EQ(wingHelper.GetThirdDirection(), TIGL_Z_AXIS);


}

TEST_F(creatorWingHelper, MultipleWings_GetTipAndRootUID)
{
    // symmetry x-z case
    setVariables("TestData/multiple_wings.xml", "W2_RX90");
    EXPECT_EQ(wingHelper.GetRootUID(), "W2_RX90_Sec1_El1");
    EXPECT_EQ(wingHelper.GetTipUID(), "W2_RX90_Sec3_El1");

    // symmetry x-z case
    setWingHelper("W3_RX40");
    EXPECT_EQ(wingHelper.GetRootUID(), "W3_RX40_Sec1_El1");
    EXPECT_EQ(wingHelper.GetTipUID(), "W3_RX40_Sec3_El1");

    // no symmetry case
    setWingHelper("W6_RX60b");
    EXPECT_EQ(wingHelper.GetRootUID(), "W6_RX60b_Sec1_El1");
    EXPECT_EQ(wingHelper.GetTipUID(), "W6_RX60b_Sec3_El1");

    setWingHelper("W8_SBW");
    EXPECT_EQ(wingHelper.GetRootUID(), "W8_SBW_Sec1_El1");
    EXPECT_EQ(wingHelper.GetTipUID(), "W8_SBW_Sec2_El1");

    // case of a empty wing (no sections, no segments, no positionings)
    setWingHelper("W13_EmptyWing");
    EXPECT_EQ(wingHelper.GetRootUID(), "");
    EXPECT_EQ(wingHelper.GetTipUID(), "");

    // case when the wing is not set
    // In this case, we expect to log a warning and to return the default direction
    wingHelper.SetWing(nullptr);
    EXPECT_EQ(wingHelper.GetRootUID(), "");
    EXPECT_EQ(wingHelper.GetTipUID(), "");
}
