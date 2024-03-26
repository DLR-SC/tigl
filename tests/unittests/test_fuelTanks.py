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

        self.fuelTank = uid_mgr.get_geometric_component("genericTank1")
        self.hull = uid_mgr.get_geometric_component("outerHull")
        self.hull_with_guides = uid_mgr.get_geometric_component(
            "genericTank2_outerHull"
        )

    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_genericFuelTanks(self):
        # Test custom class methods:
        fuelTanks = self.fuelTank.get_parent()
        tank1_uID = "genericTank1"

        ## Test accessability of childs:
        self.assertIsInstance(
            fuelTanks.get_generic_fuel_tank(1), configuration.CCPACSGenericFuelTank
        )
        self.assertEqual(
            fuelTanks.get_generic_fuel_tank(1).get_defaulted_uid(), tank1_uID
        )
        self.assertIsInstance(
            fuelTanks.get_generic_fuel_tank(tank1_uID),
            configuration.CCPACSGenericFuelTank,
        )
        self.assertEqual(
            fuelTanks.get_generic_fuel_tank_index(tank1_uID),
            1,
        )
        self.assertEqual(fuelTanks.get_generic_fuel_tanks_count(), 2)

        # Test availability of generated class:
        self.assertIsInstance(
            fuelTanks.get_parent(), configuration.CPACSFuselageFuelTanks
        )

    def test_genericFuelTank(self):
        # Test custom class methods:
        self.assertTrue(self.fuelTank.has_hulls(), True)
        self.assertIsInstance(self.fuelTank.get_hulls(), configuration.CCPACSHulls)

        # Test availability of generated class:
        self.assertEqual(self.fuelTank.get_name(), "Simple tank")

    def test_hulls(self):
        # Test custom class methods:
        hulls = self.hull.get_parent()
        hull1_uID = "outerHull"

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
        self.assertIsInstance(hulls.get_parent(), configuration.CCPACSGenericFuelTank)

    def test_hull(self):
        # Test custom class methods:
        self.assertEqual(self.hull.get_section_count(), 3)
        self.assertEqual(self.hull.get_segment_count(), 2)
        self.assertAlmostEqual(round(self.hull.get_volume(), 2), 0.41)
        self.assertAlmostEqual(round(self.hull.get_surface_area(), 2), 2.79)
        self.assertAlmostEqual(round(self.hull.get_circumference(1, 0.5), 2), 1.86)

        self.assertEqual(self.hull.get_segment_count(), 2)
        self.assertIsInstance(
            self.hull.get_segment(1), configuration.CCPACSFuselageSegment
        )

        self.assertEqual(self.hull.get_section_count(), 3)
        self.assertIsInstance(
            self.hull.get_section(1), configuration.CCPACSFuselageSection
        )
        self.assertIsInstance(
            self.hull.get_section_face("outerHull_section3"), TopoDS_Face
        )

        self.assertEqual(self.hull.get_defaulted_uid(), "outerHull")

        self.assertIsInstance(
            self.hull.get_configuration(), configuration.CCPACSConfiguration
        )

        point = self.hull.get_point(1, 0.5, 0.5)
        self.assertAlmostEqual(round(point.X(), 2), 1.34)
        self.assertAlmostEqual(round(point.Y(), 2), 0)
        self.assertAlmostEqual(round(point.Z(), 2), -0.3)

        point = self.hull_with_guides.get_guide_curve_points()[1]
        self.assertAlmostEqual(round(point.X(), 2), 2.75, 1e-2)
        self.assertAlmostEqual(round(point.Y(), 2), 0.0, 1e-5)
        self.assertAlmostEqual(round(point.Z(), 2), -0.24, 1e-2)

        self.assertEqual(
            self.hull_with_guides.get_guide_curve_segment(
                "genericTank2_seg1_upper"
            ).get_guide_curve_profile_uid(),
            "gc_upper",
        )

        # Test availability of generated class:
        self.assertEqual(self.hull.get_name(), "Outer hull")

    def test_structure(self):
        structure = self.hull.get_structure()
        # Test custom class methods:
        self.assertIsInstance(
            structure.get_frames(), configuration.CCPACSFramesAssembly
        )

        # Test availability of generated class:
        self.assertEqual(structure.get_uid(), "outerHullStructure")


if __name__ == "__main__":
    unittest.main()
