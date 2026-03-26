from tixi3.tixi3wrapper import Tixi3
from tigl3.tigl3wrapper import Tigl3
import tigl3.configuration
import unittest
import os
import sys
from OCC.Core.TopoDS import TopoDS_Edge
from OCC.Core.gp import gp_Vec2d
from tigl3.configuration import NACA4Calculator


class TestNACA(unittest.TestCase):
    def setUp(self):
        self.tixi_h = Tixi3()
        self.tigl_h = Tigl3()
        self.assertIsNone(self.tixi_h.open("naca_test.cpacs.xml"))
        self.assertIsNone(self.tigl_h.open(self.tixi_h, ""))
        mgr = tigl3.configuration.CCPACSConfigurationManager_get_instance()
        self.aircraft_config = mgr.get_configuration(self.tigl_h._handle.value)
        

    def tearDown(self):
        self.tigl_h.close()
        self.tixi_h.close()

    def test_nacaAircraftElements_0012(self):
        profiles = self.aircraft_config.get_wing_profiles()
        self.wing_profile = profiles.get_profile("NACA0012")
        choice = self.wing_profile.get_naca_profile_choice4()
        assert isinstance(self.wing_profile.get_naca_profile_choice4(), tigl3.configuration.CPACSNacaProfile)
        upper_wire = self.wing_profile.get_upper_wire()
        assert isinstance(upper_wire, TopoDS_Edge)
        lower_wire = self.wing_profile.get_lower_wire()
        assert isinstance(lower_wire, TopoDS_Edge)
        trailing_edge = self.wing_profile.get_trailing_edge()
        assert isinstance(trailing_edge, TopoDS_Edge)

        upper_point = self.wing_profile.get_upper_point(1.0)
        point = upper_point.Z()
        self.assertAlmostEqual(point, 0, delta=1e-6)

        calculator = tigl3.configuration.NACA4Calculator(0,0,9, 0.000945)
        assert isinstance(calculator, NACA4Calculator)
        calculator_upper_curve = calculator.upper_curve(0.5)
        assert isinstance(calculator_upper_curve, gp_Vec2d)
        
if __name__ == "__main__":
    unittest.main()