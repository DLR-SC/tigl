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

#include "CCPACSFuelTank.h"
#include "CCPACSFuelTanks.h"
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
        const char* filename = "TestData/simpletest-fuelTanks.cpacs.xml";
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

    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    tigl::CTiglUIDManager& uidMgr =
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(FuselageTank::tiglHandle).GetUIDManager();

    // tank
    tigl::CCPACSFuelTank const* fuelTank = &uidMgr.ResolveObject<tigl::CCPACSFuelTank>("tank1");

    // hulls
    const tigl::CCPACSHulls& hulls    = fuelTank->GetHulls();
    tigl::CCPACSHull* hull_segments   = &uidMgr.ResolveObject<tigl::CCPACSHull>("tank1_outerHull");
    tigl::CCPACSHull* hull_guides     = &uidMgr.ResolveObject<tigl::CCPACSHull>("tank2_outerHull");
    tigl::CCPACSHull* hull_parametric = &uidMgr.ResolveObject<tigl::CCPACSHull>("tank3_sphericalDome");

    tigl::CCPACSHull* hull_spherical     = &uidMgr.ResolveObject<tigl::CCPACSHull>("tank3_sphericalDome");
    tigl::CCPACSHull* hull_ellipsoid     = &uidMgr.ResolveObject<tigl::CCPACSHull>("tank3_ellipsoidDome");
    tigl::CCPACSHull* hull_torispherical = &uidMgr.ResolveObject<tigl::CCPACSHull>("tank4_torisphericalDome");
    tigl::CCPACSHull* hull_isotensoid    = &uidMgr.ResolveObject<tigl::CCPACSHull>("tank5_isotensoidDome");

    const char* tankTypeExceptionString = "This method is only available for hulls with segments. No segment found.";
};

TixiDocumentHandle FuselageTank::tixiHandle           = 0;
TiglCPACSConfigurationHandle FuselageTank::tiglHandle = 0;

void CheckExceptionMessage(std::function<void()> func, const char* expectedMessage)
{
    try {
        func();
        FAIL() << "Expected tigl::CTiglError but no exception was thrown.";
    }
    catch (const tigl::CTiglError& e) {
        EXPECT_STREQ(e.what(), expectedMessage);
    }
    catch (...) {
        FAIL() << "Expected tigl::CTiglError but a different exception was thrown.";
    }
}

TEST_F(FuselageTank, configuration)
{
    auto& config    = fuelTank->GetConfiguration();
    std::string uID = "tank1";
    EXPECT_EQ(config.GetFuelTanksCount(), 6);
    EXPECT_EQ(config.GetFuelTank(1).GetDefaultedUID(), uID);
    EXPECT_NO_THROW(config.GetFuelTank(uID));
    EXPECT_EQ(config.GetFuelTankIndex(uID), 1);
}

TEST_F(FuselageTank, fuelTanks)
{
    std::string uID                        = "tank1";
    const tigl::CCPACSFuelTanks* fuelTanks = fuelTank->GetParent();
    EXPECT_EQ(fuelTanks->GetFuelTank(1).GetDefaultedUID(), uID);
    EXPECT_NO_THROW(fuelTanks->GetFuelTank(uID));
    EXPECT_EQ(fuelTanks->GetFuelTankIndex(uID), 1);
    EXPECT_EQ(fuelTanks->GetFuelTanksCount(), 6);
}

TEST_F(FuselageTank, fuelTank)
{
    EXPECT_NO_THROW(fuelTank->GetHulls());

    const std::string name = fuelTank->GetName();
    EXPECT_EQ(name, "Simple tank 1");
}

// ToDo: Check how to use pointer ->
TEST_F(FuselageTank, hulls)
{
    EXPECT_EQ(hulls.GetHullsCount(), 2);
    EXPECT_EQ(hulls.GetHull(1).GetDefaultedUID(), "tank1_outerHull");
    EXPECT_EQ(hulls.GetHull("tank1_outerHull").GetDefaultedUID(), "tank1_outerHull");
    EXPECT_EQ(hulls.GetHullIndex("tank1_outerHull"), 1);
    EXPECT_EQ(hulls.GetHulls().at(0)->GetDefaultedUID(), "tank1_outerHull");
}

TEST_F(FuselageTank, hull_general)
{
    EXPECT_EQ(hull_segments->GetConfiguration().GetUID(), "testAircraft");
    EXPECT_EQ(hull_segments->GetDefaultedUID(), "tank1_outerHull");
}

TEST_F(FuselageTank, hull_component_info)
{
    EXPECT_EQ(hull_segments->GetComponentType(), TIGL_COMPONENT_FUSELAGE_TANK_HULL);
    EXPECT_EQ(hull_segments->GetComponentIntent(), TIGL_INTENT_PHYSICAL);
}

TEST_F(FuselageTank, hull_type_info)
{
    EXPECT_TRUE(hull_segments->IsHullViaSegments());
    EXPECT_FALSE(hull_segments->IsHullViaDesignParameters());
    EXPECT_FALSE(hull_spherical->IsHullViaSegments());
    EXPECT_TRUE(hull_spherical->IsHullViaDesignParameters());

    EXPECT_FALSE(hull_segments->HasSphericalDome());
    EXPECT_FALSE(hull_segments->HasEllipsoidDome());
    EXPECT_FALSE(hull_segments->HasTorisphericalDome());
    EXPECT_FALSE(hull_segments->HasIsotensoidDome());

    EXPECT_TRUE(hull_spherical->HasSphericalDome());
    EXPECT_TRUE(hull_spherical->HasEllipsoidDome());
    EXPECT_FALSE(hull_spherical->HasTorisphericalDome());
    EXPECT_FALSE(hull_spherical->HasIsotensoidDome());

    EXPECT_FALSE(hull_ellipsoid->HasSphericalDome());
    EXPECT_TRUE(hull_ellipsoid->HasEllipsoidDome());
    EXPECT_FALSE(hull_ellipsoid->HasTorisphericalDome());
    EXPECT_FALSE(hull_ellipsoid->HasIsotensoidDome());

    EXPECT_FALSE(hull_torispherical->HasSphericalDome());
    EXPECT_FALSE(hull_torispherical->HasEllipsoidDome());
    EXPECT_TRUE(hull_torispherical->HasTorisphericalDome());
    EXPECT_FALSE(hull_torispherical->HasIsotensoidDome());

    EXPECT_FALSE(hull_isotensoid->HasSphericalDome());
    EXPECT_FALSE(hull_isotensoid->HasEllipsoidDome());
    EXPECT_FALSE(hull_isotensoid->HasTorisphericalDome());
    EXPECT_TRUE(hull_isotensoid->HasIsotensoidDome());
}

TEST_F(FuselageTank, hull_sections)
{
    const char* invalidIndexMessage    = "Invalid index in CCPACSFuselageSections::GetSection";
    const char* wrongSectionUIDMessage = "GetSectionFace: Could not find a fuselage section for the given UID";

    EXPECT_EQ(hull_segments->GetSectionCount(), 3);
    EXPECT_EQ(hull_parametric->GetSectionCount(), 0);

    EXPECT_NO_THROW(hull_segments->GetSection(1));
    CheckExceptionMessage([&]() { hull_segments->GetSection(4); }, invalidIndexMessage);
    CheckExceptionMessage([&]() { hull_parametric->GetSection(2); }, tankTypeExceptionString);

    EXPECT_NO_THROW(hull_segments->GetSectionFace("outerHull_section3"));
    CheckExceptionMessage([&]() { hull_segments->GetSectionFace("wrongSectionUID"); }, wrongSectionUIDMessage);
    CheckExceptionMessage([&]() { hull_parametric->GetSectionFace("outerHull_section3"); }, tankTypeExceptionString);
}

TEST_F(FuselageTank, hull_segments)
{
    EXPECT_EQ(hull_segments->GetSegmentCount(), 2);
    EXPECT_EQ(hull_parametric->GetSegmentCount(), 0);

    EXPECT_NO_THROW(hull_segments->GetSegment(1));
    CheckExceptionMessage([&]() { hull_parametric->GetSegment(1); }, tankTypeExceptionString);

    EXPECT_NO_THROW(hull_segments->GetSegment("outerHull_segment1"));
    CheckExceptionMessage([&]() { hull_segments->GetSegment(3); },
                          "Invalid index value in CCPACSFuselageSegments::GetSegment");
}

TEST_F(FuselageTank, hull_guide_curves)
{
    auto points = hull_guides->GetGuideCurvePoints();
    EXPECT_EQ(points.size(), 24);
    EXPECT_NEAR(points.at(1).X(), 3.5, 1e-2);
    EXPECT_NEAR(points.at(1).Y(), 0, 1e-5);
    EXPECT_NEAR(points.at(1).Z(), -0.65, 1e-2);
    CheckExceptionMessage([&]() { hull_parametric->GetGuideCurvePoints(); }, tankTypeExceptionString);

    EXPECT_EQ(hull_guides->GetGuideCurveSegment("tank2_seg1_upper").GetGuideCurveProfileUID(), "gc_upper");
    CheckExceptionMessage([&]() { hull_parametric->GetGuideCurveSegment("tank2_seg1_upper"); },
                          tankTypeExceptionString);
}

TEST_F(FuselageTank, hull_loft_evaluation)
{
    EXPECT_NEAR(hull_segments->GetVolume(), 6.57, 1e-2);
    EXPECT_NEAR(hull_parametric->GetVolume(), 18.1, 1e-2);

    EXPECT_NEAR(hull_segments->GetSurfaceArea(), 11.15, 1e-2);
    EXPECT_NEAR(hull_parametric->GetSurfaceArea(), 36.19, 1e-2);

    EXPECT_NEAR(hull_segments->GetCircumference(1, 0.5), 7.43, 1e-2);
    CheckExceptionMessage([&]() { hull_parametric->GetCircumference(1, 0.5); }, tankTypeExceptionString);

    EXPECT_NEAR(hull_segments->GetPoint(1, 0.5, 0.5).X(), 1.54, 1e-2);
    EXPECT_NEAR(hull_segments->GetPoint(1, 0.5, 0.5).Y(), 0, 1e-5);
    EXPECT_NEAR(hull_segments->GetPoint(1, 0.5, 0.5).Z(), -1.2, 1e-1);
    CheckExceptionMessage([&]() { hull_parametric->GetPoint(1, 0.5, 0.5); }, tankTypeExceptionString);

    EXPECT_EQ(hull_segments->GetGetPointBehavior(), asParameterOnSurface);
    EXPECT_NO_THROW(hull_segments->SetGetPointBehavior(onLinearLoft));
    EXPECT_EQ(hull_segments->GetGetPointBehavior(), onLinearLoft);
}

TEST_F(FuselageTank, parametric_hull)
{
    auto& loft = hull_parametric->GetLoft();
}

TEST_F(FuselageTank, structure)
{
    auto& structure = hull_segments->GetStructure();

    EXPECT_EQ(structure->GetFrames()->GetFrames().size(), 1);
    EXPECT_EQ(structure->GetUID(), "outerHullStructure");
}