/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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
#include "CCPACSConfiguration.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CTiglFuselageSectionElement.h"

#include <string.h>

class creatorFuselageHelper : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfigurationManager* manager = nullptr;
    tigl::CCPACSConfiguration* config         = nullptr;
    tigl::CCPACSFuselage* fuselage            = nullptr;
    tigl::CTiglFuselageHelper* fuselageHelper = nullptr;

    void setVariables(std::string inFilename)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager = &(tigl::CCPACSConfigurationManager::GetInstance());
        config  = &(manager->GetConfiguration(tiglHandle));
    }

    // set the fusleage and it helper classe
    void setFuselage(std::string fuselageUID)
    {
        tigl::CCPACSFuselages& fuselages = config->GetFuselages();
        fuselage                         = &(fuselages.GetFuselage(fuselageUID));
        fuselageHelper                   = new tigl::CTiglFuselageHelper(fuselage);
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

        manager  = nullptr;
        config   = nullptr;
        fuselage = nullptr;
        delete fuselageHelper;
        fuselageHelper = nullptr;
    }

    void TearDown()
    {
        unsetVariables();
    }

};

TEST_F(creatorFuselageHelper, getNoiseUID_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml");

    setFuselage("FuselageUnconventionalOrdering");
    // simple reordering ( if the segments are swapped ) is supported
    std::string uid = fuselageHelper->GetNoseUID();
    ASSERT_EQ("D150_Fuselage_2Section1IDElement1", uid);

    // Todo: "complex" reordering when FromUID and toUID are also swapped is not supported, may not be a priority
    //setVariables("TestData/multiple_fuselages.xml", "FuselageUnconventionalOrderingExtrem");

    //uid = fuselage->GetNoseUID();
    //ASSERT_EQ("D150_Fuselage_2aSection1IDElement1", uid);
}

TEST_F(creatorFuselageHelper, getTailUID_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");
    setFuselage("FuselageUnconventionalOrdering");
    std::string uid = fuselageHelper->GetTailUID();
    ASSERT_EQ("D150_Fuselage_2Section3IDElement1", uid);
}


TEST_F(creatorFuselageHelper, getElementUIDsInOrder_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    setFuselage("SimpleFuselage");

    std::vector<std::string> elementUIDs = fuselageHelper->GetElementUIDsInOrder();
    std::vector<std::string> expectedElementUIDs;
    expectedElementUIDs.push_back("D150_Fuselage_1Section1IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_1Section2IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_1Section3IDElement1");

    EXPECT_EQ(expectedElementUIDs.size(), elementUIDs.size());

    for (int i = 0; i < expectedElementUIDs.size(); i++) {
        EXPECT_EQ(expectedElementUIDs.at(i), elementUIDs.at(i));
    }

    setFuselage("FuselageUnconventionalOrdering");
    elementUIDs = fuselageHelper->GetElementUIDsInOrder();
    expectedElementUIDs.clear();
    expectedElementUIDs.push_back("D150_Fuselage_2Section1IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_2Section2IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_2Section3IDElement1");

    EXPECT_EQ(expectedElementUIDs.size(), elementUIDs.size());

    for (int i = 0; i < expectedElementUIDs.size(); i++) {
        EXPECT_EQ(expectedElementUIDs.at(i), elementUIDs.at(i));
    }
}



TEST_F(creatorFuselageHelper, getCTiglElementOfFuselage_MultipleFuselagesModel)
{

    tigl::CTiglFuselageSectionElement* cElement;

    setVariables("TestData/multiple_fuselages.xml");
    setFuselage("SimpleFuselage");

    cElement = fuselageHelper->GetCTiglElementOfFuselage("D150_Fuselage_1Section1IDElement1");
    EXPECT_EQ(cElement->GetSectionElementUID() ,  "D150_Fuselage_1Section1IDElement1");

    cElement = fuselageHelper->GetCTiglElementOfFuselage("fasdfasdghfdagfa");
    EXPECT_EQ(cElement, nullptr);

}
