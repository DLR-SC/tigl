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



class creatorPositionings : public ::testing::Test
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
        myfile.open("TestData/Output/creatorPositionings-out.xml", std::ios::trunc);
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


TEST_F(creatorPositionings, GetPositioningTransformation)
{
    setVariables("TestData/simpletest-positionings.xml");

    tigl::CTiglTransformation m;
    tigl::CTiglPoint t, tE ;
    tigl::CTiglSectionElement* cElement;

    tigl::CCPACSPositionings& positionings = config->GetFuselage(1).GetPositionings().value();

    m = positionings.GetPositioningTransformation("D150_Fuselage_1Section1ID");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(tigl::CTiglPoint(-0.5,0,0)));

    m = positionings.GetPositioningTransformation("D150_Fuselage_1Section2ID");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(tigl::CTiglPoint(1.5,0,0))); // -0.5 + 2
    // Remark the position take not in account the translation of the section trans see:
    cElement = GetCElementOf("D150_Fuselage_1Section2IDElement1");
    tE = cElement->GetTotalTransformation().GetTranslation();
    EXPECT_TRUE(tE.isNear(tigl::CTiglPoint(2.2,0,0))); // -0.5 + 2 + 0.7

    // positionings lives in fuselage or wing space (the wing or fuselage transformation is not applied on it)
    // the positioning take only positioning into account also for the from section position
    m = positionings.GetPositioningTransformation("D150_Fuselage_1Section3ID");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(tigl::CTiglPoint(2.5,1,0), 0.0001));
    cElement = GetCElementOf("D150_Fuselage_1Section3IDElement1");
    tE = cElement->GetTotalTransformation().GetTranslation();
    EXPECT_TRUE(tE.isNear(tigl::CTiglPoint(2.5,0.5,0), 0.0001));

}


TEST_F(creatorPositionings, SetPositioningTransformation)
{
    setVariables("TestData/simpletest-positionings.xml");

    saveInOutputFile();

    tigl::CTiglTransformation m;
    tigl::CTiglPoint t, tE, depT, depTAfter ;
    tigl::CTiglSectionElement* cElement;
    tigl::CTiglPoint newPoint;

    tigl::CCPACSPositionings* positionings = &(config->GetFuselage(1).GetPositionings().value());

    newPoint = tigl::CTiglPoint(-3,0,0);
    depT = positionings->GetPositioningTransformation("D150_Fuselage_1Section2ID").GetTranslation(); // dependent position of D150_Fuselage_1Section1ID
    positionings->SetPositioningTransformation("D150_Fuselage_1Section1ID", newPoint, false);
    m = positionings->GetPositioningTransformation("D150_Fuselage_1Section1ID");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(newPoint));
    cElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    tE = cElement->GetTotalTransformation().GetTranslation();
    EXPECT_TRUE(tE.isNear(newPoint));
    depTAfter = positionings->GetPositioningTransformation("D150_Fuselage_1Section2ID").GetTranslation(); // should not have moved
    EXPECT_TRUE(depTAfter.isNear(depT));

    saveInOutputFile();

    setVariables("TestData/simpletest-positionings.xml");
    positionings = &(config->GetFuselage(1).GetPositionings().value());

    newPoint = tigl::CTiglPoint(-3,0,0);
    depT = positionings->GetPositioningTransformation("D150_Fuselage_1Section2ID").GetTranslation(); // dependent position of D150_Fuselage_1Section1ID
    positionings->SetPositioningTransformation("D150_Fuselage_1Section1ID", newPoint, true); // this time we want to have the cascading effect
    m = positionings->GetPositioningTransformation("D150_Fuselage_1Section1ID");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(newPoint));
    cElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    tE = cElement->GetTotalTransformation().GetTranslation();
    EXPECT_TRUE(tE.isNear(newPoint));
    depTAfter = positionings->GetPositioningTransformation("D150_Fuselage_1Section2ID").GetTranslation(); // should not have moved
    EXPECT_TRUE(depTAfter.isNear(depT + tigl::CTiglPoint(-2.5,0,0)));   // it's shifted

    saveInOutputFile();

    // wing
    positionings = &(config->GetWing(1).GetPositionings().value());

    newPoint = tigl::CTiglPoint(-1,2,1);
    depT = positionings->GetPositioningTransformation("Cpacs2Test_Wing_Sec3").GetTranslation(); // dependent position of D150_Fuselage_1Section1ID
    positionings->SetPositioningTransformation("Cpacs2Test_Wing_Sec2", newPoint, false); // this time we want to have the cascading effect
    m = positionings->GetPositioningTransformation("Cpacs2Test_Wing_Sec2");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(newPoint));
    cElement = GetCElementOf("Cpacs2Test_Wing_Sec2_El1");
    tE = cElement->GetTotalTransformation().GetTranslation();
    EXPECT_TRUE(tE.isNear(tigl::CTiglPoint(-1,2,0.5)));
    depTAfter = positionings->GetPositioningTransformation("Cpacs2Test_Wing_Sec3").GetTranslation(); // should not have moved
    EXPECT_TRUE(depTAfter.isNear(depT) );

    saveInOutputFile();


    newPoint = tigl::CTiglPoint(1,2,1);
    depT = positionings->GetPositioningTransformation("Cpacs2Test_Wing_Sec3").GetTranslation(); // dependent position of D150_Fuselage_1Section1ID
    positionings->SetPositioningTransformation("Cpacs2Test_Wing_Sec2", newPoint, false); // this time we want to have the cascading effect
    m = positionings->GetPositioningTransformation("Cpacs2Test_Wing_Sec2");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(newPoint));
    cElement = GetCElementOf("Cpacs2Test_Wing_Sec2_El1");
    tE = cElement->GetTotalTransformation().GetTranslation();
    EXPECT_TRUE(tE.isNear(tigl::CTiglPoint(1,2,0.5)));
    depTAfter = positionings->GetPositioningTransformation("Cpacs2Test_Wing_Sec3").GetTranslation(); // should not have moved
    EXPECT_TRUE(depTAfter.isNear(depT) );

    saveInOutputFile();

    // create new positioning when needed

    positionings = &(config->GetFuselage(2).GetPositionings().value());

    newPoint = tigl::CTiglPoint( 5,0,0);
    // no positioning is present for know
    EXPECT_TRUE(positionings->GetPositioningTransformation("FuselageNoPos_1Section2ID").GetTranslation().isNear(tigl::CTiglPoint(0,0,0)));
    positionings->SetPositioningTransformation("FuselageNoPos_1Section2ID", newPoint, false);
    m = positionings->GetPositioningTransformation("FuselageNoPos_1Section2ID");
    t = m.GetTranslation();
    EXPECT_TRUE(t.isNear(newPoint));
    cElement = GetCElementOf("FuselageNoPos_1Section2IDElement1");
    tE = cElement->GetTotalTransformation().GetTranslation();
    EXPECT_TRUE(tE.isNear(tigl::CTiglPoint(-3,0,0)));

    saveInOutputFile();




}