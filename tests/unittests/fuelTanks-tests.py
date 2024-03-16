import unittest

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *

class TestFuelTanks(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi3()
        self.tigl = Tigl3()
        self.tixi.open('TestData/simpletest.cpacs.xml')
        self.tigl.open(self.tixi, '')
        
    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_tiglGetVersion(self):
        print('TIGL-Version: ' + self.tigl.version)
        
if __name__ == '__main__':
    unittest.main()