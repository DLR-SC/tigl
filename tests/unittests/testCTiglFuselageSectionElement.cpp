/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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
#include <tixicpp.h>

#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSFuselageSectionElement.h"
#include "CTiglFuselageSectionElement.h"

#include <iostream>
#include <fstream>

#include <string.h>

class testCTiglFuselageSectionElement : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfiguration* config = nullptr;
    tigl::CTiglUIDManager* uidManager = nullptr;

    void setVariables(std::string inFilename)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        tigl::CCPACSConfigurationManager* manager = &(tigl::CCPACSConfigurationManager::GetInstance());
        config                                    = &(manager->GetConfiguration(tiglHandle));
        uidManager                                = &(config->GetUIDManager());
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

        config     = nullptr;
        uidManager = nullptr;
    }

    void saveCurrentConfig(std::string outFilename)
    {
        // Save the result in a new file (For visual check purpose)
        // write the change in tixi memory
        config->WriteCPACS(config->GetUID());
        std::string newConfig = tixi::TixiExportDocumentAsString(tixiHandle);
        // Import-export to flat the xml // todo inform tixi developers about this "bug
        TixiDocumentHandle tixiHandle2 = tixi::TixiImportFromString(newConfig);
        newConfig                      = tixi::TixiExportDocumentAsString(tixiHandle2);
        // save the content into the output file
        std::ofstream myfile;
        myfile.open(outFilename, std::ios::trunc);
        myfile << newConfig;
        myfile.close();
        tixiCloseDocument(tixiHandle2);
    }

    void TearDown()
    {
        unsetVariables();
    }
};

TEST_F(testCTiglFuselageSectionElement, simpleModel)
{
    setVariables("TestData/simpletest.cpacs.xml");

    tigl::CTiglFuselageSectionElement* ctiglElement = nullptr;

    tigl::CCPACSFuselageSectionElement& element =
        uidManager->ResolveObject<tigl::CCPACSFuselageSectionElement>("D150_Fuselage_1Section1IDElement1");
    ctiglElement = element.GetCTiglSectionElement();

    ASSERT_NE(nullptr, ctiglElement);

    // Test Get functions

    EXPECT_TRUE(tigl::CTiglPoint(-0.5, 0, 0).isNear(ctiglElement->GetOrigin()));

    EXPECT_NEAR(M_PI * 2 * 0.5, ctiglElement->GetCircumferenceOfProfile(), 0.1);

    EXPECT_NEAR(M_PI * 2, ctiglElement->GetCircumferenceOfProfile(FUSELAGE_COORDINATE_SYSTEM), 0.1);

    EXPECT_TRUE(tigl::CTiglPoint(-0.5, 0, 0).isNear(ctiglElement->GetCenter()));

    // Test Set functions

    tigl::CTiglPoint newOrigin, newCenter;

    newOrigin = tigl::CTiglPoint(2, 1.1, 0.13);
    ctiglElement->SetOrigin(newOrigin);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin()));

    newOrigin = tigl::CTiglPoint(-3, 1, 2);
    ctiglElement->SetOrigin(newOrigin);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin()));
    // check the value of the element transformation
    // remark that to compute the element translation we must take in account the position and the scaling that affect this transformation
    tigl::CTiglPoint elementTranslation = element.GetTranslation();
    EXPECT_TRUE(tigl::CTiglPoint(-2.5, 2, 4).isNear(elementTranslation));

    newOrigin = tigl::CTiglPoint(-3, 1, 2);
    ctiglElement->SetOrigin(newOrigin, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(-3, 0.5, 1).isNear(ctiglElement->GetOrigin()));

    newOrigin = tigl::CTiglPoint(-3, 1, 2);
    ctiglElement->SetOrigin(newOrigin, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(-3, 0.5, 1).isNear(ctiglElement->GetOrigin()));

    newCenter = tigl::CTiglPoint(-5, 1, 2);
    ctiglElement->SetCenter(newCenter, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newCenter.isNear(ctiglElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(-5, 0.5, 1).isNear(ctiglElement->GetCenter()));

    // save the file for visual check
    saveCurrentConfig("TestData/Output/simpletest-out.xml");
}

TEST_F(testCTiglFuselageSectionElement, simpleModelShifftedProfiles)
{
    setVariables("TestData/simpletest-shifftedProfiles.xml");

    tigl::CTiglFuselageSectionElement* ctiglElement = nullptr;

    tigl::CCPACSFuselageSectionElement& element =
        uidManager->ResolveObject<tigl::CCPACSFuselageSectionElement>("D150_Fuselage_1Section2IDElement1");
    ctiglElement = element.GetCTiglSectionElement();

    ASSERT_NE(nullptr, ctiglElement);

    // Test Get functions ( In this case center and origin differs)

    EXPECT_TRUE(tigl::CTiglPoint(0.5, 0, 0).isNear(ctiglElement->GetOrigin()));

    EXPECT_TRUE(tigl::CTiglPoint(0.5, 0, 0).isNear(ctiglElement->GetOrigin(FUSELAGE_COORDINATE_SYSTEM)));

    EXPECT_TRUE(tigl::CTiglPoint(0.5, 0.5, 0).isNear(ctiglElement->GetCenter()));

    EXPECT_TRUE(tigl::CTiglPoint(0.5, 1, 0).isNear(ctiglElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM)));

    EXPECT_NEAR(M_PI * 2 * 0.5, ctiglElement->GetCircumferenceOfProfile(), 0.1);

    EXPECT_NEAR(M_PI * 2, ctiglElement->GetCircumferenceOfProfile(FUSELAGE_COORDINATE_SYSTEM), 0.1);

    // Test Set functions

    tigl::CTiglPoint newOrigin, newCenter;

    newOrigin = tigl::CTiglPoint(0, 2, 2);
    ctiglElement->SetOrigin(newOrigin);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin()));
    tigl::CTiglPoint elementTranslation = element.GetTranslation();
    EXPECT_TRUE(tigl::CTiglPoint(-0.5, 4, 4).isNear(elementTranslation));

    newCenter = tigl::CTiglPoint(1, 0, 1);
    ctiglElement->SetCenter(newCenter, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newCenter.isNear(ctiglElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(1, 0, 0.5).isNear(ctiglElement->GetCenter()));
    elementTranslation = element.GetTranslation();
    EXPECT_TRUE(tigl::CTiglPoint(0.5, -1, 1).isNear(elementTranslation));

    newCenter = tigl::CTiglPoint(0.5, 0, 0);
    ctiglElement->SetCenter(newCenter);
    EXPECT_TRUE(newCenter.isNear(ctiglElement->GetCenter()));

    // save the file for visual check
    saveCurrentConfig("TestData/Output/simpletest-shifftedProfiles-out.xml");
}
