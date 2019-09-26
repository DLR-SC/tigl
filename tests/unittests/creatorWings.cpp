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
#include <tixi.h>
#include <tixicpp.h>
#include "CPACSWing.h"
#include "CPACSWings.h"
#include "CCPACSWingSection.h"
#include "CCPACSWingSectionElement.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"

class creatorWings : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfigurationManager* manager = nullptr;
    tigl::CCPACSConfiguration* config         = nullptr;
    tigl::CCPACSWings* wings                  = nullptr;

    void setVariables(std::string inFilename)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager = &(tigl::CCPACSConfigurationManager::GetInstance());
        config  = &(manager->GetConfiguration(tiglHandle));
        wings   = &(config->GetWings());
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
        wings   = nullptr;
    }

    // Save the result in a new file (For visual check purpose)
    void saveInOutputFile()
    {
        // write the change in tixi memory
        config->WriteCPACS(config->GetUID());
        std::string newConfig = tixi::TixiExportDocumentAsString(tixiHandle);

        // Import-export to flat the xml // todo inform tixi developers about this "bug
        TixiDocumentHandle tixiHandle2 = tixi::TixiImportFromString(newConfig);
        newConfig                      = tixi::TixiExportDocumentAsString(tixiHandle2);

        std::ofstream myfile;
        myfile.open("TestData/Output/creatorWings-out.xml", std::ios::trunc);
        myfile << newConfig;
        myfile.close();
        tixiCloseDocument(tixiHandle2);
    }

    void TearDown()
    {
        unsetVariables();
    }
};


TEST_F(creatorWings, createWing_emptyModel)
{
    setVariables("TestData/emptyWithProfiles.cpacs3.xml");

    tigl::CCPACSWing& wing = wings->CreateWing("W1",3,"NACA0012");
    ASSERT_EQ(wing.GetUID(), "W1");
    ASSERT_EQ(wing.GetSectionCount(), 3);
    ASSERT_EQ(wing.GetSegmentCount(), 2);
    ASSERT_EQ(wing.GetSection(1).GetSectionElement(1).GetCTiglSectionElement()->GetProfileUID(), "NACA0012");

    tigl::CCPACSWing& wing2 = wings->CreateWing("W2",23,"NACA0012");
    wing2.SetRootLEPosition(tigl::CTiglPoint(15,0,0));
    ASSERT_EQ(wing2.GetUID(), "W2");
    ASSERT_EQ(wing2.GetSectionCount(), 23);
    ASSERT_EQ(wing2.GetSegmentCount(), 22);
    ASSERT_EQ(wing2.GetSection(1).GetSectionElement(1).GetCTiglSectionElement()->GetProfileUID(), "NACA0012");

    saveInOutputFile();


}
