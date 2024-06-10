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

class FuselageStandardProfileSuperEllipse : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // Test case on standardProfile, mixed profiles: rectangle, rectangle with rounded corners, circle, circle with kinks

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

TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle = 0;
TixiDocumentHandle FuselageStandardProfileSuperEllipse::tixiHandle1 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfileSuperEllipse::tiglHandle1 = 0;

TEST_F(FuselageStandardProfileSuperEllipse, BuildWireSuperEllipse)
{
    auto wire = BuildWireSuperEllipse(0.25,5.,0.5,3.,2);
    ASSERT_TRUE(wire.Closed());
    auto trafo = gp_Trsf();
    auto vec = gp_Vec(-1.,0.,0.);
    trafo.SetTranslation(vec);
    auto wire2 = BRepBuilderAPI_Transform(wire, trafo).Shape();
    ASSERT_TRUE(wire2.Closed());
    auto loft = CTiglMakeLoft();
    loft.addProfiles(wire);
    loft.addProfiles(wire2);
    ASSERT_TRUE(BRepCheck_Analyzer(loft.Shape()).IsValid());
}


TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesWithGuides_Superellipse)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CTiglUIDManager& uidmgr = config.GetUIDManager();
    auto wing = uidmgr.GetGeometricComponent("Wing").GetLoft();
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}


TEST_F(FuselageStandardProfileSuperEllipse, BuildFuselageMixedProfilesWithKinks_Superellipse)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle1);
    tigl::CTiglUIDManager& uidmgr = config.GetUIDManager();
    auto wing = uidmgr.GetGeometricComponent("Wing").GetLoft();
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
}
