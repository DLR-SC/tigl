/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2015-01-07 Anton Reiswich <Anton.Reiswich@dlr.de>
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
/**
* @file
* @brief Tests for testing duct functions.
*/

#include "test.h" // Brings in the GTest framework

#include "tigl.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"
#include "CCPACSDuct.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Pnt.hxx>

/******************************************************************************/

class DuctSimple : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char* filename = "TestData/simpletest_ducts.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_EQ (tixiRet, SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "DuctColaDose", &tiglHandle);
        ASSERT_EQ(tiglRet, TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() override {
        auto& uidMgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetUIDManager();
        duct = &uidMgr.ResolveObject<tigl::CCPACSDuct>("SimpleDuct");

    }
    void TearDown() override {
        // reset WithDuctCutouts flags
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(1).SetWithDuctCutouts(false);
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(2).SetWithDuctCutouts(false);
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetWing(1).SetWithDuctCutouts(false);
    }

    static TixiDocumentHandle           tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;
    tigl::CCPACSDuct const* duct {nullptr};
};

TixiDocumentHandle DuctSimple::tixiHandle = 0;
TiglCPACSConfigurationHandle DuctSimple::tiglHandle = 0;

TEST_F(DuctSimple, LoftGeometry)
{
    auto shape = duct->GetLoft();
    const TopoDS_Shape& ductShape = shape->Shape();

    //Check the positioning of the duct with help of its bounding box
    Bnd_Box ductBBox;
    BRepBndLib::AddOptimal(ductShape, ductBBox);

    // X positioning of duct in coordinate system
    const double precisionX = 1.E-5 * (ductBBox.CornerMax().X() - ductBBox.CornerMin().X());

    ASSERT_NEAR(ductBBox.CornerMax().X(), 3., precisionX );
    ASSERT_NEAR(ductBBox.CornerMin().X(), -1., precisionX );

    // Y positioning of duct in coordinate system
    const double precisionY = 1.E-5 * (ductBBox.CornerMax().Y() - ductBBox.CornerMin().Y());

    ASSERT_NEAR(ductBBox.CornerMax().Y(), 0.25, precisionY );
    ASSERT_NEAR(ductBBox.CornerMin().Y(), -0.25, precisionY );

    // Z positioning of duct in coordinate system
    const double precisionZ = 1.E-5 * (ductBBox.CornerMax().Z() - ductBBox.CornerMin().Z());

    ASSERT_NEAR(ductBBox.CornerMax().Z(), 0.1, precisionZ );
    ASSERT_NEAR(ductBBox.CornerMin().Z(), -0.1, precisionZ );

    //Check for the right number of faces
    EXPECT_EQ(GetNumberOfFaces(ductShape), 4);

    //Check for the right number of edges
    EXPECT_EQ(GetNumberOfEdges(ductShape),10);

}

TEST_F(DuctSimple, SanityCheck)
{
    auto& ducts = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetDucts();

    ASSERT_TRUE(ducts);
    EXPECT_EQ(ducts->GetDucts().size(), 10);
    EXPECT_EQ(duct->GetSegments().GetSegmentCount(), 2);
    EXPECT_EQ(duct->GetSections().GetSectionCount(), 3);
    ASSERT_TRUE(duct->GetPositionings());
    EXPECT_EQ(duct->GetPositionings()->GetPositionings().size(), 2);
}

TEST_F(DuctSimple, FuselageDuctCutOut)
{
    auto& fuselage_1 = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(2);
    fuselage_1.SetWithDuctCutouts(true);
    auto shape = fuselage_1.GetLoft();
    const TopoDS_Shape& cutFuselage_1_Shape = shape->Shape();

    EXPECT_FALSE(IsPointInsideShape(cutFuselage_1_Shape, gp_Pnt(0.0, 0.0, 0.0)));
    EXPECT_TRUE(IsPointInsideShape(cutFuselage_1_Shape, gp_Pnt(0.0, 0.0, 0.25)));
    EXPECT_FALSE(IsPointInsideShape(cutFuselage_1_Shape, gp_Pnt(0.0, 0.0, 0.6)));
    EXPECT_FALSE(IsPointInsideShape(cutFuselage_1_Shape, gp_Pnt(0.0, 0.4, -0.1)));
    EXPECT_FALSE(IsPointInsideShape(cutFuselage_1_Shape, gp_Pnt(1.0, 0.0, 0.25)));

    //Check for the right number of faces
    EXPECT_EQ(GetNumberOfFaces(cutFuselage_1_Shape), 12);

    //Check for the right number of edges
    EXPECT_EQ(GetNumberOfEdges(cutFuselage_1_Shape),76); //not checked

    //Check if the bounding box of the cut fuselage has the right dimensions
    Bnd_Box ductBBox;
    BRepBndLib::AddOptimal(cutFuselage_1_Shape, ductBBox);

    // X positioning of cut fuselage in coordinate system
    const double precisionX = 1.E-5 * (ductBBox.CornerMax().X() - ductBBox.CornerMin().X());

    ASSERT_NEAR(ductBBox.CornerMax().X(), 0.5, precisionX );
    ASSERT_NEAR(ductBBox.CornerMin().X(), -0.5, precisionX );
/*
    // Y positioning of cut fuselage in coordinate system
    const double precisionY = 1.E-5 * (ductBBox.CornerMax().Y() - ductBBox.CornerMin().Y());

    ASSERT_NEAR(ductBBox.CornerMax().Y(), 0.25, precisionY );
    ASSERT_NEAR(ductBBox.CornerMin().Y(), -0.25, precisionY );

    // Z positioning of duct in coordinate system
    const double precisionZ = 1.E-5 * (ductBBox.CornerMax().Z() - ductBBox.CornerMin().Z());

    ASSERT_NEAR(ductBBox.CornerMax().Z(), 0.1, precisionZ );
    ASSERT_NEAR(ductBBox.CornerMin().Z(), -0.1, precisionZ );
*/


}

TEST_F(DuctSimple, WingDuctCutOut)
{
    auto& wing = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetWing(1);
    wing.SetWithDuctCutouts(true);
    wing.GetLoft();

    // TODO test geometry somehow (e.g. IsPointInside?)
}

TEST_F(DuctSimple, tiglConfigurationSetWithDuctCutouts){

    auto& wing = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetWing(1);
    auto& fuselage1 = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(1);
    auto& fuselage2 = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(2);

    // test errors
    EXPECT_EQ(TIGL_NOT_FOUND, tiglConfigurationSetWithDuctCutouts(-1, "Wing", TIGL_TRUE));
    EXPECT_EQ(TIGL_UID_ERROR, tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, "Rxlquap", TIGL_TRUE));
    EXPECT_EQ(TIGL_UID_ERROR, tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, "fuselageCircleProfileuID", TIGL_TRUE));

    // ducts are not considered by default
    EXPECT_FALSE(wing.WithDuctCutouts());
    EXPECT_FALSE(fuselage1.WithDuctCutouts());
    EXPECT_FALSE(fuselage2.WithDuctCutouts());

    // set the flag for all components, by setting the componentUID to NULL
    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, NULL, TIGL_TRUE));
    EXPECT_TRUE(wing.WithDuctCutouts());
    EXPECT_TRUE(fuselage1.WithDuctCutouts());
    EXPECT_TRUE(fuselage2.WithDuctCutouts());

    // set the flag for individual components
    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, "SimpleFuselage", TIGL_FALSE));
    EXPECT_TRUE(wing.WithDuctCutouts());
    EXPECT_FALSE(fuselage1.WithDuctCutouts());
    EXPECT_TRUE(fuselage2.WithDuctCutouts());

    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, "SimpleFuselage2", TIGL_FALSE));
    EXPECT_TRUE(wing.WithDuctCutouts());
    EXPECT_FALSE(fuselage1.WithDuctCutouts());
    EXPECT_FALSE(fuselage2.WithDuctCutouts());

    ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, "Wing", TIGL_FALSE));
    EXPECT_FALSE(wing.WithDuctCutouts());
    EXPECT_FALSE(fuselage1.WithDuctCutouts());
    EXPECT_FALSE(fuselage2.WithDuctCutouts());

    //TODO: Should we test also the fuselage wing structure?
}

TEST_F(DuctSimple, tiglConfigurationGetWithDuctCutouts)
{
    TiglBoolean flag;

    // test errors
    EXPECT_EQ(TIGL_NOT_FOUND, tiglConfigurationGetWithDuctCutouts(-1, "Wing", &flag));
    EXPECT_EQ(TIGL_NULL_POINTER, tiglConfigurationGetWithDuctCutouts(DuctSimple::tiglHandle, NULL, &flag));
    EXPECT_EQ(TIGL_UID_ERROR, tiglConfigurationGetWithDuctCutouts(DuctSimple::tiglHandle, "Rxlquap", &flag));
    EXPECT_EQ(TIGL_UID_ERROR, tiglConfigurationGetWithDuctCutouts(DuctSimple::tiglHandle, "fuselageCircleProfileuID", &flag));

    std::vector<std::string> uids {"SimpleFuselage", "SimpleFuselage2", "Wing"};
    for (auto& uid: uids) {
        ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationGetWithDuctCutouts(DuctSimple::tiglHandle, uid.c_str(), &flag))
                << "for uid " << uid;
        // ducts are not considered by default
        EXPECT_FALSE(flag);

        // check if the flag changes after a call to tiglConfigurationSetWithDuctCutouts
        tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, uid.c_str(), (TiglBoolean)!(bool)flag);
        ASSERT_EQ(TIGL_SUCCESS, tiglConfigurationGetWithDuctCutouts(DuctSimple::tiglHandle, uid.c_str(), &flag))
                << "for uid " << uid;
        EXPECT_TRUE(flag);
    }

    //TODO: Should we test also the fuselage wing structure?

}


TEST_F(DuctSimple, Export)
{
    tiglConfigurationSetWithDuctCutouts(DuctSimple::tiglHandle, NULL, TIGL_TRUE);
    tiglSetExportOptions("iges", "ApplySymmetries", "true");
    tiglSetExportOptions("iges", "IncludeFarfield", "false");
    tiglSetExportOptions("iges", "FaceNames", "UIDandFaceName");
    EXPECT_EQ(TIGL_SUCCESS, tiglExportIGES(DuctSimple::tiglHandle,"TestData/export/duct_simple.iges"));
    EXPECT_EQ(TIGL_SUCCESS, tiglExportFusedWingFuselageIGES(DuctSimple::tiglHandle,"TestData/export/duct_simple_fused.iges"));
}

