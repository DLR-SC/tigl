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

#include "CCPACSVessel.h"
#include "CNamedShape.h"

class DummyAircraftModel : public tigl::CCPACSAircraftModel
{
public:
    DummyAircraftModel()
        : tigl::CCPACSAircraftModel(nullptr, nullptr)
    {
    }
};

class FuelTanks : public ::testing::Test
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
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(FuelTanks::tiglHandle).GetUIDManager();

    // tank
    tigl::CCPACSFuelTank const* fuelTank = &uidMgr.ResolveObject<tigl::CCPACSFuelTank>("tank1");

    // vessels
    const tigl::CCPACSVessels& vessels    = fuelTank->GetVessels();
    tigl::CCPACSVessel* vessel_segments   = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank1_outerVessel");
    tigl::CCPACSVessel* vessel_guides     = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank2_outerVessel");
    tigl::CCPACSVessel* vessel_parametric = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank3_sphericalDome");

    tigl::CCPACSVessel* vessel_spherical     = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank3_sphericalDome");
    tigl::CCPACSVessel* vessel_ellipsoid     = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank3_ellipsoidDome");
    tigl::CCPACSVessel* vessel_torispherical = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank4_torisphericalDome");
    tigl::CCPACSVessel* vessel_isotensoid    = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank5_isotensoidDome");

    tigl::CCPACSVessel* vessel_symmetric = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank7_symmetricVessel");
    tigl::CCPACSVessel* vessel_corrupt   = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank_corrupt_vessel");

    const char* tankTypeExceptionString = "This method is only available for vessels with segments. No segment found.";
};

TixiDocumentHandle FuelTanks::tixiHandle           = 0;
TiglCPACSConfigurationHandle FuelTanks::tiglHandle = 0;

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

TEST_F(FuelTanks, configuration)
{
    auto& config    = fuelTank->GetConfiguration();
    std::string uID = "tank1";
    EXPECT_EQ(config.GetFuelTanksCount(), 8);
    EXPECT_EQ(config.GetFuelTank(1).GetDefaultedUID(), uID);
    EXPECT_NO_THROW(config.GetFuelTank(uID));
    EXPECT_EQ(config.GetFuelTankIndex(uID), 1);

    CheckExceptionMessage([&]() { config.GetFuelTank("not_existing_tank"); },
                          "Could not find fuelTank with uid not_existing_tank");

    CheckExceptionMessage([&]() { config.GetFuelTankIndex("not_existing_tank"); },
                          "Could not find fuelTank with uid not_existing_tank");

    CheckExceptionMessage([&]() { config.GetFuelTank(99); }, "Invalid index in CCPACSFuelTanks::GetFuelTank");
}

TEST_F(FuelTanks, fuelTanks)
{
    std::string uID                        = "tank1";
    const tigl::CCPACSFuelTanks* fuelTanks = fuelTank->GetParent();
    EXPECT_EQ(fuelTanks->GetFuelTank(1).GetDefaultedUID(), uID);
    EXPECT_NO_THROW(fuelTanks->GetFuelTank(uID));
    EXPECT_EQ(fuelTanks->GetFuelTankIndex(uID), 1);
    EXPECT_EQ(fuelTanks->GetFuelTanksCount(), 8);
}

TEST_F(FuelTanks, fuelTank)
{
    EXPECT_NO_THROW(fuelTank->GetVessels());

    const std::string name = fuelTank->GetName();
    EXPECT_EQ(name, "Simple tank 1");

    EXPECT_EQ(fuelTank->GetComponentType(), TIGL_COMPONENT_TANK);

    // Test loft building
    const auto loft = fuelTank->GetLoft();
    ASSERT_NE(loft, nullptr);
    EXPECT_EQ(loft->Name(), "tank1");
    EXPECT_EQ(loft->ShortName(), "T1");

    // Test return of UNKNOWN ShortName
    DummyAircraftModel dummyAircraft;
    tigl::CTiglUIDManager dummyUidMgr;
    tigl::CCPACSFuelTanks dummyTanks(&dummyAircraft, &dummyUidMgr);
    tigl::CCPACSFuelTank dummyTank(&dummyTanks, &dummyUidMgr);
    dummyTank.SetUID("not_in_list");
    const auto dummyLoft = dummyTank.GetLoft();
    EXPECT_EQ(dummyLoft->ShortName(), "UNKNOWN");

    // Test invalidation
    bool called = false;
    dummyTank.RegisterInvalidationCallback([&]() { called = true; });
    dummyTank.Invalidate();
    EXPECT_TRUE(called);
}

// ToDo: Check how to use pointer ->
TEST_F(FuelTanks, vessels)
{
    EXPECT_EQ(vessels.GetVesselsCount(), 2);
    EXPECT_EQ(vessels.GetVessel(1).GetDefaultedUID(), "tank1_outerVessel");
    EXPECT_EQ(vessels.GetVessel("tank1_outerVessel").GetDefaultedUID(), "tank1_outerVessel");
    EXPECT_EQ(vessels.GetVesselIndex("tank1_outerVessel"), 1);
    EXPECT_EQ(vessels.GetVessels().at(0)->GetDefaultedUID(), "tank1_outerVessel");

    CheckExceptionMessage([&]() { vessels.GetVessel("not_existing_vessel"); },
                          "Could not find vessel with uid not_existing_vessel");

    CheckExceptionMessage([&]() { vessels.GetVesselIndex("not_existing_vessel"); },
                          "Could not find vessel with uid not_existing_vessel");

    CheckExceptionMessage([&]() { vessels.GetVessel(99); }, "Invalid index in CCPACSVessels::GetVessel");
}

TEST_F(FuelTanks, vessel_general)
{
    EXPECT_EQ(vessel_segments->GetConfiguration().GetUID(), "testAircraft");
    EXPECT_EQ(vessel_segments->GetDefaultedUID(), "tank1_outerVessel");
}

TEST_F(FuelTanks, vessel_component_info)
{
    EXPECT_EQ(vessel_segments->GetComponentType(), TIGL_COMPONENT_TANK_HULL);
    EXPECT_EQ(vessel_segments->GetComponentIntent(), TIGL_INTENT_PHYSICAL);
}

TEST_F(FuelTanks, vessel_type_info)
{
    EXPECT_TRUE(vessel_segments->IsVesselViaSegments());
    EXPECT_FALSE(vessel_segments->IsVesselViaDesignParameters());
    EXPECT_FALSE(vessel_spherical->IsVesselViaSegments());
    EXPECT_TRUE(vessel_spherical->IsVesselViaDesignParameters());

    EXPECT_FALSE(vessel_segments->HasSphericalDome());
    EXPECT_FALSE(vessel_segments->HasEllipsoidDome());
    EXPECT_FALSE(vessel_segments->HasTorisphericalDome());
    EXPECT_FALSE(vessel_segments->HasIsotensoidDome());

    EXPECT_TRUE(vessel_spherical->HasSphericalDome());
    EXPECT_TRUE(vessel_spherical->HasEllipsoidDome());
    EXPECT_FALSE(vessel_spherical->HasTorisphericalDome());
    EXPECT_FALSE(vessel_spherical->HasIsotensoidDome());

    EXPECT_FALSE(vessel_ellipsoid->HasSphericalDome());
    EXPECT_TRUE(vessel_ellipsoid->HasEllipsoidDome());
    EXPECT_FALSE(vessel_ellipsoid->HasTorisphericalDome());
    EXPECT_FALSE(vessel_ellipsoid->HasIsotensoidDome());

    EXPECT_FALSE(vessel_torispherical->HasSphericalDome());
    EXPECT_FALSE(vessel_torispherical->HasEllipsoidDome());
    EXPECT_TRUE(vessel_torispherical->HasTorisphericalDome());
    EXPECT_FALSE(vessel_torispherical->HasIsotensoidDome());

    EXPECT_FALSE(vessel_isotensoid->HasSphericalDome());
    EXPECT_FALSE(vessel_isotensoid->HasEllipsoidDome());
    EXPECT_FALSE(vessel_isotensoid->HasTorisphericalDome());
    EXPECT_TRUE(vessel_isotensoid->HasIsotensoidDome());

    EXPECT_THROW(vessel_corrupt->IsVesselViaSegments(), tigl::CTiglError);
    EXPECT_THROW(vessel_corrupt->IsVesselViaDesignParameters(), tigl::CTiglError);
}

TEST_F(FuelTanks, vessel_sections)
{
    const char* invalidIndexMessage    = "Invalid index in CCPACSFuselageSections::GetSection";
    const char* wrongSectionUIDMessage = "GetSectionFace: Could not find a fuselage section for the given UID";

    EXPECT_EQ(vessel_segments->GetSectionCount(), 3);
    EXPECT_EQ(vessel_parametric->GetSectionCount(), 0);

    EXPECT_NO_THROW(vessel_segments->GetSection(1));
    CheckExceptionMessage([&]() { vessel_segments->GetSection(4); }, invalidIndexMessage);
    CheckExceptionMessage([&]() { vessel_parametric->GetSection(2); }, tankTypeExceptionString);

    EXPECT_NO_THROW(vessel_segments->GetSectionFace("outerVessel_section1"));
    EXPECT_NO_THROW(vessel_segments->GetSectionFace("outerVessel_section3"));
    CheckExceptionMessage([&]() { vessel_segments->GetSectionFace("wrongSectionUID"); }, wrongSectionUIDMessage);
    CheckExceptionMessage([&]() { vessel_parametric->GetSectionFace("outerVessel_section3"); },
                          tankTypeExceptionString);
}

TEST_F(FuelTanks, vessel_segments)
{
    EXPECT_EQ(vessel_segments->GetSegmentCount(), 2);
    EXPECT_EQ(vessel_parametric->GetSegmentCount(), 0);

    const tigl::CCPACSVessel* const_vessel_parametric = vessel_parametric;

    EXPECT_NO_THROW(vessel_segments->GetSegment(1));
    CheckExceptionMessage([&]() { vessel_parametric->GetSegment(1); }, tankTypeExceptionString);
    CheckExceptionMessage([&]() { const_vessel_parametric->GetSegment(1); }, tankTypeExceptionString);
    CheckExceptionMessage([&]() { vessel_parametric->GetSegment("outerVessel_segment1"); }, tankTypeExceptionString);

    EXPECT_NO_THROW(vessel_segments->GetSegment("outerVessel_segment1"));
    CheckExceptionMessage([&]() { vessel_segments->GetSegment(3); },
                          "Invalid index value in CCPACSFuselageSegments::GetSegment");
}

TEST_F(FuelTanks, vessel_guide_curves)
{
    auto points = vessel_guides->GetGuideCurvePoints();
    EXPECT_EQ(points.size(), 24);
    EXPECT_NEAR(points.at(1).X(), 3.5, 1e-2);
    EXPECT_NEAR(points.at(1).Y(), 0, 1e-5);
    EXPECT_NEAR(points.at(1).Z(), -0.65, 1e-2);
    CheckExceptionMessage([&]() { vessel_parametric->GetGuideCurvePoints(); }, tankTypeExceptionString);

    EXPECT_EQ(vessel_guides->GetGuideCurveSegment("tank2_seg1_upper").GetGuideCurveProfileUID(), "gc_upper");
    CheckExceptionMessage([&]() { vessel_parametric->GetGuideCurveSegment("tank2_seg1_upper"); },
                          tankTypeExceptionString);

    CheckExceptionMessage([&]() { vessel_segments->GetGuideCurveSegment("not_existing_guide_curve"); },
                          "Guide Curve with UID not_existing_guide_curve does not exists");
}

TEST_F(FuelTanks, vessel_loft_evaluation)
{
    EXPECT_NEAR(vessel_segments->GetGeometricVolume(), 6.57, 1e-2);
    EXPECT_NEAR(vessel_spherical->GetGeometricVolume(), 18.1, 1e-2);
    EXPECT_NEAR(vessel_ellipsoid->GetGeometricVolume(), 5.43, 1e-2);
    EXPECT_NEAR(vessel_torispherical->GetGeometricVolume(), 7.87, 1e-2);
    EXPECT_NEAR(vessel_isotensoid->GetGeometricVolume(), 9.01, 1e-2);

    EXPECT_NEAR(vessel_segments->GetSurfaceArea(), 11.15, 1e-2);
    EXPECT_NEAR(vessel_spherical->GetSurfaceArea(), 36.19, 1e-2);
    EXPECT_NEAR(vessel_ellipsoid->GetSurfaceArea(), 16.22, 1e-2);
    EXPECT_NEAR(vessel_torispherical->GetSurfaceArea(), 20.49, 1e-2);
    EXPECT_NEAR(vessel_isotensoid->GetSurfaceArea(), 22.40, 1e-2);

    EXPECT_NEAR(vessel_segments->GetCircumference(1, 0.5), 7.43, 1e-2);
    CheckExceptionMessage([&]() { vessel_parametric->GetCircumference(1, 0.5); }, tankTypeExceptionString);

    EXPECT_NEAR(vessel_segments->GetPoint(1, 0.5, 0.5).X(), 1.54, 1e-2);
    EXPECT_NEAR(vessel_segments->GetPoint(1, 0.5, 0.5).Y(), 0, 1e-5);
    EXPECT_NEAR(vessel_segments->GetPoint(1, 0.5, 0.5).Z(), -1.2, 1e-1);
    CheckExceptionMessage([&]() { vessel_parametric->GetPoint(1, 0.5, 0.5); }, tankTypeExceptionString);

    EXPECT_EQ(vessel_segments->GetGetPointBehavior(), asParameterOnSurface);
    EXPECT_NO_THROW(vessel_segments->SetGetPointBehavior(onLinearLoft));
    EXPECT_EQ(vessel_segments->GetGetPointBehavior(), onLinearLoft);

    EXPECT_THROW(vessel_corrupt->GetLoft(), tigl::CTiglError);
}

TEST_F(FuelTanks, vessel_face_traits)
{
    auto symmetric_loft = vessel_symmetric->GetLoft();
    EXPECT_EQ(symmetric_loft->FaceTraits(0).Name(), vessel_symmetric->GetUID());
    EXPECT_EQ(symmetric_loft->FaceTraits(1).Name(), "symmetry");
    EXPECT_EQ(symmetric_loft->FaceTraits(2).Name(), "Front");
    EXPECT_EQ(symmetric_loft->FaceTraits(3).Name(), "Rear");

    auto standard_loft = vessel_segments->GetLoft();
    EXPECT_EQ(standard_loft->FaceTraits(0).Name(), vessel_segments->GetUID());
    EXPECT_EQ(standard_loft->FaceTraits(1).Name(), vessel_segments->GetUID());
    EXPECT_EQ(standard_loft->FaceTraits(2).Name(), "Front");
    EXPECT_EQ(standard_loft->FaceTraits(3).Name(), "Rear");
}

TEST_F(FuelTanks, structure)
{
    auto& structure_with_walls           = vessel_isotensoid->GetStructure();
    auto& structure_with_stringer_frames = vessel_segments->GetStructure();

    EXPECT_EQ(structure_with_stringer_frames->GetFrames()->GetFrames().size(), 1);
    EXPECT_EQ(structure_with_stringer_frames->GetStringers()->GetStringers().size(), 1);

    EXPECT_EQ(structure_with_walls->GetUID(), "tank5_structure");
    EXPECT_EQ(structure_with_walls->GetWalls()->GetWallSegments().GetWallSegments().size(), 3);
}
