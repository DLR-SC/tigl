#############################################################################
# Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
#
# Created: 2026-01-30 Marko Alder <marko.alder@dlr.de>
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

from OCC.Core.Bnd import Bnd_Box
from OCC.Core.BRepBndLib import brepbndlib_Add


class Systems(unittest.TestCase):

    def setUp(self):
        self.tixi = Tixi3()
        self.tigl = Tigl3()
        self.assertIsNone(self.tixi.open("TestData/simpletest-systems.cpacs.xml"))
        self.assertIsNone(self.tigl.open(self.tixi, ""))

        mgr = configuration.CCPACSConfigurationManager.get_instance()
        self.config = mgr.get_configuration(self.tigl._handle.value)
        self.uid_mgr = self.config.get_uidmanager()

    def tearDown(self):
        self.tigl.close()
        self.tixi.close()

    def test_accessability(self):

        genSys = self.uid_mgr.get_geometric_component("genSys_1")
        self.assertIsNotNone(genSys)

        # Access to CPACSGenericSystem
        self.assertEqual(genSys.get_name(), "Generic System 1")
        self.assertEqual(genSys.get_uid(), "genSys_1")

        # Access to CCPACSGenericSystem
        self.assertEqual(genSys.get_defaulted_uid(), "genSys_1")

        # Access to CPACSComponents:
        components = genSys.get_components()
        self.assertIsNotNone(components)
        self.assertEqual(components.get_component_count(), 18)

        # Access to CCPACSComponent:
        component = components.get_component(1)
        self.assertIsNotNone(component)
        self.assertEqual(component.get_defaulted_uid(), "rectCube_1")

    def test_shapes(self):

        # Shape from CPACSVehicleElementBase
        cube = self.uid_mgr.get_geometric_component("rectCube_1")
        shape = cube.get_loft()

        self.assertIsNotNone(shape)
        self.assertEqual(shape.get_face_count(), 6)

        box = Bnd_Box()
        brepbndlib_Add(shape.shape(), box)
        xmin, ymin, zmin, xmax, ymax, zmax = box.Get()
        eps = 1e-6
        self.assertAlmostEqual(xmax - xmin, 1.0, delta=eps)
        self.assertAlmostEqual(ymax - ymin, 1.0, delta=eps)
        self.assertAlmostEqual(zmax - zmin, 1.0, delta=eps)

        # Shape from explicit elements
        emotor = self.uid_mgr.get_geometric_component("electricMotor")
        shape = emotor.get_loft()
        self.assertEqual(shape.get_face_count(), 1)

        # Shape from external elements
        external = self.uid_mgr.get_geometric_component("external")
        shape = external.get_loft()
        self.assertEqual(shape.get_face_count(), 6)


if __name__ == "__main__":
    unittest.main()
