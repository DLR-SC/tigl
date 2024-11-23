import unittest

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
from tigl3 import configuration

from OCC.Core.TopoDS import TopoDS_Face


class FuelTanks(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi3()
        self.tigl = Tigl3()
        self.assertIsNone(self.tixi.open("TestData/simpletest-fuelTanks.cpacs.xml"))
        self.assertIsNone(self.tigl.open(self.tixi, ""))

        mgr = configuration.CCPACSConfigurationManager.get_instance()
        self.config = mgr.get_configuration(self.tigl._handle.value)
        uid_mgr = self.config.get_uidmanager()

        self.fuelTank = uid_mgr.get_geometric_component("tank1")
        self.vessel_segments = uid_mgr.get_geometric_component("tank1_outerVessel")
        self.vessel_guides = uid_mgr.get_geometric_component("tank2_outerVessel")
        self.vessel_spherical = uid_mgr.get_geometric_component("tank3_sphericalDome")
        self.vessel_ellipsoid = uid_mgr.get_geometric_component("tank3_ellipsoidDome")
        self.vessel_torispherical = uid_mgr.get_geometric_component(
            "tank4_torisphericalDome"
        )
        self.vessel_isotensoid = uid_mgr.get_geometric_component("tank5_isotensoidDome")

        self.tank_type_exception_msg = (
            "This method is only available for vessels with segments. No segment found."
        )

    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_configuration(self):

        tank1_uID = "tank1"
        self.assertEqual(self.config.get_fuel_tanks_count(), 6)
        self.assertIsInstance(
            self.config.get_fuel_tank(1), configuration.CCPACSFuelTank
        )
        self.assertIsInstance(
            self.config.get_fuel_tank(tank1_uID),
            configuration.CCPACSFuelTank,
        )
        self.assertEqual(
            self.config.get_fuel_tank_index(tank1_uID),
            1,
        )
        self.assertIsInstance(
            self.config.get_fuel_tanks(), configuration.CCPACSFuelTanks
        )

    def test_fuelTanks(self):
        # Test custom class methods:
        fuelTanks = self.fuelTank.get_parent()
        tank1_uID = "tank1"

        ## Test accessability of childs:
        self.assertIsInstance(fuelTanks.get_fuel_tank(1), configuration.CCPACSFuelTank)
        self.assertEqual(fuelTanks.get_fuel_tank(1).get_defaulted_uid(), tank1_uID)
        self.assertIsInstance(
            fuelTanks.get_fuel_tank(tank1_uID),
            configuration.CCPACSFuelTank,
        )
        self.assertEqual(
            fuelTanks.get_fuel_tank_index(tank1_uID),
            1,
        )
        self.assertEqual(fuelTanks.get_fuel_tanks_count(), 6)

        # Test availability of generated class:
        self.assertIsInstance(fuelTanks, configuration.CCPACSFuelTanks)

    def test_genericFuelTank(self):
        # Test custom class methods:
        self.assertIsInstance(self.fuelTank.get_vessels(), configuration.CCPACSVessels)

        # Test availability of generated class:
        self.assertEqual(self.fuelTank.get_name(), "Simple tank 1")

    def test_vessels(self):
        # Test custom class methods:
        vessels = self.vessel_segments.get_parent()
        vessel1_uID = "tank1_outerVessel"

        ## Test accessability of childs:
        self.assertIsInstance(vessels.get_vessel(1), configuration.CCPACSVessel)
        self.assertEqual(vessels.get_vessel(1).get_defaulted_uid(), vessel1_uID)
        self.assertIsInstance(vessels.get_vessel(vessel1_uID), configuration.CCPACSVessel)
        self.assertEqual(
            vessels.get_vessel_index(vessel1_uID),
            1,
        )
        self.assertEqual(vessels.get_vessels_count(), 2)

        # Test availability of generated class:
        self.assertIsInstance(vessels.get_parent(), configuration.CCPACSFuelTank)

    def test_vessel_general(self):

        # Custom class methods
        self.assertEqual(
            self.vessel_segments.get_configuration().get_uid(), "testAircraft"
        )
        self.assertEqual(self.vessel_segments.get_defaulted_uid(), "tank1_outerVessel")

        # Generated class methods
        self.assertIsInstance(
            self.vessel_segments.get_configuration(),
            configuration.CCPACSConfiguration,
        )

        self.assertEqual(self.vessel_segments.get_name(), "Outer vessel")

    def test_vessel_type_info(self):

        self.assertTrue(self.vessel_segments.is_vessel_via_segments())
        self.assertFalse(self.vessel_segments.is_vessel_via_design_parameters())
        self.assertFalse(self.vessel_spherical.is_vessel_via_segments())
        self.assertTrue(self.vessel_spherical.is_vessel_via_design_parameters())

        self.assertFalse(self.vessel_segments.has_spherical_dome())
        self.assertFalse(self.vessel_segments.has_ellipsoid_dome())
        self.assertFalse(self.vessel_segments.has_torispherical_dome())
        self.assertFalse(self.vessel_segments.has_isotensoid_dome())

        self.assertTrue(self.vessel_spherical.has_spherical_dome())
        self.assertTrue(self.vessel_spherical.has_ellipsoid_dome())
        self.assertFalse(self.vessel_spherical.has_torispherical_dome())
        self.assertFalse(self.vessel_spherical.has_isotensoid_dome())

        self.assertFalse(self.vessel_ellipsoid.has_spherical_dome())
        self.assertTrue(self.vessel_ellipsoid.has_ellipsoid_dome())
        self.assertFalse(self.vessel_ellipsoid.has_torispherical_dome())
        self.assertFalse(self.vessel_ellipsoid.has_isotensoid_dome())

        self.assertFalse(self.vessel_torispherical.has_spherical_dome())
        self.assertFalse(self.vessel_torispherical.has_ellipsoid_dome())
        self.assertTrue(self.vessel_torispherical.has_torispherical_dome())
        self.assertFalse(self.vessel_torispherical.has_isotensoid_dome())

        self.assertFalse(self.vessel_isotensoid.has_spherical_dome())
        self.assertFalse(self.vessel_isotensoid.has_ellipsoid_dome())
        self.assertFalse(self.vessel_isotensoid.has_torispherical_dome())
        self.assertTrue(self.vessel_isotensoid.has_isotensoid_dome())

    def test_vessel_sections(self):
        vessel_segments = self.vessel_segments
        vessel_parametric = self.vessel_spherical

        self.assertEqual(vessel_segments.get_section_count(), 3)
        self.assertEqual(vessel_parametric.get_section_count(), 0)

        self.assertIsInstance(
            vessel_segments.get_section(1), configuration.CCPACSFuselageSection
        )
        with self.assertRaises(RuntimeError) as context:
            vessel_parametric.get_section(1)
        self.assertEqual(str(context.exception), self.tank_type_exception_msg)

        self.assertIsInstance(
            vessel_segments.get_section_face("outerVessel_section3"), TopoDS_Face
        )
        with self.assertRaises(RuntimeError) as context:
            vessel_parametric.get_section_face("outerVessel_section3")
        self.assertEqual(str(context.exception), self.tank_type_exception_msg)

    def test_vessel_segments(self):
        vessel_segments = self.vessel_segments
        vessel_parametric = self.vessel_spherical

        self.assertEqual(vessel_segments.get_segment_count(), 2)
        self.assertEqual(vessel_parametric.get_segment_count(), 0)

        self.assertIsInstance(
            vessel_segments.get_segment(1), configuration.CCPACSFuselageSegment
        )
        with self.assertRaises(RuntimeError) as context:
            vessel_parametric.get_segment(1)
        self.assertEqual(str(context.exception), self.tank_type_exception_msg)

    def test_vessel_guide_curves(self):
        point = self.vessel_guides.get_guide_curve_points()[1]
        self.assertAlmostEqual(round(point.X(), 2), 3.5, 1e-2)
        self.assertAlmostEqual(round(point.Y(), 2), 0.0, 1e-5)
        self.assertAlmostEqual(round(point.Z(), 2), -0.65, 1e-2)
        with self.assertRaises(RuntimeError) as context:
            self.vessel_spherical.get_guide_curve_points()[1]
        self.assertEqual(str(context.exception), self.tank_type_exception_msg)

        self.assertEqual(
            self.vessel_guides.get_guide_curve_segment(
                "tank2_seg1_upper"
            ).get_guide_curve_profile_uid(),
            "gc_upper",
        )
        with self.assertRaises(RuntimeError) as context:
            self.vessel_spherical.get_guide_curve_segment("tank2_seg1_upper")
        self.assertEqual(str(context.exception), self.tank_type_exception_msg)

    def test_vessel_loft_evaluation(self):
        vessel_segments = self.vessel_segments
        vessel_parametric = self.vessel_spherical

        self.assertAlmostEqual(round(vessel_segments.get_geometric_volume(), 2), 6.57)
        self.assertAlmostEqual(round(vessel_parametric.get_geometric_volume(), 2), 18.1)

        self.assertAlmostEqual(round(vessel_segments.get_surface_area(), 2), 11.15)
        self.assertAlmostEqual(round(vessel_parametric.get_surface_area(), 2), 36.19)

        self.assertAlmostEqual(round(vessel_segments.get_circumference(1, 0.5), 2), 7.43)
        with self.assertRaises(RuntimeError) as context:
            vessel_parametric.get_circumference(1, 0.5)
        self.assertEqual(str(context.exception), self.tank_type_exception_msg)

        point = vessel_segments.get_point(1, 0.5, 0.5)
        self.assertAlmostEqual(round(point.X(), 2), 1.54)
        self.assertAlmostEqual(round(point.Y(), 2), 0.0)
        self.assertAlmostEqual(round(point.Z(), 2), -1.2)
        with self.assertRaises(RuntimeError) as context:
            vessel_parametric.get_point(1, 0.5, 0.5)
        self.assertEqual(str(context.exception), self.tank_type_exception_msg)

        self.assertEqual(
            vessel_segments.get_get_point_behavior(), configuration.asParameterOnSurface
        )
        self.assertIsNone(
            vessel_segments.set_get_point_behavior(configuration.onLinearLoft)
        )
        self.assertEqual(
            vessel_segments.get_get_point_behavior(), configuration.onLinearLoft
        )

    def test_structure(self):
        structure = self.vessel_segments.get_structure()
        # Test custom class methods:
        self.assertIsInstance(
            structure.get_frames(), configuration.CCPACSFramesAssembly
        )

        # Test availability of generated class:
        self.assertEqual(structure.get_uid(), "outerVesselStructure")


if __name__ == "__main__":
    unittest.main()
