import unittest

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
from tigl3 import configuration


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

    def test_fuelTank(self):
        print("Intend: " + self.tigl.version)

    def test_hull(self):
        self.assertEqual(self.hull.get_section_count(), 3)
        self.assertEqual(self.hull.get_segment_count(), 2)
        self.assertEqual(self.hull.get_volume(), 0.41085764204179498)
        self.assertEqual(self.hull.get_surface_area(), 2.7879599486116566)
        self.assertEqual(self.hull.get_circumference(1, 0.5), 1.8586415856382374)


if __name__ == "__main__":
    unittest.main()
