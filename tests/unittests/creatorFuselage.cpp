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

    std::map<std::string, double> GetAllCircumferencesOfFuselage()
    {
        std::map<std::string, double> circumferences;
        std::map<std::string, tigl::CTiglFuselageSectionElement*> connections = fuselage->GetSections().GetCTiglElements();
        std::map<std::string, tigl::CTiglFuselageSectionElement*>::iterator it;
        for (it = connections.begin(); it != connections.end(); it++) {
            circumferences[it->first] = (it->second)->GetCircumferenceOfProfile();
        }
        return circumferences;
    }
};



TEST_F(creatorFuselage, getNoiseUID_SimpleModel)
{
    setVariables("TestData/simpletest.cpacs.xml", 1);

    std::string uid = fuselage->GetNoiseUID();
    ASSERT_EQ("D150_Fuselage_1Section1IDElement1", uid);
}

TEST_F(creatorFuselage, getNoiseUID_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml", "FuselageUnconventionalOrdering");
    // simple reordering ( if the segment a are swap ) is supported : )
    std::string uid = fuselage->GetNoiseUID();
    ASSERT_EQ("D150_Fuselage_2Section1IDElement1", uid);

    // todo "complex" reordering when FromUID to toUID are also swap is not supported, may not be a priority
    //setVariables("TestData/multiple_fuselages.xml", "FuselageUnconventionalOrderingExtrem");

    //uid = fuselage->GetNoiseUID();
    //ASSERT_EQ("D150_Fuselage_2aSection1IDElement1", uid);
}


TEST_F(creatorFuselage, getTailUID_SimpleModel)
{
    setVariables("TestData/simpletest.cpacs.xml", 1);

    std::string uid = fuselage->GetTailUID();
    ASSERT_EQ("D150_Fuselage_1Section3IDElement1", uid);
}


TEST_F(creatorFuselage, getTailUID_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml", "FuselageUnconventionalOrdering");

    std::string uid = fuselage->GetTailUID();
    ASSERT_EQ("D150_Fuselage_2Section3IDElement1", uid);
}

TEST_F(creatorFuselage, getConnectionElementUIDs_SimpleModel)
{
    setVariables("TestData/simpletest.cpacs.xml", 1);

    std::vector<std::string> elementUIDs = fuselage->GetConnectionElementUIDs();
    std::vector<std::string> expectedElementUIDs;
    expectedElementUIDs.push_back("D150_Fuselage_1Section1IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_1Section2IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_1Section3IDElement1");

    EXPECT_EQ(expectedElementUIDs.size(), elementUIDs.size());

    for (int i = 0; i < expectedElementUIDs.size(); i++) {
        EXPECT_EQ(expectedElementUIDs[i], elementUIDs[i]);
    }
}

TEST_F(creatorFuselage, getConnectionElementUIDs_MultipleFuselagesModel)
{

    setVariables("TestData/multiple_fuselages.xml", "FuselageUnconventionalOrdering");

    std::vector<std::string> elementUIDs = fuselage->GetConnectionElementUIDs();
    std::vector<std::string> expectedElementUIDs;
    expectedElementUIDs.push_back("D150_Fuselage_2Section1IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_2Section2IDElement1");
    expectedElementUIDs.push_back("D150_Fuselage_2Section3IDElement1");

    EXPECT_EQ(expectedElementUIDs.size(), elementUIDs.size());

    for (int i = 0; i < expectedElementUIDs.size(); i++) {
        EXPECT_EQ(expectedElementUIDs[i], elementUIDs[i]);
    }
}

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




TEST_F(creatorFuselage, getMaximalCircumference_MultipleFuselagesModel) {


    double circumference =-1;

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
    circumference = fuselage->GetMaximalCircumference();
    EXPECT_NEAR( circumference, 19 , 1 ); // the interpolation can gives significant difference from the perfect circle


    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");
    circumference = fuselage->GetMaximalCircumference();
    EXPECT_NEAR( circumference, 1 * M_PI , 0.1 ); // the interpolation can gives significant difference from the perfect circle


    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageStdP");
    circumference = fuselage->GetMaximalCircumference();
    EXPECT_NEAR( circumference, 2 * M_PI , 0.1 ); // the interpolation can gives significant difference from the perfect circle


}




TEST_F(creatorFuselage, getMaximalCircumferenceOfElements_CrmWingModel) // also to check th e preformance ( should run in less then 0.5s)
{
    setVariables("TestData/crm.xml", 1);

    double c                  = fuselage->GetMaximalCircumference();
    ASSERT_NEAR(20, c, 2);
}




TEST_F(creatorFuselage, getMaximalCircumferenceBetween_MultipleFuselagesModel) {

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
    double circumference = -1 ;

    circumference = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_CSection1IDElement1", "D150_Fuselage_CSection1IDElement1");
    EXPECT_NEAR(circumference, 2*M_PI, 0.5 );
    circumference = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_CSection2IDElement1", "D150_Fuselage_CSection2IDElement1");
    EXPECT_NEAR(circumference, 19, 0.5 );
    circumference = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_CSection3IDElement1", "D150_Fuselage_CSection3IDElement1");
    EXPECT_NEAR(circumference, 3.23, 0.5 );

    circumference = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_CSection1IDElement1", "D150_Fuselage_CSection2IDElement1");
    EXPECT_NEAR(circumference, 19, 0.5 );

    circumference = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_CSection1IDElement1", "D150_Fuselage_CSection3IDElement1");
    EXPECT_NEAR(circumference, 19, 0.5 );

    // the function return -1 if it does not find the uid in
    circumference = fuselage->GetMaximalCircumferenceBetween("fsd", "sf");
    EXPECT_EQ(-1, circumference);
}






TEST_F(creatorFuselage, setMaximalCircumference_MultipleFuselagesModel) {


    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");

    std::map<std::string, double> circumferencesBefore,  circumferencesAfter ;
    std::map<std::string, double>::iterator it;
    double scale, maxCircBerfore, maxCircAfter;

    circumferencesBefore = GetAllCircumferencesOfFuselage();
    scale = 3.3;
    maxCircBerfore = fuselage->GetMaximalCircumference();
    fuselage->SetMaximalCircumference(scale * maxCircBerfore);
    circumferencesAfter = GetAllCircumferencesOfFuselage();
    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
    }


    setVariables("TestData/multiple_fuselages.xml", "FuselageUnconventionalOrdering");
    circumferencesBefore = GetAllCircumferencesOfFuselage();
    scale = 0.3;
    maxCircBerfore = fuselage->GetMaximalCircumference();
    fuselage->SetMaximalCircumference(scale * maxCircBerfore);
    circumferencesAfter = GetAllCircumferencesOfFuselage();
    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
    }



    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselageCircumference");
    circumferencesBefore = GetAllCircumferencesOfFuselage();
    scale = 0.9;
    maxCircBerfore = fuselage->GetMaximalCircumference();
    fuselage->SetMaximalCircumference(scale * maxCircBerfore);
    circumferencesAfter = GetAllCircumferencesOfFuselage();
    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
    }



    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage3");
    circumferencesBefore = GetAllCircumferencesOfFuselage();
    scale = 4;
    maxCircBerfore = fuselage->GetMaximalCircumference();
    fuselage->SetMaximalCircumference(scale * maxCircBerfore);
    circumferencesAfter = GetAllCircumferencesOfFuselage();
    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
    }
    scale = 1.0/4.0;
    maxCircBerfore = fuselage->GetMaximalCircumference();
    fuselage->SetMaximalCircumference(scale * maxCircBerfore);
    circumferencesAfter = GetAllCircumferencesOfFuselage();
    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
        EXPECT_NEAR(circumferencesAfter[it->first], it->second, 0.0001 );
    }


    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage4");
    circumferencesBefore = GetAllCircumferencesOfFuselage();
    scale = 4;
    maxCircBerfore = fuselage->GetMaximalCircumference();
    fuselage->SetMaximalCircumference(scale * maxCircBerfore);
    circumferencesAfter = GetAllCircumferencesOfFuselage();
    for(it = circumferencesBefore.begin(); it != circumferencesBefore.end(); it++){
        EXPECT_NEAR(circumferencesAfter[it->first], it->second * scale, 0.0001 );
    }

}


TEST_F(creatorFuselage, SetMaximalCircumferenceBetween_MultipleFuselagesModel) {

    setVariables("TestData/multiple_fuselages.xml", "SimpleFuselage");

    std::map<std::string, double> circumferencesBefore,  circumferencesAfter ;
    std::map<std::string, double>::iterator it;
    double scale, maxCircBerfore, maxCircAfter;


    fuselage->SetMaximalCircumferenceBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section1IDElement1", 5.1);
    maxCircAfter = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_1Section1IDElement1", "D150_Fuselage_1Section1IDElement1");
    EXPECT_NEAR(maxCircAfter, 5.1, 0.001);


    fuselage->SetMaximalCircumferenceBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section2IDElement1", 5.1);
    maxCircAfter = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section2IDElement1");
    EXPECT_NEAR(maxCircAfter, 5.1, 0.001);


    fuselage->SetMaximalCircumferenceBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section3IDElement1", 9.1);
    maxCircAfter = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_1Section2IDElement1", "D150_Fuselage_1Section3IDElement1");
    EXPECT_NEAR(maxCircAfter, 9.1, 0.001);
    maxCircAfter = fuselage->GetMaximalCircumferenceBetween("D150_Fuselage_1Section3IDElement1", "D150_Fuselage_1Section3IDElement1");
    EXPECT_NEAR(maxCircAfter, M_PI* 9.1/5.1, 0.1);



}


