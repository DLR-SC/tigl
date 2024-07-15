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
#include "BRepBuilderAPI_Transform.hxx"
#include <BRepCheck_Analyzer.hxx>
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CNamedShape.h"

class FuselageStandardProfile : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // Test case on standardProfile, mixed profiles: rectangle, rectangle with rounded corners, circle, circle with kinks

        const char* filename = "TestData/simpletest_standard_profile_rectangle_circle_kink.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // Test case on standardProfile rectangle with guide curves

        const char* filename1 = "TestData/simpletest_standard_profile_rectangle_circle_guides.cpacs.xml";
        ReturnCode tixiRet1;
        TiglReturnCode tiglRet1;

        tiglHandle1 = -1;
        tixiHandle1 = -1;

        tixiRet1 = tixiOpenDocument(filename1, &tixiHandle1);
        ASSERT_TRUE (tixiRet1 == SUCCESS);
        tiglRet1 = tiglOpenCPACSConfiguration(tixiHandle1, "", &tiglHandle1);
        ASSERT_TRUE(tiglRet1 == TIGL_SUCCESS);

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

    }

    void SetUp() override {}
    void TearDown() override {}


    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    static TixiDocumentHandle           tixiHandle1;
    static TiglCPACSConfigurationHandle tiglHandle1;

};

TixiDocumentHandle FuselageStandardProfile::tixiHandle = 0;
TiglCPACSConfigurationHandle FuselageStandardProfile::tiglHandle = 0;
TixiDocumentHandle FuselageStandardProfile::tixiHandle1 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfile::tiglHandle1 = 0;



TEST_F(FuselageStandardProfile, BuildFuselageMixedProfilesWithKinks_ValidValues)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CTiglUIDManager& uidmgr = config.GetUIDManager();
    auto wing = uidmgr.GetGeometricComponent("Wing").GetLoft();
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}

TEST_F(FuselageStandardProfile, BuildFuselageMixedProfilesWithGuides_ValidValues)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle1);
    tigl::CTiglUIDManager& uidmgr = config.GetUIDManager();
    auto wing = uidmgr.GetGeometricComponent("Wing").GetLoft();
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}

TEST_F(FuselageStandardProfile, BuildFuselageMixedProfilesWithGuides_InvalidInput)
{
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    //add invalid element
    tixiCreateElementAtIndex(tixiHandle1, "/cpacs/vehicles/profiles/fuselageProfiles", "fuselageProfile", 1);
    tixiCreateElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "invalidType");
    tixiAddTextAttribute(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "uID", "std2");

    // change uid of one segment to invalid profile type
    tixiUpdateTextElement(tixiHandle1, "/cpacs/vehicles/aircraft/model/fuselages/fuselage[1]/sections/section[1]/elements/element[1]/profileUID", "std2");
    tiglOpenCPACSConfiguration(tixiHandle1, "", &tiglHandle1);
    tigl::CCPACSConfiguration& config1         = manager.GetConfiguration(tiglHandle1);

    // fuselage cannot be build with invalid profile
    ASSERT_THROW(config1.GetFuselage(1).GetLoft(),tigl::CTiglError);

    //add  point list profile with 2 elements
    tixiCreateElementAtIndex(tixiHandle1, "/cpacs/vehicles/profiles/fuselageProfiles", "fuselageProfile", 1);
    tixiCreateElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "pointList");
    tixiAddTextAttribute(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]", "uID", "pls");
    tixiCreateElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/pointList", "x");
    tixiAddTextAttribute(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/x", "mapType", "vector");
    tixiAddTextElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]","x", "1.0;0.9");
    tixiCreateElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/pointList", "y");
    tixiAddTextAttribute(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/y", "mapType", "vector");
    tixiAddTextElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]","y", "0.0;0.0");
    tixiCreateElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/pointList", "z");
    tixiAddTextAttribute(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]/z", "mapType", "vector");
    tixiAddTextElement(tixiHandle1,"/cpacs/vehicles/profiles/fuselageProfiles/fuselageProfile[1]","z", "1.0;0.9");

    // change uid of one segment to profile type with only two points in point list
    tixiUpdateTextElement(tixiHandle1, "/cpacs/vehicles/aircraft/model/fuselages/fuselage[1]/sections/section[1]/elements/element[1]/profileUID", "pls");
    tiglOpenCPACSConfiguration(tixiHandle1, "", &tiglHandle1);
    tigl::CCPACSConfiguration& config2         = manager.GetConfiguration(tiglHandle1);
    // fuselage cannot be build with invalid profile
    ASSERT_THROW(config2.GetFuselage(1).GetLoft(),tigl::CTiglError);

}
