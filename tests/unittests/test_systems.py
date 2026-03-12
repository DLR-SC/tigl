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
        self.assertEqual(components.get_component_count(), 24)

        # Access to CCPACSComponent:
        component = components.get_component(1)
        self.assertIsNotNone(component)
        self.assertEqual(component.get_defaulted_uid(), "cuboid_1")

    def test_getGeometricComonent(self):

        # Check if CCPACSComponent is part of CTiglUIDManager %factory
        component = self.uid_mgr.get_geometric_component("cuboid_1")
        self.assertIsInstance(component, configuration.CCPACSComponent)

    def test_shapes(self):

        # Shape from CPACSVehicleElementBase
        cube = self.uid_mgr.get_geometric_component("cuboid_1")
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

        # Combined components
        combined = self.uid_mgr.get_geometric_component("combinedComponent")
        shape = combined.get_loft()
        self.assertEqual(shape.get_face_count(), 33)

        # Shape from external elements
        external = self.uid_mgr.get_geometric_component("external")
        shape = external.get_loft()
        self.assertEqual(shape.get_face_count(), 6)

        # Shape from multiSegmentShape
        mss = self.uid_mgr.get_geometric_component("multiSegmentComponent3")
        shape = mss.get_loft()
        self.assertEqual(shape.get_face_count(), 10)

    def test_system_mass(self):
        eps = 1e-6

        generic = self.uid_mgr.get_geometric_component("genSys_1")

        # ---- Mass values ----
        m_all = generic.get_mass_all_components()
        m_pos = generic.get_mass_positioned_components()

        self.assertAlmostEqual(m_all, 2.8995165, delta=eps)
        self.assertAlmostEqual(m_pos, 2.7761165, delta=eps)

        # ---- Center of gravity ----
        cog = generic.get_center_of_gravity()
        self.assertIsNotNone(cog)

        self.assertAlmostEqual(cog.x, 27.4124525, delta=eps)
        self.assertAlmostEqual(cog.y, 10.7533060, delta=eps)
        self.assertAlmostEqual(cog.z, 3.8039630, delta=eps)

    def test_component_mass(self):

        eps = 1e-6

        # --- Mass value ---
        cuboid = self.uid_mgr.get_geometric_component("cuboid_2")
        m = cuboid.get_mass()
        self.assertIsNotNone(m)
        self.assertAlmostEqual(m, 0.375, delta=eps)

        cuboid3 = self.uid_mgr.get_geometric_component("cuboid_3")
        m3 = cuboid3.get_mass()
        self.assertIsNotNone(m3)
        self.assertAlmostEqual(m, m3, delta=eps)

        # wedge_1: no mass definition -> None
        wedge = self.uid_mgr.get_geometric_component("wedge_1")
        self.assertIsNone(wedge.get_mass())

        # external mass
        external = self.uid_mgr.get_geometric_component("external")
        m_ext = external.get_mass()
        self.assertIsNotNone(m_ext)
        self.assertAlmostEqual(m_ext, 0.2476386, delta=eps)

        # --- CoG local/global ---
        rect4 = self.uid_mgr.get_geometric_component("cuboid_4")

        cog_local = rect4.get_center_of_gravity_local()
        self.assertIsNotNone(cog_local)
        self.assertAlmostEqual(cog_local.x, 0.3, delta=eps)
        self.assertAlmostEqual(cog_local.y, 0.25, delta=eps)
        self.assertAlmostEqual(cog_local.z, 0.4, delta=eps)

        # global CoG available since <transformation> is set
        self.assertTrue(rect4.is_positioned())
        cog_global = rect4.get_center_of_gravity_global()
        self.assertIsNotNone(cog_global)

        self.assertAlmostEqual(cog_global.x, -0.0707107, delta=eps)
        self.assertAlmostEqual(cog_global.y, 15.25, delta=eps)
        self.assertAlmostEqual(cog_global.z, 0.49497475, delta=eps)

        # global CoG not available since <transformation> is not set
        unpos = self.uid_mgr.get_geometric_component("unpositionedCuboid")

        self.assertFalse(unpos.is_positioned())
        self.assertIsNotNone(unpos.get_center_of_gravity_local())
        self.assertIsNone(unpos.get_center_of_gravity_global())

        # --- MassInertia local ---
        cuboid1 = self.uid_mgr.get_geometric_component("cuboid_1")
        mi = cuboid1.get_mass_inertia_local()
        self.assertIsNotNone(mi)

        self.assertEqual(mi.Jxx, 1)
        self.assertEqual(mi.Jyy, 2)
        self.assertEqual(mi.Jzz, 3)
        self.assertIsNone(mi.Jxy)
        self.assertIsNone(mi.Jxz)
        self.assertIsNone(mi.Jyz)

        cuboid2 = self.uid_mgr.get_geometric_component("cuboid_2")
        mi = cuboid2.get_mass_inertia_local()
        self.assertIsNone(mi)

    def test_component_representation(self):

        # explicitly physical
        cuboid1 = self.uid_mgr.get_geometric_component("cuboid_1")
        self.assertEqual(cuboid1.get_component_representation(), 1)
        self.assertEqual(cuboid1.get_component_representation_as_string(), "physical")

        # explicitly envelope
        cuboid2 = self.uid_mgr.get_geometric_component("cuboid_2")
        self.assertEqual(cuboid2.get_component_representation(), 2)
        self.assertEqual(cuboid2.get_component_representation_as_string(), "envelope")

        # missing attribute -> default must be physical
        wedge1 = self.uid_mgr.get_geometric_component("wedge_1")
        self.assertEqual(wedge1.get_component_representation(), 1)
        self.assertEqual(wedge1.get_component_representation_as_string(), "physical")

    def test_enums(self):

        # Systems
        system = self.uid_mgr.get_geometric_component("genSys_1")
        self.assertEqual(system.get_component_intent(), 1)
        self.assertEqual(system.get_component_type(), 12)

        # Components
        component = self.uid_mgr.get_geometric_component("cuboid_1")
        self.assertEqual(component.get_component_intent(), 1)
        self.assertEqual(component.get_component_type(), 29)

    def test_systemArchitectures(self):
        self.assertEqual(self.config.get_system_architectures_count(), 1)
        sa = self.config.get_system_architecture(1)
        self.assertEqual(sa.get_name(), "Test system architecture")


if __name__ == "__main__":
    unittest.main()
