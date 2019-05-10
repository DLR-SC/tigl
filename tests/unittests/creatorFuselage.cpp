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

#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CTiglFuselageSectionElement.h"

#include <string.h>

class creatorFuselage : public ::testing::Test
{

protected:
    std::string filename = "";

    TiglCPACSConfigurationHandle tiglHandle = -1;
    TixiDocumentHandle tixiHandle           = -1;

    tigl::CCPACSConfigurationManager* manager = nullptr;
    tigl::CCPACSConfiguration* config         = nullptr;
    tigl::CCPACSFuselage* fuselage            = nullptr;

    void setVariables(std::string inFilename, int inFuselageIdx)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager  = &(tigl::CCPACSConfigurationManager::GetInstance());
        config   = &(manager->GetConfiguration(tiglHandle));
        fuselage = &(config->GetFuselage(inFuselageIdx));
    }

    void setVariables(std::string inFilename, std::string inFuselageUID)
    {
        unsetVariables();
        filename = inFilename;
        ASSERT_EQ(SUCCESS, tixiOpenDocument(filename.c_str(), &tixiHandle));
        ASSERT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
        manager                          = &(tigl::CCPACSConfigurationManager::GetInstance());
        config                           = &(manager->GetConfiguration(tiglHandle));
        tigl::CCPACSFuselages& fuselages = config->GetFuselages();
        fuselage                         = &(fuselages.GetFuselage(inFuselageUID));
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
    }

    void TearDown()
    {
        unsetVariables();
    }
};




TEST_F(creatorFuselage, getLength_SimpleModel)
{
    setVariables("TestData/simpletest.cpacs.xml", 1);

    double length = fuselage->GetLength();
    ASSERT_NEAR(2, length, 0.0001);
}

TEST_F(creatorFuselage, getLength_BoxWingModel)
{
    setVariables("TestData/boxWing.xml", 1);

    double length = fuselage->GetLength();
    ASSERT_NEAR(37, length, 2);
}

TEST_F(creatorFuselage, getLength_CrmWingModel)
{
    setVariables("TestData/crm.xml", 1);

    double length = fuselage->GetLength();
    ASSERT_NEAR(61, length, 2);
}


TEST_F(creatorFuselage, setLength_SimpleModel)
{
    setVariables("TestData/simpletest.cpacs.xml", 1);

    double lengthBefore, lengthAfter;

    lengthBefore = fuselage->GetLength();
    EXPECT_NEAR(2, lengthBefore, 0.0001);
    fuselage->SetLength(5);
    lengthAfter = fuselage->GetLength();
    EXPECT_NEAR(5, lengthAfter, 0.0001);
    fuselage->SetLength(3.2);
    lengthAfter = fuselage->GetLength();
    EXPECT_NEAR(3.2, lengthAfter, 0.0001);
}


TEST_F(creatorFuselage, setLength_ZeroScalingCase)
{
    // TODO manage this case
//    double newLength, length;
//
//    setVariables("TestData/D150_v30.xml", "D150Fuselage1ID");
//
//    newLength = 30;
//    fuselage->SetLength(newLength);
//    length = fuselage->GetLength();
//    EXPECT_NEAR(length, newLength, 0.0001);
//
//    config->WriteCPACS(config->GetUID());
//    ASSERT_EQ(SUCCESS, tixiSaveDocument(tixiHandle, "TestData/Output/D150_v30-out.xml"));
//
//
//    setVariables("TestData/multiple_fuselages.xml", "ZeroScalingFuselage");
//
//    newLength = 30;
//    fuselage->SetLength(newLength);
//    length = fuselage->GetLength();
//    EXPECT_NEAR(length, newLength, 0.0001);
//
//    config->WriteCPACS(config->GetUID());
//    ASSERT_EQ(SUCCESS, tixiSaveDocument(tixiHandle, "TestData/Output/multiple_fuselages-out.xml"));
}



TEST_F(creatorFuselage, getDimension_MultipleFuselagesModel) { // height and width

    double height, width;
    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");

    height = fuselage->GetMaximalHeight();
    EXPECT_NEAR(height, 3.939, 0.1 );
    width = fuselage->GetMaximalWidth();
    EXPECT_NEAR(width, 7.51754, 0.1 );

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    height = fuselage->GetMaximalHeight();
    EXPECT_NEAR(height, 1, 0.01 );
    width = fuselage->GetMaximalWidth();
    EXPECT_NEAR(width, 1, 0.01 );

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage5");
    height = fuselage->GetMaximalHeight();
    EXPECT_NEAR(height, 1, 0.01 );
    width = fuselage->GetMaximalWidth();
    EXPECT_NEAR(width, 1, 0.01 );

}


TEST_F(creatorFuselage, setDimensionBetween_MultipleFuselagesModel) {

    double height, width,newHeight, newWidth;
    double tolerance = 0.1; // Get height and GetWidth are not really precise by the fact that they use bounding boxes

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    newHeight = 3;
    fuselage->SetMaxHeight(newHeight);
    height = fuselage->GetMaximalHeight();
    width = fuselage->GetMaximalWidth();
    EXPECT_NEAR( height, newHeight, tolerance);
    EXPECT_NEAR(width, fuselage->GetMaximalWidth(), tolerance);  // check that width was not affected by the height


    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage5");
    newWidth = 3;
    fuselage->SetMaxWidth(newWidth);
    width = fuselage->GetMaximalWidth();
    height = fuselage->GetMaximalHeight();
    EXPECT_NEAR( width, newWidth, tolerance);
    EXPECT_NEAR(height, fuselage->GetMaximalHeight(), tolerance);

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
    newHeight = 3;
    fuselage->SetMaxHeight(newHeight);
    height = fuselage->GetMaximalHeight();
    width = fuselage->GetMaximalWidth();
    EXPECT_NEAR( height, newHeight, tolerance);
    EXPECT_NEAR(width, fuselage->GetMaximalWidth(), tolerance);  // check that width was not affected by the height

    // todo for the moment this operation is not working perfectly because we can not decompose the matrix properly
    // -> can be resolved if use element + section setter 
//    newWidth = 0.5;
//    fuselage->SetMaxWidth(newWidth);
//    width = fuselage->GetMaximalWidth();
//    height = fuselage->GetMaximalHeight();
//    EXPECT_NEAR( width, newWidth, tolerance);
//    EXPECT_NEAR(height, fuselage->GetMaximalHeight(), tolerance);

}

TEST_F(creatorFuselage, setNoseCenter_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    tigl::CTiglPoint oldCenter, newCenter, resultCenter;

    oldCenter = fuselage->GetNoseCenter();
    EXPECT_TRUE(oldCenter.isNear(tigl::CTiglPoint(-0.5, 0, 0), 0.0001));

    newCenter = tigl::CTiglPoint(13, 5.5, 2);
    fuselage->SetNoseCenter(newCenter);
    resultCenter = fuselage->GetNoseCenter();
    EXPECT_TRUE(resultCenter.isNear(newCenter, 0.0001));
}

TEST_F(creatorFuselage, getRotation_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    tigl::CTiglPoint rot;

    rot = fuselage->GetRotation();
    EXPECT_TRUE(rot.isNear(tigl::CTiglPoint(0, 0, 0), 0.0001));

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage4");
    rot = fuselage->GetRotation();
    EXPECT_TRUE(rot.isNear(tigl::CTiglPoint(0, 0, 40), 0.0001));

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage5");
    rot = fuselage->GetRotation();
    EXPECT_TRUE(rot.isNear(tigl::CTiglPoint(10, 20, 40), 0.0001));

}


TEST_F(creatorFuselage, setRotation_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    tigl::CTiglPoint newRot, resRot;

    newRot = tigl::CTiglPoint(0,10,0);
    fuselage->SetRotation(newRot);
    resRot = fuselage->GetRotation();
    EXPECT_TRUE(resRot.isNear(newRot, 0.0001));

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage4");
    newRot = tigl::CTiglPoint(-20,10,80);
    fuselage->SetRotation(newRot);
    resRot = fuselage->GetRotation();
    EXPECT_TRUE(resRot.isNear(newRot, 0.0001));

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage5");
    newRot = tigl::CTiglPoint(0,0,0);
    fuselage->SetRotation(newRot);
    resRot = fuselage->GetRotation();
    EXPECT_TRUE(resRot.isNear(newRot, 0.0001));
}