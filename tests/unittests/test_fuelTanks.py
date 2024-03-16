import unittest

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
from tigl3 import configuration

class FuselageTank(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi3()
        self.tigl = Tigl3()
        self.assertIsNone(self.tixi.open('TestData/simpletest.cpacs.xml'))
        self.assertIsNone(self.tigl.open(self.tixi, ''))
        
        mgr = configuration.CCPACSConfigurationManager_get_instance()
        uid_mgr = mgr.get_configuration(self.tigl._handle.value).get_uidmanager()
        fuselage = uid_mgr.get_geometric_component('SimpleFuselage')
        fuelTanks = fuselage.get_fuel_tanks()
        # help(fuselage)
        print(fuelTanks)
        
        
    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_tiglGetVersion(self):
        print('TIGL-Version: ' + self.tigl.version)
        
    def test_true(self):
        self.assertTrue(True)
        
if __name__ == '__main__':
    unittest.main()