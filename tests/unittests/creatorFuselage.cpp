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

TEST_F(creatorFuselage, getLengthBetween_SimpleModel)
{
    setVariables("TestData/simpletest.cpacs.xml", 1);

    double length =
        fuselage->GetLengthBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section2IDElement1");
    ASSERT_NEAR(1, length, 0.0001);
}

TEST_F(creatorFuselage, getLengthBetween_MultipleFuselagesModel)
{
    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    double r = -1;

    r = fuselage->GetLengthBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section2IDElement1");
    EXPECT_DOUBLE_EQ(r, 1);

    r = fuselage->GetLengthBetween("D150_Fuselage_1Section3IDElement1", "D150_Fuselage_1Section2IDElement1");
    EXPECT_DOUBLE_EQ(r, 1);

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage4");
    r = fuselage->GetLengthBetween("D150_Fuselage_4Section1IDElement1", "D150_Fuselage_4Section3IDElement1");
    EXPECT_DOUBLE_EQ(r, 4);

    // invalid input // TODO error management
    //EXPECT_THROW(fuselage2.GetLengthBetween("D150_Fuselage_4Section1IDElement1", "D150_Fuselage_4Section3IDElement1fas"), tigl::CTiglError  );
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

TEST_F(creatorFuselage, setLengthBetween_MultipleFuselagesModel)
{

    double newPartialL, oldPartialL, globalL, oldGlobalL, r;

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");

    newPartialL = 3;
    fuselage->SetLengthBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section2IDElement1", newPartialL);
    r = fuselage->GetLengthBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section2IDElement1");
    EXPECT_NEAR(r, newPartialL, 0.0001);

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage4");
    newPartialL = 5;
    fuselage->SetLengthBetween("D150_Fuselage_4Section2IDElement1", "D150_Fuselage_4Section3IDElement1", newPartialL);
    r = fuselage->GetLengthBetween("D150_Fuselage_4Section2IDElement1", "D150_Fuselage_4Section3IDElement1");
    EXPECT_NEAR(r, newPartialL, 0.0001);

    ASSERT_EQ(SUCCESS, tixiSaveDocument(tixiHandle, "TestData/Output/multiple-fuselages-out.cpacs.xml"));
}
//
//
//
//
//TEST_F(creatorFuselage, GetCircumferenceOfElements_MultipleFuselagesModel) {
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
//
//    std::map<std::string, double> circumferences;
//
//    circumferences = fuselage->GetElementCircumferences();
//
//    EXPECT_EQ(circumferences.size(), 3);
//    EXPECT_NEAR(circumferences["D150_Fuselage_CSection1IDElement1"], 2 * M_PI, 0.5  ); // the interpolation can gives significant difference from the perfect circle
//    EXPECT_NEAR(circumferences["D150_Fuselage_CSection2IDElement1"], 19, 1 );
//    EXPECT_NEAR(circumferences["D150_Fuselage_CSection3IDElement1"], 3.23, 1 );
//
//}
//
//
//
//TEST_F(creatorFuselage, getMaximalCircumferenceOfElements_MultipleFuselagesModel) {
//
//
//    double circumference =-1;
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
//    circumference = fuselage->GetMaximalCircumferenceOfElements();
//    EXPECT_NEAR( circumference, 19 , 1 ); // the interpolation can gives significant difference from the perfect circle
//
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
//    circumference = fuselage->GetMaximalCircumferenceOfElements();
//    EXPECT_NEAR( circumference, 1 * M_PI , 0.1 ); // the interpolation can gives significant difference from the perfect circle
//
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageStdP");
//    circumference = fuselage->GetMaximalCircumferenceOfElements();
//    EXPECT_NEAR( circumference, 2 * M_PI , 0.1 ); // the interpolation can gives significant difference from the perfect circle
//
//
//}
//
//
//
//
//TEST_F(creatorFuselage, getMaximalCircumferenceOfElements_CrmWingModel) // also to check th e preformance ( should run in less then 0.5s)
//{
//    setVariables("TestData/crm.xml", 1);
//
//    double c                  = fuselage->GetMaximalCircumferenceOfElements();
//    ASSERT_NEAR(20, c, 2);
//}
//
//
//
//
//TEST_F(creatorFuselage, getMaximalCircumferenceOfElementsBetween_MultipleFuselagesModel) {
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
//    double circumference = -1 ;
//
//    circumference = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_CSection1IDElement1", "D150_Fuselage_CSection1IDElement1");
//    EXPECT_NEAR(circumference, 2*M_PI, 0.5 );
//    circumference = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_CSection2IDElement1", "D150_Fuselage_CSection2IDElement1");
//    EXPECT_NEAR(circumference, 19, 0.5 );
//    circumference = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_CSection3IDElement1", "D150_Fuselage_CSection3IDElement1");
//    EXPECT_NEAR(circumference, 3.23, 0.5 );
//
//    circumference = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_CSection1IDElement1", "D150_Fuselage_CSection2IDElement1");
//    EXPECT_NEAR(circumference, 19, 0.5 );
//
//    circumference = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_CSection1IDElement1", "D150_Fuselage_CSection3IDElement1");
//    EXPECT_NEAR(circumference, 19, 0.5 );
//
//    EXPECT_THROW(fuselage->GetMaximalCircumferenceOfElementsBetween("fsd", "sf"), tigl::CTiglError );
//}
//
//
//
//
//
//TEST_F(creatorFuselage, setMaximalCircumferenceOfElements_MultipleFuselagesModel) {
//
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
//
//    std::map<std::string, double> circumferencesBefore,  circumferencesAfter ;
//    std::map<std::string, double>::iterator it;
//    double scale, maxCircBerfore, maxCircAfter;
//
//    circumferencesBefore = fuselage->GetElementCircumferences();
//    scale = 3.3;
//    maxCircBerfore = fuselage->GetMaximalCircumferenceOfElements();
//    fuselage->SetMaximalCircumferenceOfElements(scale * maxCircBerfore);
//    circumferencesAfter = fuselage->GetElementCircumferences();
//    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
//        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
//    }
//
//
//    setVariables("TestData/multiple_fuselages.xml", "FuselageUnconventionalOrdering");
//    circumferencesBefore = fuselage->GetElementCircumferences();
//    scale = 0.3;
//    maxCircBerfore = fuselage->GetMaximalCircumferenceOfElements();
//    fuselage->SetMaximalCircumferenceOfElements(scale * maxCircBerfore);
//    circumferencesAfter = fuselage->GetElementCircumferences();
//    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
//        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
//    }
//
//
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
//    circumferencesBefore = fuselage->GetElementCircumferences();
//    scale = 0.9;
//    maxCircBerfore = fuselage->GetMaximalCircumferenceOfElements();
//    fuselage->SetMaximalCircumferenceOfElements(scale * maxCircBerfore);
//    circumferencesAfter = fuselage->GetElementCircumferences();
//    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
//        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
//    }
//
//
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage3");
//    circumferencesBefore = fuselage->GetElementCircumferences();
//    scale = 4;
//    maxCircBerfore = fuselage->GetMaximalCircumferenceOfElements();
//    fuselage->SetMaximalCircumferenceOfElements(scale * maxCircBerfore);
//    circumferencesAfter = fuselage->GetElementCircumferences();
//    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
//        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
//    }
//    scale = 1.0/4.0;
//    maxCircBerfore = fuselage->GetMaximalCircumferenceOfElements();
//    fuselage->SetMaximalCircumferenceOfElements(scale * maxCircBerfore);
//    circumferencesAfter = fuselage->GetElementCircumferences();
//    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
//        EXPECT_NEAR(circumferencesAfter[it->first], it->second, 0.0001 );
//    }
//
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage4");
//    circumferencesBefore = fuselage->GetElementCircumferences();
//    scale = 4;
//    maxCircBerfore = fuselage->GetMaximalCircumferenceOfElements();
//    fuselage->SetMaximalCircumferenceOfElements(scale * maxCircBerfore);
//    circumferencesAfter = fuselage->GetElementCircumferences();
//    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
//        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
//    }
//
//}
//
//
//TEST_F(creatorFuselage, setMaximalCircumferenceOfElementsBetween_MultipleFuselagesModel) {
//
//    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
//
//    std::map<std::string, double> circumferencesBefore,  circumferencesAfter ;
//    std::map<std::string, double>::iterator it;
//    double scale, maxCircBerfore, maxCircAfter;
//
//
//    fuselage->SetMaximalCircumferenceOfElementsBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section1IDElement1", 5.1);
//    maxCircAfter = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section1IDElement1");
//    EXPECT_NEAR(maxCircAfter, 5.1, 0.001);
//
//
//    fuselage->SetMaximalCircumferenceOfElementsBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section2IDElement1", 5.1);
//    maxCircAfter = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section2IDElement1");
//    EXPECT_NEAR(maxCircAfter, 5.1, 0.001);
//
//
//    fuselage->SetMaximalCircumferenceOfElementsBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section3IDElement1", 9.1);
//    maxCircAfter = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section3IDElement1");
//    EXPECT_NEAR(maxCircAfter, 9.1, 0.001);
//    maxCircAfter = fuselage->GetMaximalCircumferenceOfElementsBetween("D150_Fuselage_1Section3IDElement1", "D150_Fuselage_1Section3IDElement1");
//    EXPECT_NEAR(maxCircAfter, M_PI* 9.1/5.1, 0.1);
//
//
//
//}
//
//
