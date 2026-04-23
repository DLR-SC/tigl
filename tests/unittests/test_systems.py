#############################################################################
# Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
#
# Licensed under the Apache License, Version 2.0
#############################################################################

import unittest

from tigl3.tigl3wrapper import *
from tixi3.tixi3wrapper import *
from tigl3 import configuration, geometry


class SystemsBindings(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.tixi = Tixi3()
        cls.tigl = Tigl3()

        cls.assertIsNone = staticmethod(unittest.TestCase().assertIsNone)
        cls.assertIsNone(cls.tixi.open("TestData/simpletest-systems.cpacs.xml"))
        cls.assertIsNone(cls.tigl.open(cls.tixi, "testAircraft"))

        mgr = configuration.CCPACSConfigurationManager.get_instance()
        cls.config = mgr.get_configuration(cls.tigl._handle.value)
        cls.uid_mgr = cls.config.get_uidmanager()

    @classmethod
    def tearDownClass(cls):
        cls.tigl.close()
        cls.tixi.close()

    def test_uid_manager_factory_returns_concrete_types(self):
        system = self.uid_mgr.get_geometric_component("genSys_1")
        component = self.uid_mgr.get_geometric_component("cuboid_1")

        self.assertIsInstance(system, geometry.ITiglGeometricComponent)
        self.assertIsInstance(system, configuration.CCPACSGenericSystem)

        self.assertIsInstance(component, geometry.ITiglGeometricComponent)
        self.assertIsInstance(component, configuration.CCPACSComponent)

    def test_system_accessors(self):
        system = self.uid_mgr.get_geometric_component("genSys_1")

        self.assertEqual(system.get_uid(), "genSys_1")
        self.assertEqual(system.get_defaulted_uid(), "genSys_1")
        self.assertEqual(system.get_name(), "Generic System 1")
        self.assertIsInstance(
            system.get_configuration(), configuration.CCPACSConfiguration
        )

        components = system.get_components()
        self.assertIsNotNone(components)
        self.assertEqual(components.get_component_count(), 24)
        self.assertIsInstance(
            components.get_component(1), configuration.CCPACSComponent
        )

        # curated Python API: generated plural getter is intentionally hidden
        with self.assertRaises(AttributeError):
            components.get_components()

        # smoke-check wrapped system methods without reproducing C++ numerics
        self.assertIsInstance(system.get_mass_all_components(), float)
        self.assertIsInstance(system.get_mass_positioned_components(), float)
        self.assertIsInstance(system.get_center_of_gravity(), geometry.CTiglPoint)

        shape = system.get_loft()
        self.assertIsNotNone(shape)
        self.assertGreater(shape.get_face_count(), 0)

    def test_component_accessors(self):
        component = self.uid_mgr.get_geometric_component("cuboid_2")

        self.assertIsInstance(component.get_component_intent(), int)
        self.assertIsInstance(component.get_component_type(), int)
        self.assertIsInstance(component.get_component_representation(), int)
        self.assertEqual(component.get_component_representation_as_string(), "envelope")

        self.assertIsInstance(component.get_mass(), float)
        self.assertTrue(component.is_positioned())
        self.assertIsInstance(
            component.get_center_of_gravity_local(), geometry.CTiglPoint
        )
        self.assertIsInstance(
            component.get_center_of_gravity_global(), geometry.CTiglPoint
        )

        self.assertIsInstance(component.get_centroid_local(), geometry.CTiglPoint)
        self.assertIsInstance(component.get_centroid_global(), geometry.CTiglPoint)

        # optional mass inertia -> None in Python
        self.assertIsNone(component.get_mass_inertia_local())

        shape = component.get_loft()
        self.assertIsNotNone(shape)
        self.assertGreater(shape.get_face_count(), 0)

    def test_none_for_missing_optional_values(self):
        wedge = self.uid_mgr.get_geometric_component("wedge_1")
        self.assertIsNone(wedge.get_mass())

        unpositioned = self.uid_mgr.get_geometric_component("unpositionedCuboid")
        self.assertFalse(unpositioned.is_positioned())
        self.assertIsNotNone(unpositioned.get_center_of_gravity_local())
        self.assertIsNone(unpositioned.get_center_of_gravity_global())

        cuboid1 = self.uid_mgr.get_geometric_component("cuboid_1")
        inertia = cuboid1.get_mass_inertia_local()
        self.assertIsInstance(inertia, configuration.CTiglMassInertia)
        self.assertIsNone(inertia.Jxy)
        self.assertIsNone(inertia.Jxz)
        self.assertIsNone(inertia.Jyz)

    def test_system_architecture_access(self):
        self.assertEqual(self.config.get_system_architectures_count(), 1)

        sa = self.config.get_system_architecture(1)
        self.assertEqual(sa.get_name(), "Test system architecture")

        # overload resolution (index vs uid)
        sa_by_uid = self.config.get_system_architecture("systemArchitecture1")
        self.assertEqual(sa_by_uid.get_name(), "Test system architecture")

        connections = sa.get_connections()
        self.assertIsNotNone(connections)
        self.assertEqual(connections.get_connection_count(), 5)

    def test_system_architecture_connection_typemaps(self):
        sa = self.config.get_system_architecture(1)
        connections = sa.get_connections()

        # component -> component
        c1 = connections.get_connection(1)
        self.assertIsInstance(c1.get_source_component(), configuration.CCPACSComponent)
        self.assertIsInstance(c1.get_target_component(), configuration.CCPACSComponent)

        # component -> fuselage: wrapped component accessor must return None
        c3 = connections.get_connection(3)
        self.assertIsInstance(c3.get_source_component(), configuration.CCPACSComponent)
        self.assertIsNone(c3.get_target_component())
        self.assertEqual(c3.get_target().get_component_uid_choice4(), "SimpleFuselage")

        # fuselage -> externalElement: optional enum should be available
        c4 = connections.get_connection(4)
        self.assertIsNone(c4.get_source_component())
        self.assertIsNone(c4.get_target_component())
        self.assertEqual(c4.get_target().get_external_element_choice1(), 0)

    def test_component_sequence_exposure(self):
        sa = self.config.get_system_architecture(1)
        components = sa.get_generic_system_components()

        self.assertEqual(len(components), 3)
        self.assertTrue(
            all(isinstance(c, configuration.CCPACSComponent) for c in components)
        )
        self.assertEqual(
            [c.get_defaulted_uid() for c in components],
            ["cuboid_1", "cuboid_2", "cuboid_3"],
        )


if __name__ == "__main__":
    unittest.main()
