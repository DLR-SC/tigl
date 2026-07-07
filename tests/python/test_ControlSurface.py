#############################################################################
# Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
#
# Created: 2025-11-07 Ole Albers <ole.albers@dlr.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################

import unittest

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
from tigl3 import configuration


class ControlSurfaceTest(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi3()
        self.tigl = Tigl3()
        self.assertIsNone(self.tixi.open("TestData/simpletest-flaps.cpacs.xml"))
        self.assertIsNone(self.tigl.open(self.tixi, ""))

        mgr = configuration.CCPACSConfigurationManager.get_instance()
        self.config = mgr.get_configuration(self.tigl._handle.value)
        uid_mgr = self.config.get_uidmanager()

    def tearDown(self):
        self.tigl.close()
        self.tixi.close()


    def test_control_surface_api_methods_reachable(self):

        self.assertEqual("InnerLED", self.tigl.getLeadingEdgeDeviceUID("WING_CS1", 1))
        self.assertEqual("FlapInner", self.tigl.getTrailingEdgeDeviceUID("WING_CS1", 1))

        uid = "FlapInner"

        ctype = self.tigl.getControlSurfaceType(uid)
        self.assertEqual(ctype, 0)

        minparm = self.tigl.controlSurfaceGetMinimumControlParameter(uid)
        self.assertEqual(minparm, 0.0)

        maxparm = self.tigl.controlSurfaceGetMaximumControlParameter(uid)
        self.assertEqual(maxparm, 1.0)

        curparm = self.tigl.controlSurfaceGetControlParameter(uid)
        self.assertEqual(curparm, 0.0)

        self.tigl.controlSurfaceSetControlParameter(uid, 1.0)
        curparm2 = self.tigl.controlSurfaceGetControlParameter(uid)
        self.assertEqual(curparm2, 1.0)

    def test_control_surfaces(self):
        comp_seg = self.config.get_wing(1).get_component_segment("WING_CS1")

        ctrl_surfaces_opt = comp_seg.get_control_surfaces()
        self.assertTrue(bool(ctrl_surfaces_opt))
        ctrl_surfaces = ctrl_surfaces_opt

        teds_opt = ctrl_surfaces.get_trailing_edge_devices()
        self.assertTrue(bool(teds_opt))
        teds = teds_opt
        ted = teds.get_trailing_edge_device(1)
        self.assertEqual(ted.get_defaulted_uid(), "FlapInner")
        short_name = ted.get_short_name()
        self.assertEqual(short_name, "W1CS1TED0")

        leds_opt = ctrl_surfaces.get_leading_edge_devices()
        self.assertTrue(bool(leds_opt))
        leds = leds_opt
        led = leds.get_leading_edge_device(1)
        self.assertEqual(led.get_defaulted_uid(), "InnerLED")
        short_name = led.get_short_name()
        self.assertEqual(short_name, "W1CS1LED0")


if __name__ == "__main__":
    unittest.main()
