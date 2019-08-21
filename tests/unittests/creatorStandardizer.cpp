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

#include <tixi.h>
#include <tixicpp.h>
#include "test.h"
#include "tigl.h"
#include "CCPACSPositionings.h"
#include "CCPACSPositioning.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglSectionElement.h"
#include "CTiglWingSectionElement.h"
#include "CTiglFuselageSectionElement.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSWingSectionElement.h"
#include "CTiglStandardizer.h"
#include "UniquePtr.h"

class creatorStandardizer : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfigurationManager* manager = nullptr;
    tigl::CCPACSConfiguration* config         = nullptr;

    void setVariables(std::string inFilename)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager = &(tigl::CCPACSConfigurationManager::GetInstance());
        config  = &(manager->GetConfiguration(tiglHandle));
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
        myfile.open("TestData/Output/creatorStandardizer-out.xml", std::ios::trunc);
        myfile << newConfig;
        myfile.close();
        tixiCloseDocument(tixiHandle2);
    }

    tigl::CTiglSectionElement* GetCElementOf(std::string elementUid)
    {
        tigl::CTiglSectionElement* cElement     = nullptr;
        tigl::CTiglUIDManager::TypedPtr typePtr = config->GetUIDManager().ResolveObject(elementUid);
        if (typePtr.type == &typeid(tigl::CCPACSFuselageSectionElement)) {
            tigl::CCPACSFuselageSectionElement& fuselageElement =
                *reinterpret_cast<tigl::CCPACSFuselageSectionElement*>(typePtr.ptr);
            cElement = fuselageElement.GetCTiglSectionElement();
        }
        else if (typePtr.type == &typeid(tigl::CCPACSWingSectionElement)) {
            tigl::CCPACSWingSectionElement& wingElement =
                *reinterpret_cast<tigl::CCPACSWingSectionElement*>(typePtr.ptr);
            cElement = wingElement.GetCTiglSectionElement();
        }
        return cElement;
    }

    void TearDown()
    {
        unsetVariables();
    }
};

TEST_F(creatorStandardizer, standardizeFuselage)
{
    setVariables("TestData/multiple_fuselages.xml");

    saveInOutputFile();
    tigl::CCPACSFuselage& fuselage = config->GetFuselage("SimpleFuselage");

    // save center to verify if they do not move
    std::vector<std::string> elementUIDs = fuselage.GetOrderedConnectedElement();
    std::map<std::string, tigl::CTiglPoint> centers;
    for (int i = 0; i < elementUIDs.size(); i++) {
        centers[elementUIDs.at(i)] = GetCElementOf(elementUIDs.at(i))->GetCenter();
    }

    tigl::CTiglStandardizer::StandardizeFuselage(fuselage);

    saveInOutputFile();
    elementUIDs = fuselage.GetOrderedConnectedElement();
    // verify that each element was not moved
    for (int i = 0; i < elementUIDs.size(); i++) {
        EXPECT_TRUE((centers.at(elementUIDs.at(i))).isNear(GetCElementOf(elementUIDs.at(i))->GetCenter()));
    }
    // verfying that the translation of the fuselage is stored in the fuselage transformation and not in positioning or others
    tigl::CTiglPoint translationFuselage = fuselage.GetTranslation(); // todo verify behavior with parent uid
    EXPECT_TRUE(translationFuselage.isNear(fuselage.GetNoseCenter()));
    // verfying that positonining has the correct structure:
    std::vector<tigl::unique_ptr<tigl::CCPACSPositioning>>& posVec =
        fuselage.GetPositionings(tigl::CreateIfNotExistsTag()).GetPositionings();
    EXPECT_EQ(posVec.size(), 3);
    EXPECT_EQ(*(posVec.at(0)->GetFromSectionUID()), "");
    EXPECT_EQ(posVec.at(0)->GetToSectionUID(), "D150_Fuselage_1Section1ID");
    EXPECT_EQ(*(posVec.at(1)->GetFromSectionUID()), "D150_Fuselage_1Section1ID");
    EXPECT_EQ(posVec.at(1)->GetToSectionUID(), "D150_Fuselage_1Section2ID");
    EXPECT_EQ(*(posVec.at(2)->GetFromSectionUID()), "D150_Fuselage_1Section2ID");
    EXPECT_EQ(posVec.at(2)->GetToSectionUID(), "D150_Fuselage_1Section3ID");
}

TEST_F(creatorStandardizer, standardizeFuselageSimpleDecomposition)
{
    setVariables("TestData/multiple_fuselages.xml");

    saveInOutputFile();
    tigl::CCPACSFuselage& fuselage = config->GetFuselage("SimpleFuselage");

    // save center to verify if they do not move
    std::vector<std::string> elementUIDs = fuselage.GetOrderedConnectedElement();
    std::map<std::string, tigl::CTiglPoint> centers;
    for (int i = 0; i < elementUIDs.size(); i++) {
        centers[elementUIDs.at(i)] = GetCElementOf(elementUIDs.at(i))->GetCenter();
    }

    tigl::CTiglStandardizer::StandardizeFuselage(fuselage, true);

    saveInOutputFile();
    elementUIDs = fuselage.GetOrderedConnectedElement();
    // verify that each element was not moved
    for (int i = 0; i < elementUIDs.size(); i++) {
        EXPECT_TRUE((centers.at(elementUIDs.at(i))).isNear(GetCElementOf(elementUIDs.at(i))->GetCenter()));
    }
    // verfying that the translation of the fuselage is stored in the fuselage transformation and not in positioning or others
    tigl::CTiglPoint translationFuselage = fuselage.GetTranslation(); // todo verify behavior with parent uid
    EXPECT_TRUE(translationFuselage.isNear(fuselage.GetNoseCenter()));
    // verfying that positonining has the correct structure:
    std::vector<tigl::unique_ptr<tigl::CCPACSPositioning>>& posVec =
        fuselage.GetPositionings(tigl::CreateIfNotExistsTag()).GetPositionings();
    EXPECT_EQ(posVec.size(), 3);
    EXPECT_EQ(*(posVec.at(0)->GetFromSectionUID()), "");
    EXPECT_EQ(posVec.at(0)->GetToSectionUID(), "D150_Fuselage_1Section1ID");
    EXPECT_EQ(*(posVec.at(1)->GetFromSectionUID()), "D150_Fuselage_1Section1ID");
    EXPECT_EQ(posVec.at(1)->GetToSectionUID(), "D150_Fuselage_1Section2ID");
    EXPECT_EQ(*(posVec.at(2)->GetFromSectionUID()), "D150_Fuselage_1Section2ID");
    EXPECT_EQ(posVec.at(2)->GetToSectionUID(), "D150_Fuselage_1Section3ID");

    tigl::CCPACSFuselage& fuselage2 = config->GetFuselage("FuselageShearingSection");

    // save center to verify if they do not move
    elementUIDs = fuselage2.GetOrderedConnectedElement();
    centers.clear();
    for (int i = 0; i < elementUIDs.size(); i++) {
        centers[elementUIDs.at(i)] = GetCElementOf(elementUIDs.at(i))->GetCenter();
    }
    double area = GetCElementOf(elementUIDs.at(0))->GetArea();

    tigl::CTiglStandardizer::StandardizeFuselage(fuselage2, true);

    saveInOutputFile();

    elementUIDs = fuselage2.GetOrderedConnectedElement();
    // verify that each element was not moved
    for (int i = 0; i < elementUIDs.size(); i++) {
        EXPECT_TRUE((centers.at(elementUIDs.at(i))).isNear(GetCElementOf(elementUIDs.at(i))->GetCenter()));
    }
    // verfying that the translation of the fuselage is stored in the fuselage transformation and not in positioning or others
    translationFuselage = fuselage2.GetTranslation();
    EXPECT_TRUE(translationFuselage.isNear(fuselage2.GetNoseCenter()));
    //verfying that positonining has the correct structure:
    std::vector<tigl::unique_ptr<tigl::CCPACSPositioning>>& posVec2 =
        fuselage2.GetPositionings(tigl::CreateIfNotExistsTag()).GetPositionings();
    EXPECT_EQ(posVec2.size(), 3);
    EXPECT_EQ(*(posVec2.at(0)->GetFromSectionUID()), "");
    EXPECT_EQ(posVec2.at(0)->GetToSectionUID(), "FuselageShearingSection_1Section1ID");
    EXPECT_EQ(*(posVec2.at(1)->GetFromSectionUID()), "FuselageShearingSection_1Section1ID");
    EXPECT_EQ(posVec2.at(1)->GetToSectionUID(), "FuselageShearingSection_1Section2ID");
    EXPECT_EQ(*(posVec2.at(2)->GetFromSectionUID()), "FuselageShearingSection_1Section2ID");
    EXPECT_EQ(posVec2.at(2)->GetToSectionUID(), "FuselageShearingSection_1Section3ID");

    // but the area is no more the same the fuselage has change its shape.
    EXPECT_FALSE(fabs(area - GetCElementOf(elementUIDs.at(0))->GetArea()) < 0.0000001);

    saveInOutputFile();
}

TEST_F(creatorStandardizer, standardizeWing)
{

    setVariables("TestData/multiple_wings.xml");

    saveInOutputFile();
    tigl::CCPACSWing& wing = config->GetWing("W17_RotSec");

    // save center to verify if they do not move
    std::vector<std::string> elementUIDs = wing.GetOrderedConnectedElement();
    std::map<std::string, tigl::CTiglPoint> centers;
    for (int i = 0; i < elementUIDs.size(); i++) {
        centers[elementUIDs.at(i)] = GetCElementOf(elementUIDs.at(i))->GetCenter();
    }

    tigl::CTiglStandardizer::StandardizeWing(wing);

    saveInOutputFile();
    elementUIDs = wing.GetOrderedConnectedElement();
    // verify that each element was not moved
    for (int i = 0; i < elementUIDs.size(); i++) {
        EXPECT_TRUE((centers.at(elementUIDs.at(i))).isNear(GetCElementOf(elementUIDs.at(i))->GetCenter()));
    }
    // verfying that the translation of the wing is stored in the wing transformation and not in positioning or others
    tigl::CTiglPoint translationWing = wing.GetTranslation(); // todo verify behavior with parent uid
    EXPECT_TRUE(translationWing.isNear(wing.GetRootLEPosition()));
    // verfying that positonining has the correct structure:
    std::vector<tigl::unique_ptr<tigl::CCPACSPositioning>>& posVec =
        wing.GetPositionings(tigl::CreateIfNotExistsTag()).GetPositionings();
    EXPECT_EQ(posVec.size(), 3);
    EXPECT_EQ(*(posVec.at(0)->GetFromSectionUID()), "");
    EXPECT_EQ(posVec.at(0)->GetToSectionUID(), "W17_RotSec_Sec1");
    EXPECT_EQ(*(posVec.at(1)->GetFromSectionUID()), "W17_RotSec_Sec1");
    EXPECT_EQ(posVec.at(1)->GetToSectionUID(), "W17_RotSec_Sec2");
    EXPECT_EQ(*(posVec.at(2)->GetFromSectionUID()), "W17_RotSec_Sec2");
    EXPECT_EQ(posVec.at(2)->GetToSectionUID(), "W17_RotSec_Sec3");

    tigl::CCPACSWing& wing2 = config->GetWing("Wing");
    centers.clear();
    elementUIDs.clear();

    // save center to verify if they do not move
    elementUIDs = wing2.GetOrderedConnectedElement();
    for (int i = 0; i < elementUIDs.size(); i++) {
        centers[elementUIDs.at(i)] = GetCElementOf(elementUIDs.at(i))->GetCenter();
    }

    tigl::CTiglStandardizer::StandardizeWing(wing2);

    saveInOutputFile();
    elementUIDs = wing2.GetOrderedConnectedElement();
    // verify that each element was not moved
    for (int i = 0; i < elementUIDs.size(); i++) {
        EXPECT_TRUE((centers.at(elementUIDs.at(i))).isNear(GetCElementOf(elementUIDs.at(i))->GetCenter()));
    }
    // verfying that the translation of the wing is stored in the wing transformation and not in positioning or others
    translationWing = wing2.GetTranslation(); // todo verify behavior with parent uid
    EXPECT_TRUE(translationWing.isNear(wing2.GetRootLEPosition()));
    // verfying that positonining has the correct structure:
    std::vector<tigl::unique_ptr<tigl::CCPACSPositioning>>& posVec2 =
        wing2.GetPositionings(tigl::CreateIfNotExistsTag()).GetPositionings();
    EXPECT_EQ(posVec2.size(), 3);
    EXPECT_EQ(*(posVec2.at(0)->GetFromSectionUID()), "");
    EXPECT_EQ(posVec2.at(0)->GetToSectionUID(), "Cpacs2Test_Wing_Sec1");
    EXPECT_EQ(*(posVec2.at(1)->GetFromSectionUID()), "Cpacs2Test_Wing_Sec1");
    EXPECT_EQ(posVec2.at(1)->GetToSectionUID(), "Cpacs2Test_Wing_Sec2");
    EXPECT_EQ(*(posVec2.at(2)->GetFromSectionUID()), "Cpacs2Test_Wing_Sec2");
    EXPECT_EQ(posVec2.at(2)->GetToSectionUID(), "Cpacs2Test_Wing_Sec3");
}

TEST_F(creatorStandardizer, standardizeWingSimpleDecomposition)
{

    setVariables("TestData/multiple_wings.xml");

    saveInOutputFile();
    tigl::CCPACSWing& wing = config->GetWing("W17_RotSec");

    // save center to verify if they do not move
    std::vector<std::string> elementUIDs = wing.GetOrderedConnectedElement();
    std::map<std::string, tigl::CTiglPoint> centers;
    for (int i = 0; i < elementUIDs.size(); i++) {
        centers[elementUIDs.at(i)] = GetCElementOf(elementUIDs.at(i))->GetCenter();
    }

    tigl::CTiglStandardizer::StandardizeWing(wing, true);

    saveInOutputFile();
    elementUIDs = wing.GetOrderedConnectedElement();
    // verify that each element was not moved
    for (int i = 0; i < elementUIDs.size(); i++) {
        EXPECT_TRUE((centers.at(elementUIDs.at(i))).isNear(GetCElementOf(elementUIDs.at(i))->GetCenter()));
    }
    // verfying that the translation of the wing is stored in the wing transformation and not in positioning or others
    tigl::CTiglPoint translationWing = wing.GetTranslation(); // todo verify behavior with parent uid
    EXPECT_TRUE(translationWing.isNear(wing.GetRootLEPosition()));
    // verfying that positonining has the correct structure:
    std::vector<tigl::unique_ptr<tigl::CCPACSPositioning>>& posVec =
        wing.GetPositionings(tigl::CreateIfNotExistsTag()).GetPositionings();
    EXPECT_EQ(posVec.size(), 3);
    EXPECT_EQ(*(posVec.at(0)->GetFromSectionUID()), "");
    EXPECT_EQ(posVec.at(0)->GetToSectionUID(), "W17_RotSec_Sec1");
    EXPECT_EQ(*(posVec.at(1)->GetFromSectionUID()), "W17_RotSec_Sec1");
    EXPECT_EQ(posVec.at(1)->GetToSectionUID(), "W17_RotSec_Sec2");
    EXPECT_EQ(*(posVec.at(2)->GetFromSectionUID()), "W17_RotSec_Sec2");
    EXPECT_EQ(posVec.at(2)->GetToSectionUID(), "W17_RotSec_Sec3");

    tigl::CCPACSWing& wing2 = config->GetWing("Wing");
    centers.clear();
    elementUIDs.clear();

    // save center to verify if they do not move
    elementUIDs = wing2.GetOrderedConnectedElement();
    for (int i = 0; i < elementUIDs.size(); i++) {
        centers[elementUIDs.at(i)] = GetCElementOf(elementUIDs.at(i))->GetCenter();
    }

    tigl::CTiglStandardizer::StandardizeWing(wing2, true);

    saveInOutputFile();
    elementUIDs = wing2.GetOrderedConnectedElement();
    // verify that each element was not moved
    for (int i = 0; i < elementUIDs.size(); i++) {
        EXPECT_TRUE((centers.at(elementUIDs.at(i))).isNear(GetCElementOf(elementUIDs.at(i))->GetCenter()));
    }
    // verfying that the translation of the wing is stored in the wing transformation and not in positioning or others
    translationWing = wing2.GetTranslation(); // todo verify behavior with parent uid
    EXPECT_TRUE(translationWing.isNear(wing2.GetRootLEPosition()));
    // verfying that positonining has the correct structure:
    std::vector<tigl::unique_ptr<tigl::CCPACSPositioning>>& posVec2 =
        wing2.GetPositionings(tigl::CreateIfNotExistsTag()).GetPositionings();
    EXPECT_EQ(posVec2.size(), 3);
    EXPECT_EQ(*(posVec2.at(0)->GetFromSectionUID()), "");
    EXPECT_EQ(posVec2.at(0)->GetToSectionUID(), "Cpacs2Test_Wing_Sec1");
    EXPECT_EQ(*(posVec2.at(1)->GetFromSectionUID()), "Cpacs2Test_Wing_Sec1");
    EXPECT_EQ(posVec2.at(1)->GetToSectionUID(), "Cpacs2Test_Wing_Sec2");
    EXPECT_EQ(*(posVec2.at(2)->GetFromSectionUID()), "Cpacs2Test_Wing_Sec2");
    EXPECT_EQ(posVec2.at(2)->GetToSectionUID(), "Cpacs2Test_Wing_Sec3");
}