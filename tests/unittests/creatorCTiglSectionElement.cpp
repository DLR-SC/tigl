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
#include "CTiglSectionElement.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSWingSectionElement.h"
#include "CTiglPoint.h"

#include <string.h>

class creatorCTiglSectionElement : public ::testing::Test
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
        manager  = &(tigl::CCPACSConfigurationManager::GetInstance());
        config   = &(manager->GetConfiguration(tiglHandle));
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
    }

    void TearDown()
    {
        unsetVariables();
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


    tigl::CTiglWingSectionElement* GetWingCElementOf(std::string elementUid)
    {

        tigl::CTiglUIDManager::TypedPtr typePtr = config->GetUIDManager().ResolveObject(elementUid);
        if (typePtr.type == &typeid(tigl::CCPACSWingSectionElement)) {
            tigl::CCPACSWingSectionElement& wingElement =
                    *reinterpret_cast<tigl::CCPACSWingSectionElement*>(typePtr.ptr);
            return wingElement.GetCTiglSectionElement();
        }
        return nullptr;
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
};

TEST_F(creatorCTiglSectionElement, getOrigin)
{

    tigl::CTiglSectionElement* ctiglElement = nullptr;

    setVariables("TestData/simpletest.cpacs.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    EXPECT_TRUE(tigl::CTiglPoint(-0.5, 0, 0).isNear(ctiglElement->GetOrigin()));

    setVariables("TestData/simpletest-shifftedProfiles.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section2IDElement1");
    EXPECT_TRUE(tigl::CTiglPoint(0.5, 0, 0).isNear(ctiglElement->GetOrigin()));
    EXPECT_TRUE(tigl::CTiglPoint(0.5, 0, 0).isNear(ctiglElement->GetOrigin(FUSELAGE_COORDINATE_SYSTEM)));
}

TEST_F(creatorCTiglSectionElement, getCenter)
{

    tigl::CTiglSectionElement* ctiglElement = nullptr;

    setVariables("TestData/simpletest.cpacs.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    EXPECT_TRUE(tigl::CTiglPoint(-0.5, 0, 0).isNear(ctiglElement->GetCenter()));

    setVariables("TestData/simpletest-shifftedProfiles.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section2IDElement1");
    EXPECT_TRUE(tigl::CTiglPoint(0.5, 0.5, 0).isNear(ctiglElement->GetCenter()));
    EXPECT_TRUE(tigl::CTiglPoint(0.5, 1, 0).isNear(ctiglElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM)));
}

TEST_F(creatorCTiglSectionElement, getCircumference)
{

    tigl::CTiglSectionElement* ctiglElement = nullptr;

    setVariables("TestData/simpletest.cpacs.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    EXPECT_NEAR(M_PI * 2 * 0.5, ctiglElement->GetCircumference(), 0.1);
    EXPECT_NEAR(M_PI * 2, ctiglElement->GetCircumference(FUSELAGE_COORDINATE_SYSTEM), 0.1);

    setVariables("TestData/simpletest-shifftedProfiles.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section2IDElement1");
    EXPECT_NEAR(M_PI * 2 * 0.5, ctiglElement->GetCircumference(), 0.1);
    EXPECT_NEAR(M_PI * 2, ctiglElement->GetCircumference(FUSELAGE_COORDINATE_SYSTEM), 0.1);
}

TEST_F(creatorCTiglSectionElement, setOrigin)
{

    tigl::CTiglSectionElement* ctiglElement = nullptr;
    tigl::CTiglPoint newOrigin;
    tigl::CCPACSFuselageSectionElement* element = nullptr;

    setVariables("TestData/simpletest.cpacs.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    element      = &(
            config->GetUIDManager().ResolveObject<tigl::CCPACSFuselageSectionElement>("D150_Fuselage_1Section1IDElement1"));

    newOrigin = tigl::CTiglPoint(2, 1.1, 0.13);
    ctiglElement->SetOrigin(newOrigin);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin()));

    newOrigin = tigl::CTiglPoint(-3, 1, 2);
    ctiglElement->SetOrigin(newOrigin);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin()));
    // check the value of the element transformation
    // remark that to compute the element translation we must take in account the position and the scaling that affect this transformation
    tigl::CTiglPoint elementTranslation = element->GetTranslation();
    ;
    EXPECT_TRUE(tigl::CTiglPoint(-2.5, 2, 4).isNear(elementTranslation));

    newOrigin = tigl::CTiglPoint(-3, 1, 2);
    ctiglElement->SetOrigin(newOrigin, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(-3, 0.5, 1).isNear(ctiglElement->GetOrigin()));

    newOrigin = tigl::CTiglPoint(-3, 1, 2);
    ctiglElement->SetOrigin(newOrigin, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(-3, 0.5, 1).isNear(ctiglElement->GetOrigin()));

    // save the file for visual check
    saveCurrentConfig("TestData/Output/simpletest-out.xml");

    setVariables("TestData/simpletest-shifftedProfiles.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section2IDElement1");
    element      = &(
        config->GetUIDManager().ResolveObject<tigl::CCPACSFuselageSectionElement>("D150_Fuselage_1Section2IDElement1"));

    newOrigin = tigl::CTiglPoint(0, 2, 2);
    ctiglElement->SetOrigin(newOrigin);
    EXPECT_TRUE(newOrigin.isNear(ctiglElement->GetOrigin()));
    elementTranslation = element->GetTranslation();
    EXPECT_TRUE(tigl::CTiglPoint(-0.5, 4, 4).isNear(elementTranslation));

    saveCurrentConfig("TestData/Output/simpletest-shifftedProfiles-out.xml");
}

TEST_F(creatorCTiglSectionElement, setCenter)
{

    tigl::CTiglSectionElement* ctiglElement = nullptr;
    tigl::CTiglPoint newCenter;
    tigl::CCPACSFuselageSectionElement* element = nullptr;

    setVariables("TestData/simpletest.cpacs.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");

    newCenter = tigl::CTiglPoint(-5, 1, 2);
    ctiglElement->SetCenter(newCenter, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newCenter.isNear(ctiglElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(-5, 0.5, 1).isNear(ctiglElement->GetCenter()));

    // save the file for visual check
    saveCurrentConfig("TestData/Output/simpletest-out.xml");

    setVariables("TestData/simpletest-shifftedProfiles.xml");
    ctiglElement = GetCElementOf("D150_Fuselage_1Section2IDElement1");
    element      = &(
        config->GetUIDManager().ResolveObject<tigl::CCPACSFuselageSectionElement>("D150_Fuselage_1Section2IDElement1"));

    newCenter = tigl::CTiglPoint(1, 0, 1);
    ctiglElement->SetCenter(newCenter, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(newCenter.isNear(ctiglElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM)));
    EXPECT_TRUE(tigl::CTiglPoint(1, 0, 0.5).isNear(ctiglElement->GetCenter()));
    tigl::CTiglPoint elementTranslation = element->GetTranslation();
    EXPECT_TRUE(tigl::CTiglPoint(0.5, -1, 1).isNear(elementTranslation));

    newCenter = tigl::CTiglPoint(0.5, 0, 0);
    ctiglElement->SetCenter(newCenter);
    EXPECT_TRUE(newCenter.isNear(ctiglElement->GetCenter()));

    // save the file for visual check
    saveCurrentConfig("TestData/Output/simpletest-shifftedProfiles-out.xml");
}

TEST_F(creatorCTiglSectionElement, getProfileArea_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    double area                         = -1;

    cElement = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    area     = cElement->GetArea();
    EXPECT_NEAR(area, 1 * M_PI, 0.5);

    cElement = GetCElementOf("D150_Fuselage_CSection2IDElement1");
    area     = cElement->GetArea();
    EXPECT_NEAR(area, 28, 5);

    cElement = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    area     = cElement->GetArea();
    EXPECT_NEAR(area, 0.8, 0.5);
}

TEST_F(creatorCTiglSectionElement, getNormal_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    tigl::CTiglPoint n, expectedN;

    cElement  = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    n         = cElement->GetNormal();
    expectedN = tigl::CTiglPoint(1, 0, 0);
    EXPECT_TRUE(n.isNear(expectedN));

    n = cElement->GetNormal(TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_TRUE(n.isNear(expectedN));

    cElement  = GetCElementOf("D150_Fuselage_CSection2IDElement1");
    n         = cElement->GetNormal();
    expectedN = tigl::CTiglPoint(0.925, 0.342, -0.163);
    EXPECT_TRUE(n.isNear(expectedN, 0.001));

    cElement  = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    n         = cElement->GetNormal();
    expectedN = tigl::CTiglPoint(1, 0, 0);
    EXPECT_TRUE(n.isNear(expectedN));

    // simple fuselage 2 !

    cElement  = GetCElementOf("D150_Fuselage_2Section1IDElement1");
    n         = cElement->GetNormal();
    expectedN = tigl::CTiglPoint(1, 0, 0);
    EXPECT_TRUE(n.isNear(expectedN));

    n         = cElement->GetNormal(TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM);
    expectedN = tigl::CTiglPoint(1, 0, 0);
    EXPECT_TRUE(n.isNear(expectedN));

    // simple fuselage 3 !

    cElement  = GetCElementOf("D150_Fuselage_3Section1IDElement1");
    n         = cElement->GetNormal();
    expectedN = tigl::CTiglPoint(0.766, 0.642, 0);
    EXPECT_TRUE(n.isNear(expectedN));

    n         = cElement->GetNormal(TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM);
    expectedN = tigl::CTiglPoint(1, 0, 0);
    EXPECT_TRUE(n.isNear(expectedN));

    // simple fuselage 4 !

    cElement  = GetCElementOf("D150_Fuselage_4Section1IDElement1");
    n         = cElement->GetNormal();
    expectedN = tigl::CTiglPoint(0.766, 0.642, 0);
    EXPECT_TRUE(n.isNear(expectedN));

    n         = cElement->GetNormal(TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM);
    expectedN = tigl::CTiglPoint(1, 0, 0);
    EXPECT_TRUE(n.isNear(expectedN));
}

TEST_F(creatorCTiglSectionElement, getProfileHeight_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    double height                       = -1;

    cElement = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    height   = cElement->GetHeight();
    EXPECT_NEAR(height, 2, 0.1);

    cElement = GetCElementOf("D150_Fuselage_CSection2IDElement1");
    height   = cElement->GetHeight();
    EXPECT_NEAR(height, 4, 0.2);

    cElement = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    height   = cElement->GetHeight();
    EXPECT_NEAR(height, 1, 0.2);

    cElement = GetCElementOf("Fuselage_ETSection1IDElement1");
    height   = cElement->GetHeight();
    EXPECT_NEAR(height, 0.6, 0.01);


}

TEST_F(creatorCTiglSectionElement, getProfileWidth_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    double width                       = -1;

    cElement = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    width   = cElement->GetWidth();
    EXPECT_NEAR(width, 2, 0.1);

    cElement = GetCElementOf("D150_Fuselage_CSection2IDElement1");
    width   = cElement->GetWidth();
    EXPECT_NEAR(width, 8, 0.2);

    cElement = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    width   = cElement->GetWidth();
    EXPECT_NEAR(width, 1, 0.2);

    cElement = GetCElementOf("Fuselage_ETSection1IDElement1");
    width   = cElement->GetWidth();
    EXPECT_NEAR(width, 0.2, 0.01);
}

TEST_F(creatorCTiglSectionElement, ScaleUniformely_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    double scaleFactor;
    double height, width, area;
    tigl::CTiglPoint center;

    cElement    = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    scaleFactor = 3;
    height      = cElement->GetHeight();
    width       = cElement->GetWidth();
    area        = cElement->GetArea();
    center      = cElement->GetCenter();
    cElement->ScaleUniformly(scaleFactor);
    EXPECT_NEAR(height * scaleFactor, cElement->GetHeight(), 0.01);
    EXPECT_NEAR(width * scaleFactor, cElement->GetWidth(), 0.01);
    EXPECT_NEAR(area * scaleFactor * scaleFactor, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));


    cElement    = GetCElementOf("D150_Fuselage_CSection2IDElement1");
    scaleFactor = 1.7;
    height      = cElement->GetHeight();
    width       = cElement->GetWidth();
    area        = cElement->GetArea();
    center      = cElement->GetCenter();
    cElement->ScaleUniformly(scaleFactor);
    EXPECT_NEAR(height * scaleFactor, cElement->GetHeight(), 0.01);
    EXPECT_NEAR(width * scaleFactor, cElement->GetWidth(), 0.01);
    EXPECT_NEAR(area * scaleFactor * scaleFactor, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    cElement    = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    scaleFactor = 0.2;
    height      = cElement->GetHeight();
    width       = cElement->GetWidth();
    area        = cElement->GetArea();
    center      = cElement->GetCenter();
    cElement->ScaleUniformly(scaleFactor);
    EXPECT_NEAR(height * scaleFactor, cElement->GetHeight(), 0.01);
    EXPECT_NEAR(width * scaleFactor, cElement->GetWidth(), 0.01);
    EXPECT_NEAR(area * scaleFactor * scaleFactor, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    cElement    = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    scaleFactor = 0.2;
    height      = cElement->GetHeight();
    width       = cElement->GetWidth();
    area        = cElement->GetArea();
    center      = cElement->GetCenter();
    cElement->ScaleUniformly(scaleFactor);
    EXPECT_NEAR(height * scaleFactor, cElement->GetHeight(), 0.01);
    EXPECT_NEAR(width * scaleFactor, cElement->GetWidth(), 0.01);
    EXPECT_NEAR(area * scaleFactor * scaleFactor, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    // the scaling by 0 case
    cElement    = GetCElementOf("Fuselage_ETSection1IDElement1");
    scaleFactor = 0.0;
    height      = cElement->GetHeight();
    width       = cElement->GetWidth();
    area        = cElement->GetArea();
    center      = cElement->GetCenter();
    cElement->ScaleUniformly(scaleFactor);
    EXPECT_NEAR(height * scaleFactor, cElement->GetHeight(), 0.01);
    EXPECT_NEAR(width * scaleFactor, cElement->GetWidth(), 0.01);
    EXPECT_NEAR(area * scaleFactor * scaleFactor, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    // rescale the 0 case
    cElement    = GetCElementOf("Fuselage_ETSection1IDElement1");
    scaleFactor = 10.0;
    height      = cElement->GetHeight();
    width       = cElement->GetWidth();
    area        = cElement->GetArea();
    center      = cElement->GetCenter();
    cElement->ScaleUniformly(scaleFactor);
    EXPECT_NEAR(height, cElement->GetHeight(), 0.01);
    EXPECT_NEAR(width, cElement->GetWidth(), 0.01);
    EXPECT_NEAR(area, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));


    // case in a other CS (scaling of 0.5 in Y,Z direction)
    cElement    = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    scaleFactor = 3;
    height      = cElement->GetHeight(FUSELAGE_COORDINATE_SYSTEM);
    width       = cElement->GetWidth(FUSELAGE_COORDINATE_SYSTEM);
    area        = cElement->GetArea(FUSELAGE_COORDINATE_SYSTEM);
    center      = cElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM);
    cElement->ScaleUniformly(scaleFactor, FUSELAGE_COORDINATE_SYSTEM );
    EXPECT_NEAR(height * scaleFactor , cElement->GetHeight(FUSELAGE_COORDINATE_SYSTEM), 0.01);
    EXPECT_NEAR(height * scaleFactor * 0.5 , cElement->GetHeight(), 0.01);
    EXPECT_NEAR(width * scaleFactor, cElement->GetWidth(FUSELAGE_COORDINATE_SYSTEM), 0.01);
    EXPECT_NEAR(width * scaleFactor * 0.5 , cElement->GetWidth(), 0.01);
    EXPECT_NEAR(area * scaleFactor * scaleFactor, cElement->GetArea(FUSELAGE_COORDINATE_SYSTEM), 0.01);
    EXPECT_NEAR(area * scaleFactor * scaleFactor* 0.5 *0.5, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(FUSELAGE_COORDINATE_SYSTEM), 0.01));


    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
}

TEST_F(creatorCTiglSectionElement, SetWidth_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    double newWidth;
    tigl::CTiglPoint center, normal;

    cElement = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    newWidth = 3;
    center   = cElement->GetCenter();
    normal   = cElement->GetNormal();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth, cElement->GetWidth(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));


    cElement = GetCElementOf("D150_Fuselage_CSection2IDElement1");
    newWidth = 1;
    center   = cElement->GetCenter();
    normal   = cElement->GetNormal();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth, cElement->GetWidth(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));


    cElement = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    newWidth = 3;
    center   = cElement->GetCenter();
    normal   = cElement->GetNormal();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth, cElement->GetWidth(), 0.01);
    // center can change slightly, because the center is computed as the mass center
    // and setting the width can change the shape
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.05));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));



    cElement = GetCElementOf("FuselageShearingSection_1Section1IDElement1");
    newWidth = 1.5;
    center   = cElement->GetCenter();
    normal   = cElement->GetNormal();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth, cElement->GetWidth(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.05));
    tigl::CTiglPoint newNormal = cElement->GetNormal();
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));


    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");

    // the scaling by 0 case
    cElement = GetCElementOf("Fuselage_ETSection1IDElement1");
    newWidth = 0;
    center   = cElement->GetCenter();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth, cElement->GetWidth(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    // we do not check the normal for this case, because the normal is not define clearly when there is a 0 scaling

    // rescale the 0 case
    cElement = GetCElementOf("Fuselage_ETSection1IDElement1");
    newWidth = 3;
    center   = cElement->GetCenter();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth, cElement->GetWidth(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    // we do not check the normal for this case, because the normal is not define clearly when there is a 0 scaling


    cElement    = GetCElementOf("D150_Fuselage_3Section2IDElement1");
    newWidth = 0;
    center   = cElement->GetCenter();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth , cElement->GetWidth(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    newWidth = 1;   // Rescaling it
    center   = cElement->GetCenter();
    cElement->SetWidth(newWidth);
    EXPECT_NEAR(newWidth , cElement->GetWidth(), 0.01);
    tigl::CTiglPoint newCenter = cElement->GetCenter();
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));



    // case in a other CS (scaling of 0.5 in Y,Z direction)
    cElement    = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    newWidth = 3;
    center   = cElement->GetCenter();
    normal   = cElement->GetNormal();
    cElement->SetWidth(newWidth, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_NEAR(newWidth * 0.5, cElement->GetWidth(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));




    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
}

TEST_F(creatorCTiglSectionElement, SetHeight_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    double newHeight;
    tigl::CTiglPoint center, normal;

    cElement  = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    newHeight = 3;
    center    = cElement->GetCenter();
    normal    = cElement->GetNormal();
    cElement->SetHeight(newHeight);
    EXPECT_NEAR(newHeight, cElement->GetHeight(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));


    cElement  = GetCElementOf("D150_Fuselage_CSection2IDElement1");
    newHeight = 0.1;
    center    = cElement->GetCenter();
    normal    = cElement->GetNormal();
    cElement->SetHeight(newHeight);
    EXPECT_NEAR(newHeight, cElement->GetHeight(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));

    cElement  = GetCElementOf("D150_Fuselage_CSection3IDElement1");
    newHeight = 3.12;
    center    = cElement->GetCenter();
    normal    = cElement->GetNormal();
    cElement->SetHeight(newHeight);
    EXPECT_NEAR(newHeight, cElement->GetHeight(), 0.01);
    // center can change slightly, because the center is computed as the mass center
    // and setting the width can change the shape
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.2));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));

    // the scaling by 0 case
    cElement  = GetCElementOf("Fuselage_ETSection1IDElement1");
    newHeight = 0;
    center    = cElement->GetCenter();
    cElement->SetHeight(newHeight);
    EXPECT_NEAR(newHeight, cElement->GetHeight(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
    // rescale the 0 case
    cElement  = GetCElementOf("Fuselage_ETSection1IDElement1");
    newHeight = 7;
    center    = cElement->GetCenter();
    cElement->SetHeight(newHeight);
    EXPECT_NEAR(newHeight, cElement->GetHeight(), 0.01);
    tigl::CTiglPoint newCenter = cElement->GetCenter();
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");

    // case in a other CS (scaling of 0.5 in Y,Z direction)
    cElement    = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    newHeight = 7;
    center    = cElement->GetCenter();
    normal    = cElement->GetNormal();
    cElement->SetHeight(newHeight, FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_NEAR(newHeight * 0.5, cElement->GetHeight(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(), 0.01));

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
}


TEST_F(creatorCTiglSectionElement, SetArea_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    double newArea;
    tigl::CTiglPoint center;

    cElement  = GetCElementOf("D150_Fuselage_CSection1IDElement1");
    newArea = 3;
    center    = cElement->GetCenter();
    cElement->SetArea(newArea);
    EXPECT_NEAR(newArea, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    cElement  = GetCElementOf("Fuselage_ETSection1IDElement1");
    newArea = 7;
    center    = cElement->GetCenter();
    cElement->SetArea(newArea);
    EXPECT_NEAR(newArea, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));


    cElement  = GetCElementOf("Fuselage_ETSection1IDElement1");
    newArea = 0;
    center    = cElement->GetCenter();
    cElement->SetArea(newArea);
    EXPECT_NEAR(newArea, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    newArea = 7;
    center    = cElement->GetCenter();
    cElement->SetArea(newArea);
    EXPECT_NEAR(newArea, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));



    cElement    = GetCElementOf("D150_Fuselage_1Section1IDElement1");
    newArea = 7;
    center    = cElement->GetCenter();
    cElement->SetArea(newArea,FUSELAGE_COORDINATE_SYSTEM);
    EXPECT_NEAR(newArea * 0.5 * 0.5, cElement->GetArea(), 0.01);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(), 0.01));

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
}



TEST_F(creatorCTiglSectionElement, SetTotalTransformation_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    tigl::CTiglTransformation newTotalTransformation, temp;

    cElement  = GetCElementOf("Fuselage_ETSection1IDElement1");
    newTotalTransformation = tigl::CTiglTransformation();

    newTotalTransformation.AddScaling(3,4,1);
    cElement->SetTotalTransformation(newTotalTransformation);
    EXPECT_TRUE(cElement->GetTotalTransformation().IsNear(newTotalTransformation) ) ;

    newTotalTransformation.AddRotationIntrinsicXYZ(10,30,70);
    cElement->SetTotalTransformation(newTotalTransformation);
    EXPECT_TRUE(cElement->GetTotalTransformation().IsNear(newTotalTransformation) ) ;

    newTotalTransformation.AddTranslation(-12,3,4);
    cElement->SetTotalTransformation(newTotalTransformation);
    EXPECT_TRUE(cElement->GetTotalTransformation().IsNear(newTotalTransformation) ) ;


    // shearing from rotation scaling rotation
    newTotalTransformation.SetIdentity();
    newTotalTransformation.AddRotationY(45);
    newTotalTransformation.AddScaling(2,1,1);
    newTotalTransformation.AddRotationY(-18.43);
    cElement->SetTotalTransformation(newTotalTransformation);
    EXPECT_TRUE(cElement->GetTotalTransformation().IsNear(newTotalTransformation));

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");

    // shearing test
    cElement =  GetCElementOf("D150_Fuselage_1Section1IDElement1");
    temp.SetIdentity();
    temp.SetValue(0, 1, 0.5);
    newTotalTransformation = cElement->GetTotalTransformation();
    newTotalTransformation = temp * newTotalTransformation ;

    cElement->SetTotalTransformation(newTotalTransformation);

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
    newTotalTransformation = cElement->GetTotalTransformation();
    newTotalTransformation.AddRotationZ(-20);

    cElement->SetTotalTransformation(newTotalTransformation);

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");

    EXPECT_TRUE(true);



}

TEST_F(creatorCTiglSectionElement, SetSectionElementTransformation_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml");

    tigl::CTiglSectionElement* cElement = nullptr;
    tigl::CTiglTransformation newSectionElementTransformation, res, resE, resS, I;
    I.SetIdentity();

    double tolerance = 0.0001; // tolerance on the pairwise diff of the result matrix and the input matrix

    cElement = GetCElementOf("D150_Fuselage_1Section1IDElement1");

    // identity matrix case

    newSectionElementTransformation.SetIdentity();
    cElement->SetElementAndSectionTransformation(newSectionElementTransformation);
    resE = cElement->GetElementTransformation();
    resS = cElement->GetSectionTransformation();
    res  = resS * resE;
    EXPECT_TRUE(res.IsNear(newSectionElementTransformation, tolerance));
    EXPECT_TRUE(resE.IsNear(I, tolerance)); // we want also that E and S matrix equal I
    EXPECT_TRUE(resS.IsNear(I, tolerance));

    // simple matrix case

    newSectionElementTransformation.SetIdentity();
    newSectionElementTransformation.AddRotationIntrinsicXYZ(20, 30, 40);
    cElement->SetElementAndSectionTransformation(newSectionElementTransformation);
    resE = cElement->GetElementTransformation();
    resS = cElement->GetSectionTransformation();
    res  = resS * resE;
    EXPECT_TRUE(res.IsNear(newSectionElementTransformation, tolerance));
    EXPECT_TRUE(
        resE.IsNear(I, tolerance)); // we want also that E equal I because a proper polar decomposition should be done

    // simple matrix case2

    newSectionElementTransformation.SetIdentity();
    newSectionElementTransformation.AddScaling(1, 2, 3);
    newSectionElementTransformation.AddRotationIntrinsicXYZ(10, 40, 60);
    newSectionElementTransformation.AddTranslation(30, -10, -8);
    cElement->SetElementAndSectionTransformation(newSectionElementTransformation);
    resE = cElement->GetElementTransformation();
    resS = cElement->GetSectionTransformation();
    res  = resS * resE;
    EXPECT_TRUE(res.IsNear(newSectionElementTransformation, tolerance));
    // we want also that E equal I because a proper polar decomposition should be done
    EXPECT_TRUE(resE.IsNear(I, tolerance));

    // shearing matrix case

    newSectionElementTransformation.SetIdentity();
    newSectionElementTransformation.SetValue(0, 1, 0.5);
    cElement->SetElementAndSectionTransformation(newSectionElementTransformation);
    resE = cElement->GetElementTransformation();
    resS = cElement->GetSectionTransformation();
    res  = resS * resE;

    EXPECT_TRUE(res.IsNear(newSectionElementTransformation, tolerance));



    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
}


TEST_F(creatorCTiglSectionElement, Wing_GetChord_MultipleWingsModel)
{
    setVariables("TestData/multiple_wings.xml");
    tigl::CTiglPoint chordP;
    tigl::CTiglPoint wingPosition(-15,-10,0);

    tigl::CTiglWingSectionElement* cElement = nullptr;


    // The chord is define as the segement that goes from the leading edge to the trailing edge
    // (even if the airfoil is curved)
    cElement = GetWingCElementOf("W16_CurAir_Sec1_El1");
    // leading edge of the first element
    chordP = cElement->GetChordPoint(0);
    EXPECT_EQ(chordP, wingPosition +  tigl::CTiglPoint(0,0,0));
    // trailing edge of the first element
    chordP = cElement->GetChordPoint(1);
    EXPECT_EQ(chordP, wingPosition + tigl::CTiglPoint(1,0,0));
    // middle point
    chordP = cElement->GetChordPoint(0.5);
    EXPECT_EQ(chordP, wingPosition + tigl::CTiglPoint(0.5,0,0));

}


TEST_F(creatorCTiglSectionElement, GetRotationAroundNormal_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml");
    tigl::CTiglSectionElement* cElement = nullptr;
    double tolerance = 0.0001;
    double angle = 0;
    double expectedAngle = 0;
    tigl::CTiglPoint normal, center;
    TiglCoordinateSystem refCS = TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM;

    cElement = GetCElementOf("D150_Fuselage_1Section3IDElement1");
    expectedAngle = 0;
    normal = cElement->GetNormal(refCS);
    center = cElement->GetCenter(refCS);
    angle = cElement->GetRotationAroundNormal(refCS);
    EXPECT_NEAR(angle, expectedAngle, tolerance );
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));


    cElement = GetCElementOf("Fuselage_ETSection1IDElement1");
    expectedAngle = 40;
    normal = cElement->GetNormal(refCS);
    center = cElement->GetCenter(refCS);
    angle = cElement->GetRotationAroundNormal(refCS);
    EXPECT_NEAR(angle, expectedAngle, tolerance );
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));


    cElement = GetCElementOf("D150_Fuselage_5Section1IDElement1");
    refCS = TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM;
    expectedAngle = 0;
    normal = cElement->GetNormal(refCS);
    center = cElement->GetCenter(refCS);
    angle = cElement->GetRotationAroundNormal(refCS);
    EXPECT_NEAR(angle, expectedAngle, tolerance );
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));

    refCS = TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM;
    cElement = GetCElementOf("FuselageShearingSection_1Section1IDElement1");
    expectedAngle = 0;
    normal = cElement->GetNormal(refCS);
    center = cElement->GetCenter(refCS);
    angle = cElement->GetRotationAroundNormal(refCS);
    EXPECT_NEAR(angle, expectedAngle, tolerance );
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));

}



TEST_F(creatorCTiglSectionElement, SetNormal_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml");
    tigl::CTiglSectionElement* cElement = nullptr;
    double tolerance = 0.0001;
    double angleN = 0;
    tigl::CTiglPoint newNormal, center;
    TiglCoordinateSystem refCS = TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM;


    cElement = GetCElementOf("D150_Fuselage_1Section3IDElement1");
    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(1,0,0);
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(-1,0,0);
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0,1,0);
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0,-1,0);
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0,0,1);
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0,0,-1);
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0,1,-1);
    newNormal.normalize();
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0.5,1,0);
    newNormal.normalize();
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    cElement = GetCElementOf("FuselageShearingSection_1Section1IDElement1");

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0,0,-1);
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0,1,-1);
    newNormal.normalize();
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));

    center = cElement->GetCenter(refCS);
    angleN = cElement->GetRotationAroundNormal(refCS);
    newNormal = tigl::CTiglPoint(0.5,1,0);
    newNormal.normalize();
    cElement->SetNormal(newNormal, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_NEAR(angleN, cElement->GetRotationAroundNormal(refCS), tolerance );
    EXPECT_TRUE(newNormal.isNear(cElement->GetNormal(refCS)));
}



TEST_F(creatorCTiglSectionElement, SetRotationAroundNoraml_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml");
    tigl::CTiglSectionElement *cElement = nullptr;
    double tolerance = 0.0001;
    double newAngle;
    tigl::CTiglPoint normal, center;
    TiglCoordinateSystem refCS = TiglCoordinateSystem::GLOBAL_COORDINATE_SYSTEM;


    cElement = GetCElementOf("FuselageShearingSection_1Section1IDElement1");

    newAngle = 20;
    center = cElement->GetCenter(refCS);
    normal = cElement->GetNormal(refCS);
    cElement->SetRotationAroundNormal(newAngle);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));
    EXPECT_NEAR(newAngle, cElement->GetRotationAroundNormal(refCS), tolerance);

    newAngle = -20;
    center = cElement->GetCenter(refCS);
    normal = cElement->GetNormal(refCS);
    cElement->SetRotationAroundNormal(newAngle);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));
    EXPECT_NEAR(newAngle + 360, cElement->GetRotationAroundNormal(refCS), tolerance);


    newAngle = 180;
    center = cElement->GetCenter(refCS);
    normal = cElement->GetNormal(refCS);
    cElement->SetRotationAroundNormal(newAngle);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));
    EXPECT_NEAR(newAngle, cElement->GetRotationAroundNormal(refCS), tolerance);


    refCS = TiglCoordinateSystem::FUSELAGE_COORDINATE_SYSTEM;
    newAngle = 45;
    cElement = GetCElementOf("D150_Fuselage_5Section1IDElement1");
    center = cElement->GetCenter(refCS);
    normal = cElement->GetNormal(refCS);
    cElement->SetRotationAroundNormal(newAngle, refCS);
    EXPECT_TRUE(center.isNear(cElement->GetCenter(refCS)));
    EXPECT_TRUE(normal.isNear(cElement->GetNormal(refCS)));
    EXPECT_NEAR(newAngle, cElement->GetRotationAroundNormal(refCS), tolerance);

    saveCurrentConfig("TestData/Output/multiple_fuselages-out.xml");
}