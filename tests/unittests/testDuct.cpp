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

#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"
#include "CCPACSDuct.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

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
    void TearDown() override {}


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
    EXPECT_EQ(ducts->GetDucts().size(), 1);

    EXPECT_EQ(duct->GetSegments().GetSegmentCount(), 2);

    EXPECT_EQ(duct->GetSections().GetSectionCount(), 3);

    ASSERT_TRUE(duct->GetPositionings());
    EXPECT_EQ(duct->GetPositionings()->GetPositionings().size(), 2);

}

TEST_F(DuctSimple, DuctCutOut)
{
   /* auto& fuselage = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(DuctSimple::tiglHandle).GetFuselage(1);
    fuselage.SetWithDucts(true);
    fuselage.GetLoft();
   */
}

