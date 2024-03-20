/*
 * Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
 *
 * Created: 2023-12-29 Marko Alder <marko.alder@dlr.de>
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

#include "test.h"
#include "tigl.h"

#include "CCPACSGenericFuelTank.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"

#include "CCPACSHull.h"

// #include "generated/CPACSHulls.h"
// #include "generated/CPACSHull.h"

// #include "CNamedShape.h"
// #include "tiglcommonfunctions.h"

// #include <Bnd_Box.hxx>
// #include <BRepBndLib.hxx>
// #include <gp_Pnt.hxx>

// #include <TopoDS_Shape.hxx>

// #include <TopTools_IndexedMapOfShape.hxx>
// #include <TopExp.hxx>

// #include <TopoDS_Iterator.hxx>

class FuselageTank : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const char *filename = "TestData/simpletest-fuelTanks.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE(tixiRet == SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "testAircraft", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    void SetUp() override {}
    void TearDown() override {}

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    tigl::CTiglUIDManager &uidMgr = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(FuselageTank::tiglHandle).GetUIDManager();

    // generic tank
    tigl::CCPACSGenericFuelTank const *fuelTank = &uidMgr.ResolveObject<tigl::CCPACSGenericFuelTank>("genericTank1");

    // hulls
    const boost::optional<tigl::CCPACSHulls> &hulls = fuelTank->GetHulls_choice1();
    tigl::CCPACSHull *hull = &uidMgr.ResolveObject<tigl::CCPACSHull>("outerHull");
};

TixiDocumentHandle FuselageTank::tixiHandle = 0;
TiglCPACSConfigurationHandle FuselageTank::tiglHandle = 0;

TEST_F(FuselageTank, getName)
{
    const std::string name = fuelTank->GetName();
    EXPECT_EQ(name, "Simple tank");
}

TEST_F(FuselageTank, hulls)
{
    EXPECT_EQ(hulls->GetHullsCount(), 2);
    EXPECT_EQ(hulls->GetHull(1).GetDefaultedUID(), "outerHull");
    EXPECT_EQ(hulls->GetHull("outerHull").GetDefaultedUID(), "outerHull");
    EXPECT_EQ(hulls->GetHullIndex("outerHull"), 1);
    EXPECT_EQ(hulls->GetHulls().at(0)->GetDefaultedUID(), "outerHull");
}

TEST_F(FuselageTank, hull)
{
    EXPECT_EQ(hull->GetSectionCount(), 3);
    EXPECT_EQ(hull->GetSegmentCount(), 2);
    EXPECT_NEAR(hull->GetVolume(), 0.41, 1e-2);
    EXPECT_NEAR(hull->GetSurfaceArea(), 2.79, 1e-2);
    EXPECT_NEAR(hull->GetCircumference(1, 0.5), 1.86, 1e-2);

    EXPECT_EQ(hull->GetSectionCount(), 3);
    EXPECT_NO_THROW(hull->GetSection(1));
    EXPECT_NO_THROW(hull->GetSectionFace("outerHull_section1"));
    EXPECT_THROW(hull->GetSection(4), tigl::CTiglError);

    EXPECT_EQ(hull->GetSegmentCount(), 2);
    EXPECT_NO_THROW(hull->GetSegment(1));
    EXPECT_NO_THROW(hull->GetSegment("outerHull_segment1"));
    EXPECT_THROW(hull->GetSegment(3), tigl::CTiglError);

    EXPECT_EQ(hull->GetDefaultedUID(), "outerHull");
    EXPECT_NO_THROW(hull->GetConfiguration());

    EXPECT_NEAR(hull->GetPoint(1, 0.5, 0.5).X(), 2.54, 1e-2);
    EXPECT_NEAR(hull->GetPoint(1, 0.5, 0.5).Y(), 0, 1e-5);
    EXPECT_NEAR(hull->GetPoint(1, 0.5, 0.5).Z(), -0.3, 1e-1);
}