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
#include "tixi.h"
#include "tixicpp.h"

#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CTiglSectionElement.h"
#include "CTiglFuselageSectionElement.h"
#include "CTiglWingSectionElement.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSWingSectionElement.h"


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
        myfile.open("TestData/Output/creatorFuselage-out.xml", std::ios::trunc);
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



TEST_F(creatorFuselage, createSection_MultipleFuselageModel)
{

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    std::vector<std::string>  orderedUIDS;
    std::vector<std::string>  expectedOrderedUIDS;
    tigl::CTiglPoint expectedCenter, currentCenter;
    tigl::CTiglSectionElement * newElement;

    fuselage->CreateNewConnectedElementBetween("D150_Fuselage_1Section1IDElement1","D150_Fuselage_1Section2IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBisElem1"); // new element
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    expectedCenter = tigl::CTiglPoint(0,0,0);
    newElement = GetCElementOf("D150_Fuselage_1Section1IDBisElem1");
    EXPECT_TRUE(expectedCenter.isNear(newElement->GetCenter()));



    fuselage->CreateNewConnectedElementBetween("D150_Fuselage_1Section2IDElement1","D150_Fuselage_1Section3IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDBisElem1"); // new element
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    expectedCenter = tigl::CTiglPoint(1,0,0);
    newElement = GetCElementOf("D150_Fuselage_1Section2IDBisElem1");
    EXPECT_TRUE(expectedCenter.isNear(newElement->GetCenter()));


    fuselage->CreateNewConnectedElementBefore("D150_Fuselage_1Section1IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBeforeElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    expectedCenter = tigl::CTiglPoint(-1,0,0);
    newElement = GetCElementOf("D150_Fuselage_1Section1IDBeforeElem1");
    currentCenter = newElement->GetCenter();
    EXPECT_TRUE(expectedCenter.isNear(currentCenter));



    fuselage->CreateNewConnectedElementAfter("D150_Fuselage_1Section3IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBeforeElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDAfterElem1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    expectedCenter = tigl::CTiglPoint(2,0,0);
    newElement = GetCElementOf("D150_Fuselage_1Section3IDAfterElem1");
    EXPECT_TRUE(expectedCenter.isNear(newElement->GetCenter()));



    fuselage->CreateNewConnectedElementAfter("D150_Fuselage_1Section2IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBeforeElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDBisU1Elem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDAfterElem1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    expectedCenter = tigl::CTiglPoint(0.75,0,0);
    newElement = GetCElementOf("D150_Fuselage_1Section2IDBisU1Elem1");
    EXPECT_TRUE(expectedCenter.isNear(newElement->GetCenter()));




    fuselage->CreateNewConnectedElementBefore("D150_Fuselage_1Section2IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBeforeElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDBisBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDBisU1Elem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section2IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDAfterElem1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    expectedCenter = tigl::CTiglPoint(0.25,0,0);
    newElement = GetCElementOf("D150_Fuselage_1Section1IDBisBisElem1");
    EXPECT_TRUE(expectedCenter.isNear(newElement->GetCenter()));



    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage5");


    fuselage->CreateNewConnectedElementBefore("D150_Fuselage_5Section1IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section1IDBeforeElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }


    fuselage->CreateNewConnectedElementAfter("D150_Fuselage_5Section1IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section1IDBeforeElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section1IDBisElem1");
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section2IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_5Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }


    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageElementTransformation");

    fuselage->CreateNewConnectedElementAfter("Fuselage_ETSection1IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("Fuselage_ETSection1IDElement1");
    expectedOrderedUIDS.push_back("Fuselage_ETSection1IDBisElem1");
    expectedOrderedUIDS.push_back("Fuselage_ETSection2IDElement1");
    expectedOrderedUIDS.push_back("Fuselage_ETSection3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }


    setVariables("TestData/multiple_fuselages.xml", "FuselageShearingSection");

    fuselage->CreateNewConnectedElementAfter("FuselageShearingSection_1Section1IDElement1");
    saveInOutputFile();

    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section1IDElement1");
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section1IDBisElem1");
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section2IDElement1");
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }


}


TEST_F(creatorFuselage, deleteSection_multipleFuselgaesModel)
{
    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    std::vector<std::string>  orderedUIDS;
    std::vector<std::string>  expectedOrderedUIDS;
    int segmentsCount;


    segmentsCount = fuselage->GetSegmentCount();
    fuselage->DeleteConnectedElement("D150_Fuselage_1Section2IDElement1");
    EXPECT_EQ(segmentsCount-1, fuselage->GetSegmentCount());
    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section1IDElement1");
    expectedOrderedUIDS.push_back("D150_Fuselage_1Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    saveInOutputFile();


    setVariables("TestData/multiple_fuselages.xml", "FuselageShearingSection");

    segmentsCount = fuselage->GetSegmentCount();
    fuselage->DeleteConnectedElement("FuselageShearingSection_1Section1IDElement1");
    EXPECT_EQ(segmentsCount-1, fuselage->GetSegmentCount());
    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section2IDElement1");
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section3IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    saveInOutputFile();


    setVariables("TestData/multiple_fuselages.xml", "FuselageShearingSection");

    segmentsCount = fuselage->GetSegmentCount();
    fuselage->DeleteConnectedElement("FuselageShearingSection_1Section3IDElement1");
    EXPECT_EQ(segmentsCount-1, fuselage->GetSegmentCount());
    orderedUIDS = fuselage->GetSegments().GetElementUIDsInOrder();
    expectedOrderedUIDS.clear();
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section1IDElement1");
    expectedOrderedUIDS.push_back("FuselageShearingSection_1Section2IDElement1");
    for ( int i = 0 ; i < expectedOrderedUIDS.size() ; i++) {
        EXPECT_EQ(expectedOrderedUIDS[i], orderedUIDS[i]);
    }
    saveInOutputFile();

}


TEST_F(creatorFuselage, getAllProfiles_multiplesFuselageModel)
{
    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
    std::vector<std::string> expectedProfiles;
    std::vector<std::string> currentProfile;

    expectedProfiles.push_back("fuselageCircleProfileuID");
    expectedProfiles.push_back("fuselageTriangleProfileuID");
    currentProfile = fuselage->GetAllUsedProfiles();
    ASSERT_EQ(currentProfile.size(), expectedProfiles.size());
    for(int i = 0; i < currentProfile.size(); i ++ ){
        EXPECT_EQ(currentProfile.at(i), expectedProfiles.at(i));
    }

}


TEST_F(creatorFuselage, setAllProfiles_multiplesFuselageModel)
{

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
    std::vector<std::string> expectedProfiles;
    std::vector<std::string> currentProfile;

    saveInOutputFile();

    fuselage->SetAllProfiles("fuselageCircleProfileuID");
    expectedProfiles.push_back("fuselageCircleProfileuID");
    currentProfile = fuselage->GetAllUsedProfiles();
    ASSERT_EQ(currentProfile.size(), expectedProfiles.size());
    for(int i = 0; i < currentProfile.size(); i ++ ){
        EXPECT_EQ(currentProfile.at(i), expectedProfiles.at(i));
    }

    saveInOutputFile();
}