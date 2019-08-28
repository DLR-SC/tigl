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
#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSPositionings.h"
#include "CCPACSPositioning.h"

#include <iostream>
#include <fstream>

#include <string.h>

class creatorFuselages : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfigurationManager* manager = nullptr;
    tigl::CCPACSConfiguration* config         = nullptr;
    tigl::CCPACSFuselages* fuselages          = nullptr;

    void setVariables(std::string inFilename)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager   = &(tigl::CCPACSConfigurationManager::GetInstance());
        config    = &(manager->GetConfiguration(tiglHandle));
        fuselages = &(config->GetFuselages());
    }

    void setVariables(std::string inFilename, std::string modelUID)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, modelUID.c_str(), &tiglHandle));
        manager   = &(tigl::CCPACSConfigurationManager::GetInstance());
        config    = &(manager->GetConfiguration(tiglHandle));
        fuselages = &(config->GetFuselages());
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

        manager   = nullptr;
        config    = nullptr;
        fuselages = nullptr;
    }

    void TearDown()
    {
        unsetVariables();
    }
};

TEST_F(creatorFuselages, createFuselage_emptyModel)
{
    setVariables("TestData/emptyWithProfiles.cpacs3.xml");

    // create a valid fuselage with 3 sections
    tigl::CCPACSFuselage& fuselage = fuselages->CreateFuselage("Fuselage1", 3, "fuselageCircleProfile1");

    EXPECT_EQ("Fuselage1", fuselages->GetFuselage(1).GetUID());
    EXPECT_EQ("Fuselage1Sec2Tr", fuselages->GetFuselage(1).GetSection(2).GetTransformation().GetUID());
    EXPECT_EQ("Fuselage1Sec3Elem1", fuselages->GetFuselage(1).GetSection(3).GetSectionElement(1).GetUID());
    EXPECT_EQ(0, fuselages->GetFuselage(1).GetSection(3).GetSectionElement(1).GetTranslation().x);
    EXPECT_EQ(2, fuselages->GetFuselage(1).GetPositionings()->GetPositionings().at(2)->GetToPoint().x);


    // create a valid fuselage with 9 sections
    tigl::CCPACSFuselage& fuselage2 = fuselages->CreateFuselage("Fuselage2", 9, "fuselageCircleProfile1");

    EXPECT_EQ("Fuselage2", fuselages->GetFuselage(2).GetUID());
    EXPECT_EQ("Fuselage2Sec9TrTransl",
              fuselages->GetFuselage(2).GetSection(9).GetTransformation().GetTranslation().value().GetUID());
    EXPECT_EQ("Fuselage2Sec3Elem1", fuselages->GetFuselage(2).GetSection(3).GetSectionElement(1).GetUID());
    EXPECT_EQ(7, fuselages->GetFuselage(2).GetPositionings()->GetPositionings().at(7)->GetToPoint().x);

    // todo:  Now the following function with 13 sections make tiglViewer crash. Why? Seems to be a tiglviewer bug!
    // create a valid fuselage with 13 sections
    //tigl::CCPACSFuselage& fuselage3 = fuselages->CreateFuselage("Fuselage13", 13, "fuselageCircleProfile1");

    // try to create a fuselage with a uid already present in the file
    EXPECT_THROW(fuselages->CreateFuselage("CpacsAircraft", 3, "fuselageCircleProfile1"), tigl::CTiglError);

    // try to create a fuselage with a invalid fuselage profile
    EXPECT_THROW(fuselages->CreateFuselage("Fuselage1", 3, "fasdfas"), tigl::CTiglError);

    // try to create a fuselage with a invalid fuselage profile
    EXPECT_THROW(fuselages->CreateFuselage("Fuselage1", 3, "NACA0012"), tigl::CTiglError);

    // Save the result in a new file (For visual check purpose)
    // write the change in tixi memory
    config->WriteCPACS(config->GetUID());
    std::string newConfig = tixi::TixiExportDocumentAsString(tixiHandle);

    // Import-export to flat the xml // todo inform tixi developers about this "bug
    TixiDocumentHandle tixiHandle2 = tixi::TixiImportFromString(newConfig);
    newConfig                      = tixi::TixiExportDocumentAsString(tixiHandle2);
    // save the content into the output file
    std::ofstream myfile;
    myfile.open("TestData/Output/emptyWithProfile-out.xml", std::ios::trunc);
    myfile << newConfig;
    myfile.close();
    tixiCloseDocument(tixiHandle2);

    // check if we can open the created file correctly
    setVariables("TestData/Output/emptyWithProfile-out.xml");
    EXPECT_EQ(2, fuselages->GetFuselageCount());
}