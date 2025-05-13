/*
* Copyright (C) 2022 German Aerospace Center
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

#include "CTiglMakeLoft.h"
#include "test.h"
#include "tigl.h"
#include "Debugging.h"
#include "tiglcommonfunctions.h"
#include "CCPACSFuselage.h"
#include "BRepBuilderAPI_Transform.hxx"
#include <BRepCheck_Analyzer.hxx>
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CNamedShape.h"

class FuselageStandardProfileSuperEllipse : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // Test case on standardProfile, mixed profiles: rectangle, rectangle with rounded corners, circle, superellipse, guidecurves

        const char* filename = "TestData/simpletest_standard_profile_superellipse_guides.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // Test case on standardProfile rectangle with guide curves

        const char* filename1 = "TestData/simpletest_standard_profile_superellipse_kink.cpacs.xml";
        ReturnCode tixiRet1;
        TiglReturnCode tiglRet1;

        tiglHandle1 = -1;
        tixiHandle1 = -1;

        tixiRet1 = tixiOpenDocument(filename1, &tixiHandle1);
        ASSERT_TRUE (tixiRet1 == SUCCESS);
        tiglRet1 = tiglOpenCPACSConfiguration(tixiHandle1, "", &tiglHandle1);
        ASSERT_TRUE(tiglRet1 == TIGL_SUCCESS);

        // Test case on standardProfile, invalid elements

        ReturnCode tixiRet2;
        TiglReturnCode tiglRet2;

        tiglHandle2 = -1;
        tixiHandle2 = -1;

        tixiRet2 = tixiOpenDocument(filename1, &tixiHandle2);
        ASSERT_TRUE (tixiRet2 == SUCCESS);
        tiglRet2 = tiglOpenCPACSConfiguration(tixiHandle2, "", &tiglHandle2);
        ASSERT_TRUE(tiglRet2 == TIGL_SUCCESS);


    }

    static void TearDownTestCase()
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle) == SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;

        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle1) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle1) == SUCCESS);
        tiglHandle1 = -1;
        tixiHandle1 = -1;

        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglHandle2) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiHandle2) == SUCCESS);
        tiglHandle2 = -1;
        tixiHandle2 = -1;

    }

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    static TixiDocumentHandle           tixiHandle1;
    static TiglCPACSConfigurationHandle tiglHandle1;


    static TixiDocumentHandle           tixiHandle2;
    static TiglCPACSConfigurationHandle tiglHandle2;
};

TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle = 0;
TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle1 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle1 = 0;
TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle2 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle2 = 0;


TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesWithGuides_Superellipse)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}


TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesWithKinks_Superellipse)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle1);
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}

TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesInvalidInput)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    //add invalid element
    tixiCreateElementAtIndex(tixiHandle2, "/cpacs/vehicles/profiles/fuselageProfiles", "fuselageProfile", 1);
    tixiCreateElement(tixiHandle2,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "invalidType");
    tixiAddTextAttribute(tixiHandle2,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "uID", "std2");

    // change uid of one segment to invalid profile type
    tixiUpdateTextElement(tixiHandle2, "/cpacs/vehicles/aircraft/model/fuselages/fuselage[1]/sections/section[1]/elements/element[1]/profileUID", "std2");
    tiglOpenCPACSConfiguration(tixiHandle2, "", &tiglHandle2);
    tigl::CCPACSConfiguration& config1         = manager.GetConfiguration(tiglHandle2);

    // fuselage cannot be build with invalid profile
    ASSERT_THROW(config1.GetFuselage(1).GetLoft(),tigl::CTiglError);
}
