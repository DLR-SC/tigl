/*
 * Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
 *
 * Created: 2025-05-09 Marko Alder <marko.alder@dlr.de>
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
#include "testUtils.h"

#include "CCPACSFuelTank.h"
#include "generated/CPACSFuelTanks.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglUIDManager.h"

#include "CCPACSVessel.h"
#include "CNamedShape.h"

namespace
{
// Error message constants for exception tests
constexpr const char* tankTypeExceptionString =
    "This method is only available for vessels with segments. No segment found.";
constexpr const char* invalidIndexMessage    = "Invalid index in CCPACSFuselageSections::GetSection";
constexpr const char* wrongSectionUIDMessage = "GetSectionFace: Could not find a fuselage section for the given UID";
} // anonymous namespace

// Dummy class for exception handling tests
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
        ASSERT_EQ(tixiRet, SUCCESS);
        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "testAircraft", &tiglHandle);
        ASSERT_EQ(tiglRet, TIGL_SUCCESS);
    }

    static void TearDownTestCase()
    {
        ASSERT_EQ(tiglCloseCPACSConfiguration(tiglHandle), TIGL_SUCCESS);
        ASSERT_EQ(tixiCloseDocument(tixiHandle), SUCCESS);
        tiglHandle = -1;
        tixiHandle = -1;
    }

    // No specific setup/teardown per test needed at the moment.
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    // Static handles for CPACS document and configuration.
    static TixiDocumentHandle tixiHandle;
    static TiglCPACSConfigurationHandle tiglHandle;

    // UID manager for object resolution
    tigl::CTiglUIDManager& uidMgr =
        tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(FuelTanks::tiglHandle).GetUIDManager();

    // Fuel tank objects
    const tigl::CCPACSFuelTank* fuelTank         = &uidMgr.ResolveObject<tigl::CCPACSFuelTank>("tank1");
    const tigl::CCPACSFuelTank* fuelTank_corrupt = &uidMgr.ResolveObject<tigl::CCPACSFuelTank>("tank_corrupt");

    // Vessel objects resolved from the fuel tank
    const tigl::CCPACSVessels& vessels    = fuelTank->GetVessels();
    tigl::CCPACSVessel* vessel_segments   = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank1_outerVessel");
    tigl::CCPACSVessel* vessel_guides     = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank2_outerVessel");
    tigl::CCPACSVessel* vessel_parametric = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank3_sphericalDome");

    tigl::CCPACSVessel* vessel_spherical     = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank3_sphericalDome");
    tigl::CCPACSVessel* vessel_ellipsoid1    = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank3_ellipsoidDome1");
    tigl::CCPACSVessel* vessel_ellipsoid2    = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank3_ellipsoidDome2");
    tigl::CCPACSVessel* vessel_torispherical = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank4_torisphericalDome");
    tigl::CCPACSVessel* vessel_isotensoid    = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank5_isotensoidDome");

    tigl::CCPACSVessel* vessel_symmetric = &uidMgr.ResolveObject<tigl::CCPACSVessel>("tank7_symmetricVessel");
    tigl::CCPACSVessel* vessel_corrupt1   = &uidMgr.ResolveObject<tigl::CCPACSVessel>("vessel_corrupt_geometry1");
    tigl::CCPACSVessel* vessel_corrupt2  = &uidMgr.ResolveObject<tigl::CCPACSVessel>("vessel_corrupt_geometry2");

    // Dummy objects for exception testing
    DummyAircraftModel dummyAircraft;
    tigl::CTiglUIDManager dummyUidMgr;
    tigl::generated::CPACSFuelTanks dummyTanks{&dummyAircraft, &dummyUidMgr};
    tigl::CCPACSFuelTank dummyTank{&dummyTanks, &dummyUidMgr};
    tigl::generated::CPACSVessels dummyVessels{&dummyTank, &dummyUidMgr};
    tigl::CCPACSVessel dummyVessel{&dummyVessels, &dummyUidMgr};
};

TixiDocumentHandle FuelTanks::tixiHandle           = 0;
TiglCPACSConfigurationHandle FuelTanks::tiglHandle = 0;

TEST_F(FuelTanks, configuration)
{
    auto& config    = fuelTank->GetConfiguration();
    std::string uID = "tank1";

    EXPECT_EQ(config.GetFuelTankCount(), 8);
    EXPECT_EQ(config.GetFuelTank(1).GetDefaultedUID(), uID);
    EXPECT_NO_THROW(config.GetFuelTank(uID));
    EXPECT_EQ(config.GetFuelTankIndex(uID), 1);

    CheckExceptionMessage([&]() { config.GetFuelTank("not_existing_tank"); },
                          "Invalid UID in CPACSFuelTanks::GetFuelTank. \"not_existing_tank\" not found in CPACS file!" );

    CheckExceptionMessage([&]() { config.GetFuelTankIndex("not_existing_tank"); },
                          "Invalid UID in CPACSFuelTanks::GetFuelTankIndex");

    CheckExceptionMessage([&]() { config.GetFuelTank(99); }, "Invalid index in std::vector<std::unique_ptr<CCPACSFuelTank>>::GetFuelTank");
}

TEST_F(FuelTanks, fuelTanks)
{
    std::string uID                        = "tank1";
    const tigl::CCPACSFuelTanks* fuelTanks = fuelTank->GetParent();
    EXPECT_EQ(fuelTanks->GetFuelTank(1).GetDefaultedUID(), uID);
    EXPECT_NO_THROW(fuelTanks->GetFuelTank(uID));
    EXPECT_EQ(fuelTanks->GetFuelTankIndex(uID), 1);
    EXPECT_EQ(fuelTanks->GetFuelTankCount(), 8);
}

TEST_F(FuelTanks, fuelTank)
{
    EXPECT_NO_THROW(fuelTank->GetVessels());

    // Check tank name and type.
    EXPECT_EQ(fuelTank->GetName(), "Simple tank 1");
    EXPECT_EQ(fuelTank->GetComponentType(), TIGL_COMPONENT_TANK);

    // Test loft building and naming.
    auto loft = fuelTank->GetLoft();
    ASSERT_NE(loft, nullptr);
    EXPECT_EQ(loft->Name(), "tank1");
    EXPECT_EQ(loft->ShortName(), "T1");

    // Test return of UNKNOWN ShortName.
    dummyTank.SetUID("not_in_list");
    auto dummyTankLoft = dummyTank.GetLoft();
    EXPECT_EQ(dummyTankLoft->ShortName(), "UNKNOWN");

    // Test invalidation callback.
    dummyTank.Invalidate();
}

TEST_F(FuelTanks, vessels)
{
    EXPECT_EQ(vessels.GetVesselCount(), 2);
    EXPECT_EQ(vessels.GetVessel(1).GetDefaultedUID(), "tank1_outerVessel");
    EXPECT_EQ(vessels.GetVessel("tank1_outerVessel").GetDefaultedUID(), "tank1_outerVessel");
    EXPECT_EQ(vessels.GetVesselIndex("tank1_outerVessel"), 1);
    EXPECT_EQ(vessels.GetVessels().at(0)->GetDefaultedUID(), "tank1_outerVessel");

    CheckExceptionMessage([&]() { vessels.GetVessel("not_existing_vessel"); },
                          "Invalid UID in CPACSVessels::GetVessel. \"not_existing_vessel\" not found in CPACS file!");

    CheckExceptionMessage([&]() { vessels.GetVesselIndex("not_existing_vessel"); },
                          "Invalid UID in CPACSVessels::GetVesselIndex");

    CheckExceptionMessage([&]() { vessels.GetVessel(99); }, "Invalid index in std::vector<std::unique_ptr<CCPACSVessel>>::GetVessel");
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
    // Check whether vessels are built via segments or design parameters.
    EXPECT_TRUE(vessel_segments->IsVesselViaSegments());
    EXPECT_FALSE(vessel_segments->IsVesselViaDesignParameters());
    EXPECT_FALSE(vessel_spherical->IsVesselViaSegments());
    EXPECT_TRUE(vessel_spherical->IsVesselViaDesignParameters());

    // Check dome types.
    EXPECT_FALSE(vessel_segments->HasSphericalDome());
    EXPECT_FALSE(vessel_segments->HasEllipsoidDome());
    EXPECT_FALSE(vessel_segments->HasTorisphericalDome());
    EXPECT_FALSE(vessel_segments->HasIsotensoidDome());

    EXPECT_TRUE(vessel_spherical->HasSphericalDome());
    EXPECT_TRUE(vessel_spherical->HasEllipsoidDome());
    EXPECT_FALSE(vessel_spherical->HasTorisphericalDome());
    EXPECT_FALSE(vessel_spherical->HasIsotensoidDome());

    EXPECT_FALSE(vessel_ellipsoid1->HasSphericalDome());
    EXPECT_TRUE(vessel_ellipsoid1->HasEllipsoidDome());
    EXPECT_FALSE(vessel_ellipsoid1->HasTorisphericalDome());
    EXPECT_FALSE(vessel_ellipsoid1->HasIsotensoidDome());

    EXPECT_FALSE(vessel_torispherical->HasSphericalDome());
    EXPECT_FALSE(vessel_torispherical->HasEllipsoidDome());
    EXPECT_TRUE(vessel_torispherical->HasTorisphericalDome());
    EXPECT_FALSE(vessel_torispherical->HasIsotensoidDome());

    EXPECT_FALSE(vessel_isotensoid->HasSphericalDome());
    EXPECT_FALSE(vessel_isotensoid->HasEllipsoidDome());
    EXPECT_FALSE(vessel_isotensoid->HasTorisphericalDome());
    EXPECT_TRUE(vessel_isotensoid->HasIsotensoidDome());

    // Check for corrupt vessel exceptions.
    EXPECT_THROW(vessel_corrupt1->IsVesselViaSegments(), tigl::CTiglError);
    EXPECT_THROW(vessel_corrupt1->IsVesselViaDesignParameters(), tigl::CTiglError);
}

TEST_F(FuelTanks, vessel_sections)
{
    const tigl::CCPACSVessel* constSegments = vessel_segments;

    EXPECT_EQ(vessel_segments->GetSectionCount(), 3);
    EXPECT_EQ(vessel_parametric->GetSectionCount(), 0);

    EXPECT_NO_THROW(vessel_segments->GetSection(1));
    EXPECT_NO_THROW(constSegments->GetSection(1));
    EXPECT_NO_THROW(vessel_segments->GetSection("outerVessel_section1"));
    EXPECT_NO_THROW(constSegments->GetSection("outerVessel_section1"));
    EXPECT_THROW(vessel_segments->GetSection("outerVessel_notExistingSection"), tigl::CTiglError);

    CheckExceptionMessage([&]() { vessel_segments->GetSection(4); }, "Invalid index in CCPACSVessel::GetSection");
    CheckExceptionMessage([&]() { vessel_parametric->GetSection(2); }, tankTypeExceptionString);
    CheckExceptionMessage([&]() { vessel_parametric->GetSection("outerVessel_notExistingSection"); },
                          tankTypeExceptionString);

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

    const auto* const_vessel_parametric = vessel_parametric;
    EXPECT_NO_THROW(vessel_segments->GetSegment(1));
    CheckExceptionMessage([&]() { vessel_parametric->GetSegment(1); }, tankTypeExceptionString);
    CheckExceptionMessage([&]() { const_vessel_parametric->GetSegment(1); }, tankTypeExceptionString);
    CheckExceptionMessage([&]() { vessel_parametric->GetSegment("outerVessel_segment1"); }, tankTypeExceptionString);

    EXPECT_NO_THROW(vessel_segments->GetSegment("outerVessel_segment1"));
    CheckExceptionMessage([&]() { vessel_segments->GetSegment(3); },
                          "Invalid index in CCPACSVessel::GetSegment");
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

    EXPECT_EQ(vessel_segments->GetGuideCurvePoints().size(), 0);
}

TEST_F(FuelTanks, vessel_loft_evaluation)
{
    // Evaluate geometric properties.
    EXPECT_NEAR(vessel_segments->GetGeometricVolume(), 6.57, 1e-2);
    EXPECT_NEAR(vessel_spherical->GetGeometricVolume(), 18.1, 1e-2);
    EXPECT_NEAR(vessel_ellipsoid1->GetGeometricVolume(), 5.43, 1e-2);
    EXPECT_NEAR(vessel_torispherical->GetGeometricVolume(), 7.87, 1e-2);
    EXPECT_NEAR(vessel_isotensoid->GetGeometricVolume(), 9.01, 1e-2);

    EXPECT_NEAR(vessel_segments->GetSurfaceArea(), 19.93, 1e-2);
    EXPECT_NEAR(vessel_spherical->GetSurfaceArea(), 36.19, 1e-2);
    EXPECT_NEAR(vessel_ellipsoid1->GetSurfaceArea(), 16.22, 1e-2);
    EXPECT_NEAR(vessel_ellipsoid2->GetSurfaceArea(), 1.47, 1e-2);
    EXPECT_NEAR(vessel_torispherical->GetSurfaceArea(), 20.49, 1e-2);
    EXPECT_NEAR(vessel_isotensoid->GetSurfaceArea(), 22.40, 1e-2);

    EXPECT_NEAR(vessel_segments->GetCircumference(1, 0.5), 7.43, 1e-2);
    CheckExceptionMessage([&]() { vessel_parametric->GetCircumference(1, 0.5); }, tankTypeExceptionString);

    EXPECT_THROW(vessel_corrupt1->GetLoft(), tigl::CTiglError);
    CheckExceptionMessage([&]() { vessel_corrupt2->GetLoft(); }, "Parametric vessel specification incomplete: "
                                                       "cylinderRadius, cylinderLength and domeType required.");
    EXPECT_THROW(dummyVessel.GetLoft(), tigl::CTiglError);
}

TEST_F(FuelTanks, vessel_parametric_exceptions)
{
    CheckExceptionMessage([&]() { fuelTank_corrupt->GetVessels().GetVessel("vessel_corrupt_ellipsoid").GetLoft(); },
                          "Half axis fraction (-1.000000) of vessel \"Vessel with negative ax ratio\" "
                          "(uID=\"vessel_corrupt_ellipsoid\") must be a positive value!");

    CheckExceptionMessage(
        [&]() { fuelTank_corrupt->GetVessels().GetVessel("vessel_corrupt_torispherical1").GetLoft(); },
        "The dish radius (0.500000) of vessel \"Vessel with too small dish radius\" "
        "(uID=\"vessel_corrupt_torispherical1\") must be larger than the cylinder radius (1.000000)!");

    CheckExceptionMessage(
        [&]() { fuelTank_corrupt->GetVessels().GetVessel("vessel_corrupt_torispherical2").GetLoft(); },
        "The knuckle radius (1.500000) of vessel \"Vessel with too large knuckle radius\" "
        "(uID=\"vessel_corrupt_torispherical2\") must be larger than 0 and smaller than the cylinder Radius "
        "(1.000000)!");

    CheckExceptionMessage(
        [&]() { fuelTank_corrupt->GetVessels().GetVessel("vessel_corrupt_isotensoid").GetLoft(); },
        "The polar opening radius (1.500000) of vessel \"Vessel with too large polar opening radius\" "
        "(uID=\"vessel_corrupt_isotensoid\") must be larger than 0 and smaller than the cylinder radius (1.000000)!");

    CheckExceptionMessage(
        [&]() { fuelTank_corrupt->GetVessels().GetVessel("vessel_corrupt_negative_radius").GetLoft(); },
        "The cylinder radius (-1.000000) of vessel \"Vessel with negative radius\" "
        "(uID=\"vessel_corrupt_negative_radius\") must be larger than 0!");

    CheckExceptionMessage(
        [&]() { fuelTank_corrupt->GetVessels().GetVessel("vessel_corrupt_negative_length").GetLoft(); },
        "The cylinder length (-1.000000) of vessel \"Vessel with negative length\" "
        "(uID=\"vessel_corrupt_negative_length\") must be larger than or equal to 0!");
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

    auto parametric_loft = vessel_torispherical->GetLoft();
    EXPECT_EQ(parametric_loft->FaceTraits(0).Name(), "Dome");
    EXPECT_EQ(parametric_loft->FaceTraits(1).Name(), "Dome");
    EXPECT_EQ(parametric_loft->FaceTraits(2).Name(), "Cylinder");
    EXPECT_EQ(parametric_loft->FaceTraits(3).Name(), "Cylinder");
    EXPECT_EQ(parametric_loft->FaceTraits(4).Name(), "Dome");
    EXPECT_EQ(parametric_loft->FaceTraits(5).Name(), "Dome");
}

TEST_F(FuelTanks, structure)
{
    auto& structure_with_walls           = vessel_isotensoid->GetStructure();
    auto& structure_with_stringer_frames = vessel_segments->GetStructure();

    EXPECT_EQ(structure_with_stringer_frames->GetFrames()->GetFrames().size(), 1);
    EXPECT_EQ(structure_with_stringer_frames->GetStringers()->GetStringers().size(), 1);

    const auto& walls = structure_with_walls->GetWalls();
    EXPECT_EQ(walls->GetWallSegments().GetWallSegments().size(), 3);
    
    const auto& pos = walls->GetWallPosition("tank5_wall_p1");
    gp_Pnt p;
    ASSERT_NO_THROW(p = pos.GetBasePoint());
    EXPECT_DOUBLE_EQ(p.X(), 14.5);
    EXPECT_DOUBLE_EQ(p.Y(), -1);
    EXPECT_DOUBLE_EQ(p.Z(), -0.2);
}
