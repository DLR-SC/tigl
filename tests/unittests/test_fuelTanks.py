import unittest

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
from tigl3 import configuration

from OCC.Core.TopoDS import TopoDS_Face


class FuselageTank(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi3()
        self.tigl = Tigl3()
        self.assertIsNone(self.tixi.open("TestData/simpletest-fuelTanks.cpacs.xml"))
        self.assertIsNone(self.tigl.open(self.tixi, ""))

        mgr = configuration.CCPACSConfigurationManager_get_instance()
        uid_mgr = mgr.get_configuration(self.tigl._handle.value).get_uidmanager()

        self.fuelTank = uid_mgr.get_geometric_component("tank1")
        self.hull_with_segments = uid_mgr.get_geometric_component("tank1_outerHull")
        self.hull_with_guides = uid_mgr.get_geometric_component("tank2_outerHull")
        self.hull_with_spherical_dome = uid_mgr.get_geometric_component(
            "tank3_sphericalDome"
        )

    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

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
        self.assertIsInstance(self.fuelTank.get_hulls(), configuration.CCPACSHulls)

        # Test availability of generated class:
        self.assertEqual(self.fuelTank.get_name(), "Simple tank 1")

    def test_hulls(self):
        # Test custom class methods:
        hulls = self.hull_with_segments.get_parent()
        hull1_uID = "tank1_outerHull"

        ## Test accessability of childs:
        self.assertIsInstance(hulls.get_hull(1), configuration.CCPACSHull)
        self.assertEqual(hulls.get_hull(1).get_defaulted_uid(), hull1_uID)
        self.assertIsInstance(hulls.get_hull(hull1_uID), configuration.CCPACSHull)
        self.assertEqual(
            hulls.get_hull_index(hull1_uID),
            1,
        )
        self.assertEqual(hulls.get_hulls_count(), 2)

        # Test availability of generated class:
        self.assertIsInstance(hulls.get_parent(), configuration.CCPACSFuelTank)

    def test_hull(self):

        hull_segments = self.hull_with_segments
        hull_parametric = self.hull_with_spherical_dome
        exception_msg = "Method only available for hulls with segments."

        # Test custom class methods:
        self.assertEqual(hull_segments.get_section_count(), 3)
        self.assertEqual(hull_parametric.get_section_count(), 0)

        self.assertEqual(hull_segments.get_segment_count(), 2)
        self.assertEqual(hull_parametric.get_segment_count(), 0)

        self.assertAlmostEqual(round(hull_segments.get_volume(), 2), 6.57)
        self.assertAlmostEqual(round(hull_parametric.get_volume(), 2), 18.1)

        self.assertAlmostEqual(round(hull_segments.get_surface_area(), 2), 11.15)
        self.assertAlmostEqual(round(hull_parametric.get_surface_area(), 2), 36.19)

        self.assertAlmostEqual(round(hull_segments.get_circumference(1, 0.5), 2), 7.43)
        with self.assertRaises(RuntimeError) as context:
            hull_parametric.get_circumference(1, 0.5)
        self.assertEqual(str(context.exception), exception_msg)

        self.assertIsInstance(
            hull_segments.get_segment(1), configuration.CCPACSFuselageSegment
        )
        with self.assertRaises(RuntimeError) as context:
            hull_parametric.get_segment(1)
        self.assertEqual(str(context.exception), exception_msg)

        self.assertIsInstance(
            hull_segments.get_section(1), configuration.CCPACSFuselageSection
        )
        self.assertIsInstance(
            hull_segments.get_section_face("outerHull_section3"), TopoDS_Face
        )

        self.assertEqual(hull_segments.get_defaulted_uid(), "tank1_outerHull")

        self.assertIsInstance(
            hull_segments.get_configuration(),
            configuration.CCPACSConfiguration,
        )

        point = hull_segments.get_point(1, 0.5, 0.5)
        self.assertAlmostEqual(round(point.X(), 2), 1.54)
        self.assertAlmostEqual(round(point.Y(), 2), 0.0)
        self.assertAlmostEqual(round(point.Z(), 2), -1.2)

        # point = self.hull_with_guides.get_guide_curve_points()[1]
        # self.assertAlmostEqual(round(point.X(), 2), 2.75, 1e-2)
        # self.assertAlmostEqual(round(point.Y(), 2), 0.0, 1e-5)
        # self.assertAlmostEqual(round(point.Z(), 2), -0.24, 1e-2)

        self.assertEqual(
            self.hull_with_guides.get_guide_curve_segment(
                "tank2_seg1_upper"
            ).get_guide_curve_profile_uid(),
            "gc_upper",
        )

        # Test availability of generated class:
        self.assertEqual(self.hull_with_segments.get_name(), "Outer hull")

    def test_structure(self):
        structure = self.hull_with_segments.get_structure()
        # Test custom class methods:
        self.assertIsInstance(
            structure.get_frames(), configuration.CCPACSFramesAssembly
        )

        # Test availability of generated class:
        self.assertEqual(structure.get_uid(), "outerHullStructure")


if __name__ == "__main__":
    unittest.main()
