#############################################################################
# Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
#
# Licensed under the Apache License, Version 2.0
#############################################################################

from pathlib import Path
import unittest

from tigl3 import configuration, geometry
from tigl3.tigl3wrapper import Tigl3
from tixi3.tixi3wrapper import Tixi3


class TestSystemsBindings(unittest.TestCase):
    CPACS_FILE = Path("TestData/simpletest-systems.cpacs.xml")
    CONFIGURATION_UID = "testAircraft"

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()

        if not cls.CPACS_FILE.is_file():
            raise FileNotFoundError(f"File not found: {cls.CPACS_FILE.resolve()}")

        cls.tixi = Tixi3()
        cls.tigl = Tigl3()
        cls._tixi_is_open = False
        cls._tigl_is_open = False

        cls.addClassCleanup(cls._close_configuration)

        tixi_result = cls.tixi.open(str(cls.CPACS_FILE))
        if tixi_result is not None:
            raise AssertionError(f"Tixi3.open() unexpectedly returned {tixi_result!r}")
        cls._tixi_is_open = True

        tigl_result = cls.tigl.open(cls.tixi, cls.CONFIGURATION_UID)
        if tigl_result is not None:
            raise AssertionError(f"Tigl3.open() unexpectedly returned {tigl_result!r}")
        cls._tigl_is_open = True

        manager = configuration.CCPACSConfigurationManager.get_instance()
        cls.aircraft_config = manager.get_configuration(cls.tigl._handle.value)
        cls.uid_manager = cls.aircraft_config.get_uidmanager()

    @classmethod
    def _close_configuration(cls) -> None:
        try:
            if cls._tigl_is_open:
                cls.tigl.close()
        finally:
            if cls._tixi_is_open:
                cls.tixi.close()

    def test_factory_types(self) -> None:
        system = self.uid_manager.get_geometric_component("genSys")
        component = self.uid_manager.get_geometric_component("cuboid_1")

        self.assertIsInstance(
            system,
            geometry.ITiglGeometricComponent,
        )
        self.assertIsInstance(
            system,
            configuration.CCPACSGenericSystem,
        )
        self.assertIsInstance(
            component,
            geometry.ITiglGeometricComponent,
        )
        self.assertIsInstance(
            component,
            configuration.CCPACSComponent,
        )

    def test_system_accessors(self) -> None:
        system = self.uid_manager.get_geometric_component("genSys")

        self.assertEqual(system.get_uid(), "genSys")
        self.assertEqual(system.get_defaulted_uid(), "genSys")
        self.assertEqual(system.get_name(), "Generic System")
        self.assertIsInstance(
            system.get_configuration(),
            configuration.CCPACSConfiguration,
        )

        components = system.get_components()

        self.assertIsNotNone(components)
        self.assertEqual(components.get_component_count(), 24)
        self.assertIsInstance(
            components.get_component(1),
            configuration.CCPACSComponent,
        )

        # The generated plural getter is intentionally hidden from Python.
        with self.assertRaises(AttributeError):
            components.get_components()

        self.assertIsInstance(
            system.get_mass_all_components(),
            float,
        )
        self.assertIsInstance(
            system.get_mass_positioned_components(),
            float,
        )
        self.assertIsInstance(
            system.get_center_of_gravity(),
            geometry.CTiglPoint,
        )

        shape = system.get_loft()

        self.assertIsNotNone(shape)
        self.assertGreater(shape.get_face_count(), 0)

    def test_component_accessors(self) -> None:
        component = self.uid_manager.get_geometric_component("cuboid_2")

        self.assertIsInstance(
            component.get_component_intent(),
            int,
        )
        self.assertIsInstance(
            component.get_component_type(),
            int,
        )
        self.assertIsInstance(
            component.get_component_representation(),
            int,
        )
        self.assertEqual(
            component.get_component_representation_as_string(),
            "envelope",
        )

        self.assertIsInstance(component.get_mass(), float)
        self.assertTrue(component.is_positioned())

        self.assertIsInstance(
            component.get_center_of_gravity_local(),
            geometry.CTiglPoint,
        )
        self.assertIsInstance(
            component.get_center_of_gravity_global(),
            geometry.CTiglPoint,
        )
        self.assertIsInstance(
            component.get_centroid_local(),
            geometry.CTiglPoint,
        )
        self.assertIsInstance(
            component.get_centroid_global(),
            geometry.CTiglPoint,
        )

        self.assertIsNone(component.get_mass_inertia_local())

        shape = component.get_loft()

        self.assertIsNotNone(shape)
        self.assertGreater(shape.get_face_count(), 0)

    def test_missing_optional_values(self) -> None:
        wedge = self.uid_manager.get_geometric_component("wedge_1")

        self.assertIsNone(wedge.get_mass())

        unpositioned = self.uid_manager.get_geometric_component("unpositionedCuboid")

        self.assertFalse(unpositioned.is_positioned())
        self.assertIsNotNone(unpositioned.get_center_of_gravity_local())
        self.assertIsNone(unpositioned.get_center_of_gravity_global())

        cuboid = self.uid_manager.get_geometric_component("cuboid_1")
        inertia = cuboid.get_mass_inertia_local()

        self.assertIsInstance(
            inertia,
            configuration.CTiglMassInertia,
        )
        self.assertIsNone(inertia.Jxy)
        self.assertIsNone(inertia.Jxz)
        self.assertIsNone(inertia.Jyz)

    def test_empty_system(self) -> None:
        system = self.uid_manager.get_geometric_component("genSys_empty")

        self.assertIsInstance(
            system,
            configuration.CCPACSGenericSystem,
        )
        self.assertIsNone(system.get_components())
        self.assertIsNone(system.get_center_of_gravity())
        self.assertIsNotNone(system.get_loft())

    def test_architecture_access(self) -> None:
        self.assertEqual(
            self.aircraft_config.get_system_architectures_count(),
            1,
        )

        architecture = self.aircraft_config.get_system_architecture(1)

        self.assertEqual(
            architecture.get_name(),
            "Test system architecture",
        )

        architecture_by_uid = self.aircraft_config.get_system_architecture(
            "systemArchitecture1"
        )

        self.assertEqual(
            architecture_by_uid.get_name(),
            "Test system architecture",
        )

        connections = architecture.get_connections()

        self.assertIsNotNone(connections)
        self.assertEqual(connections.get_connection_count(), 5)

    def test_connection_typemaps(self) -> None:
        architecture = self.aircraft_config.get_system_architecture(1)
        connections = architecture.get_connections()

        component_connection = connections.get_connection(1)

        self.assertIsInstance(
            component_connection.get_source_component(),
            configuration.CCPACSComponent,
        )
        self.assertIsInstance(
            component_connection.get_target_component(),
            configuration.CCPACSComponent,
        )

        fuselage_connection = connections.get_connection(3)

        self.assertIsInstance(
            fuselage_connection.get_source_component(),
            configuration.CCPACSComponent,
        )
        self.assertIsNone(fuselage_connection.get_target_component())
        self.assertEqual(
            fuselage_connection.get_target().get_component_uid_choice4(),
            "SimpleFuselage",
        )

        external_connection = connections.get_connection(4)

        self.assertIsNone(external_connection.get_source_component())
        self.assertIsNone(external_connection.get_target_component())
        self.assertEqual(
            external_connection.get_target().get_external_element_choice1(),
            0,
        )

    def test_component_sequence(self) -> None:
        architecture = self.aircraft_config.get_system_architecture(1)
        components = architecture.get_generic_system_components()

        self.assertEqual(len(components), 3)
        self.assertTrue(
            all(
                isinstance(component, configuration.CCPACSComponent)
                for component in components
            )
        )
        self.assertSequenceEqual(
            [component.get_defaulted_uid() for component in components],
            ["cuboid_1", "cuboid_2", "cuboid_3"],
        )


if __name__ == "__main__":
    unittest.main()
