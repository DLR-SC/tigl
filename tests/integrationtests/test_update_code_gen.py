import unittest

from tixi3.tixi3wrapper import Tixi3
from tigl3.tigl3wrapper import Tigl3

from tigl3 import configuration

from tigl3.import_export_helper import export_shapes

class SimpleTest(unittest.TestCase):

	def setUp(self):
		self.tixi = Tixi3()
		self.tigl = Tigl3()
		self.assertIsNone(self.tixi.open("TestData/simpletest.cpacs.xml"))
		self.assertIsNone(self.tigl.open(self.tixi,""))
		
		mgr = configuration.CCPACSConfigurationManager_get_instance()
		self.config = mgr.get_configuration(self.tigl._handle.value)		
		uid_mgr = self.config.get_uidmanager()
		
	def tearDown(self):
		self.tigl.close()
		self.tixi.close()
		
	def test_fuselages(self):
		self.assertEqual(self.config.get_fuselages().get_fuselage_count(),1)
		self.assertEqual(self.config.get_fuselages().get_fuselage_index("SimpleFuselage"), 1)
		self.assertIsInstance(self.config.get_fuselages(), configuration.CPACSFuselages)
		self.assertIsInstance(self.config.get_fuselages().get_fuselage(1), configuration.CCPACSFuselage)
		with self.assertRaises(IndexError):
			self.config.get_fuselages().get_fuselage(0)
			self.config.get_fuselages().get_fuselage(2)


class SimpleTestRotor(unittest.TestCase):

	def setUp(self):
		self.tixi = Tixi3()
		self.tigl = Tigl3()
		self.assertIsNone(self.tixi.open("TestData/simple_test_rotors.cpacs.xml"))
		self.assertIsNone(self.tigl.open(self.tixi,""))
		
		mgr = configuration.CCPACSConfigurationManager_get_instance()
		self.config = mgr.get_configuration(self.tigl._handle.value)		
		uid_mgr = self.config.get_uidmanager()
		
		
	def tearDown(self):
		self.tigl.close()
		self.tixi.close()
		
	def test_rotors(self):
		self.assertIsInstance(self.config.get_rotor(1), configuration.CPACSRotor)
		self.assertIsInstance(self.config.get_rotor(1).get_parent(), configuration.CCPACSRotors)
		self.assertIsInstance(self.config.get_rotor(1).get_parent().get_rotor(1), configuration.CCPACSRotor)
		self.assertIsInstance(self.config.get_rotor(1).get_parent().get_rotor("MainRotor2"), configuration.CCPACSRotor)
		self.assertEqual(self.config.get_rotor(1).get_parent().get_rotor_count(), 4)
		self.assertEqual(self.config.get_rotor(1).get_parent().get_rotor_index("MainRotor2"), 2)
		with self.assertRaises(IndexError):
			#test function in CCPACSConfiguration
			self.config.get_rotor(0)
			self.config.get_rotor(5)
			#test function in CCPACSRotors
			self.config.get_rotor(1).get_parent().get_rotor(0)
			self.config.get_rotor(1).get_parent().get_rotor(5)
			
if __name__== "__main__":
	unittest.main()

