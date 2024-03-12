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
        // Test case on standardProfile rectangle, no rounded corners

        const char* filename = "TestData/simpletest_standard_profile_rectangle.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        // Test case on standardProfile rectangle, no rounded corners, mixed profiles, guidecurves

        const char* filename1 = "TestData/simpletest-rectangle-circle-kink-profile-with-guides.cpacs.xml";
        ReturnCode tixiRet1;
        TiglReturnCode tiglRet1;

        tiglHandle1 = -1;
        tixiHandle1 = -1;

        tixiRet1 = tixiOpenDocument(filename1, &tixiHandle1);
        ASSERT_TRUE (tixiRet1 == SUCCESS);
        tiglRet1 = tiglOpenCPACSConfiguration(tixiHandle1, "", &tiglHandle1);
        ASSERT_TRUE(tiglRet1 == TIGL_SUCCESS);

        // Test case on standardProfile rectangle with rounded corners

        const char* filename2 = "TestData/simpletest_standard_profile_rounded_rectangle.xml";
        ReturnCode tixiRet2;
        TiglReturnCode tiglRet2;

        tiglHandle2 = -1;
        tixiHandle2 = -1;

        tixiRet2 = tixiOpenDocument(filename2, &tixiHandle2);
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

    static TixiDocumentHandle       	tixiHandle2;
    static TiglCPACSConfigurationHandle tiglHandle2;
};

TixiDocumentHandle FuselageStandardProfile::tixiHandle = 0;
TiglCPACSConfigurationHandle FuselageStandardProfile::tiglHandle = 0;
TixiDocumentHandle FuselageStandardProfile::tixiHandle1 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfile::tiglHandle1 = 0;
TixiDocumentHandle FuselageStandardProfile::tixiHandle2 = 0;
TiglCPACSConfigurationHandle FuselageStandardProfile::tiglHandle2 = 0;

TEST_F(FuselageStandardProfile, BuildWireRectangle_CornerRadiusZero)
{
    auto wire = BuildWireRectangle(1, 0.);
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


TEST_F(FuselageStandardProfile, BuildWireRectangle_CornerRadiusOK)
{
    auto wire = BuildWireRectangle(0.5, 0.14);
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

TEST_F(FuselageStandardProfile, BuildWireRectangle_CornerRadius_Negative)
{
    auto wire = BuildWireRectangle(0.5, -0.14);
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

TEST_F(FuselageStandardProfile, BuildWireRectangle_CornerRadius_TooLargeNumber)
{
    auto wire = BuildWireRectangle(0.5, 1);
    ASSERT_TRUE(wire.Closed());
    auto trafo = gp_Trsf();
    auto vec = gp_Vec(-1.,0.,0.);
    trafo.SetTranslation(vec);
    auto wire2 = BRepBuilderAPI_Transform(wire, trafo).Shape();
    ASSERT_TRUE(wire2.Closed());
    auto loft = CTiglMakeLoft();
    loft.addProfiles(wire);
    loft.addProfiles(wire2);
    ASSERT_THROW(loft.Shape(), tigl::CTiglError);
}

TEST_F(FuselageStandardProfile, BuildFuselageRectangle_CornerRadiusZero)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
    tigl::CTiglUIDManager& uidmgr = config.GetUIDManager();
    auto wing = uidmgr.GetGeometricComponent("Wing").GetLoft();
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
    tigl::dumpShape(fuselage->Shape(), "mydir", "fuselage");
    tigl::dumpShape(wing->Shape(), "mydir", "wing1");
}

TEST_F(FuselageStandardProfile, BuildFuselageMixedProfiles_CornerRadiusZero)
{
    // read configuration
    tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle1);
    tigl::CTiglUIDManager& uidmgr = config.GetUIDManager();
    auto wing = uidmgr.GetGeometricComponent("Wing").GetLoft();
    auto fuselage = config.GetFuselage(1).GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(fuselage->Shape()).IsValid());
    tigl::dumpShape(fuselage->Shape(), "mydir", "fuselageMix");
    tigl::dumpShape(wing->Shape(), "mydir", "wing1");
}
