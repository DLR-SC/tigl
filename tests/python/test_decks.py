#############################################################################
# Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
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

from pathlib import Path
import unittest

from tigl3 import configuration, geometry
from tigl3.tigl3wrapper import Tigl3
from tixi3.tixi3wrapper import Tixi3


class TestDeckBindings(unittest.TestCase):
    CPACS_FILE = Path("TestData/simpletest-decks.cpacs.xml")
    CONFIGURATION_UID = "testAircraft"

    DECK_UID = "deck"
    COMPONENT_UID = "classDivider"

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

        cls.deck = cls.uid_manager.get_geometric_component(cls.DECK_UID)
        cls.component = cls.uid_manager.get_geometric_component(cls.COMPONENT_UID)

    @classmethod
    def _close_configuration(cls) -> None:
        try:
            if cls._tigl_is_open:
                cls.tigl.close()
        finally:
            if cls._tixi_is_open:
                cls.tixi.close()

    def test_factory_types(self) -> None:
        # The SWIG factory must expose the concrete C++ runtime types.
        self.assertIsInstance(
            self.deck,
            geometry.ITiglGeometricComponent,
        )
        self.assertIsInstance(
            self.deck,
            configuration.CCPACSDeck,
        )
        self.assertIsInstance(
            self.component,
            configuration.CCPACSDeckComponentBase,
        )

    def test_cabin_geometry(self) -> None:
        cabin_geometry = self.deck.get_cabin_geometry()

        self.assertIsInstance(
            cabin_geometry,
            configuration.CPACSCabinGeometry,
        )

        contours = cabin_geometry.get_contours()

        self.assertIsInstance(
            contours,
            configuration.CPACSCabinGeometryContours,
        )

        contour = contours.get_contour(1)

        self.assertIsInstance(
            contour,
            configuration.CPACSCabinGeometryContour,
        )
        self.assertIsInstance(
            contour.get_y(),
            configuration.CPACSDoubleVectorBase,
        )

    def test_component_containers(self) -> None:
        seat_modules = self.deck.get_seat_modules()

        self.assertIsInstance(
            seat_modules,
            configuration.CPACSSeatModules,
        )
        self.assertIsInstance(
            seat_modules.get_seat_module(1),
            configuration.CCPACSDeckComponentBase,
        )

        sidewall_panels = self.deck.get_sidewall_panels()

        self.assertIsInstance(
            sidewall_panels,
            configuration.CPACSSidewallPanels,
        )
        self.assertIsInstance(
            sidewall_panels.get_sidewall_panel(1),
            configuration.CCPACSDeckComponentBase,
        )

        luggage_compartments = self.deck.get_luggage_compartments()

        self.assertIsInstance(
            luggage_compartments,
            configuration.CPACSLuggageCompartments,
        )
        self.assertIsInstance(
            luggage_compartments.get_luggage_compartment(1),
            configuration.CCPACSDeckComponentBase,
        )

        ceiling_panels = self.deck.get_ceiling_panels()

        self.assertIsInstance(
            ceiling_panels,
            configuration.CPACSCeilingPanels,
        )
        self.assertIsInstance(
            ceiling_panels.get_ceiling_panel(1),
            configuration.CCPACSDeckComponentBase,
        )

        galleys = self.deck.get_galleys()

        self.assertIsInstance(
            galleys,
            configuration.CPACSGalleys,
        )
        self.assertIsInstance(
            galleys.get_galley(1),
            configuration.CCPACSDeckComponentBase,
        )

        floor_modules = self.deck.get_generic_floor_modules()

        self.assertIsInstance(
            floor_modules,
            configuration.CPACSGenericFloorModules,
        )
        self.assertIsInstance(
            floor_modules.get_generic_floor_module(1),
            configuration.CCPACSDeckComponentBase,
        )

        lavatories = self.deck.get_lavatories()

        self.assertIsInstance(
            lavatories,
            configuration.CPACSLavatories,
        )
        self.assertIsInstance(
            lavatories.get_lavatory(1),
            configuration.CCPACSDeckComponentBase,
        )

        class_dividers = self.deck.get_class_dividers()

        self.assertIsInstance(
            class_dividers,
            configuration.CPACSClassDividers,
        )
        self.assertIsInstance(
            class_dividers.get_class_divider(1),
            configuration.CCPACSDeckComponentBase,
        )

        cargo_containers = self.deck.get_cargo_containers()

        self.assertIsInstance(
            cargo_containers,
            configuration.CPACSCargoContainers,
        )
        self.assertIsInstance(
            cargo_containers.get_cargo_container(1),
            configuration.CCPACSDeckComponentBase,
        )

    def test_plural_getters_hidden(self) -> None:
        # Generated vector getters are intentionally hidden from Python.
        with self.assertRaises(AttributeError):
            self.deck.get_seat_modules().get_seat_modules()

        with self.assertRaises(AttributeError):
            self.deck.get_sidewall_panels().get_sidewall_panels()

        with self.assertRaises(AttributeError):
            self.deck.get_luggage_compartments().get_luggage_compartments()

        with self.assertRaises(AttributeError):
            self.deck.get_ceiling_panels().get_ceiling_panels()

        with self.assertRaises(AttributeError):
            self.deck.get_galleys().get_galleys()

        with self.assertRaises(AttributeError):
            (self.deck.get_generic_floor_modules().get_generic_floor_modules())

        with self.assertRaises(AttributeError):
            self.deck.get_lavatories().get_lavatorys()

        with self.assertRaises(AttributeError):
            self.deck.get_class_dividers().get_class_dividers()

        with self.assertRaises(AttributeError):
            self.deck.get_cargo_containers().get_cargo_containers()

    def test_component_accessors(self) -> None:
        self.assertIsInstance(
            self.component.get_mass(),
            float,
        )
        self.assertIsInstance(
            self.component.get_center_of_gravity_global(),
            geometry.CTiglPoint,
        )
        self.assertIsInstance(
            self.component.get_center_of_gravity_local(),
            geometry.CTiglPoint,
        )
        self.assertIsInstance(
            self.component.get_mass_inertia_local(),
            configuration.CTiglMassInertia,
        )

        self.assertIsInstance(
            self.component.get_component_intent(),
            int,
        )
        self.assertIsInstance(
            self.component.get_component_representation(),
            int,
        )
        self.assertIsInstance(
            self.component.get_component_representation_as_string(),
            str,
        )
        self.assertIsInstance(
            self.component.get_component_type(),
            int,
        )

        self.assertIsInstance(
            self.component.get_configuration(),
            configuration.CCPACSConfiguration,
        )
        self.assertIsInstance(
            self.component.get_defaulted_uid(),
            str,
        )


if __name__ == "__main__":
    unittest.main()
