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

    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_hull(self):
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
        self.assertAlmostEqual(round(point.X(), 2), 2.54)
        self.assertAlmostEqual(round(point.Y(), 2), 0)
        self.assertAlmostEqual(round(point.Z(), 2), -0.3)


if __name__ == "__main__":
    unittest.main()
