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
from tigl3 import configuration, geometry

from OCC.Core.Bnd import Bnd_Box
from OCC.Core.BRepBndLib import brepbndlib_Add


class Decks(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.tixi = Tixi3()
        cls.tigl = Tigl3()

        cls.assertIsNone = staticmethod(unittest.TestCase().assertIsNone)
        cls.assertIsNone(cls.tixi.open("TestData/simpletest-decks.cpacs.xml"))
        cls.assertIsNone(cls.tigl.open(cls.tixi, "testAircraft"))

        mgr = configuration.CCPACSConfigurationManager.get_instance()
        cls.config = mgr.get_configuration(cls.tigl._handle.value)
        cls.uid_mgr = cls.config.get_uidmanager()

    @classmethod
    def tearDownClass(cls):
        cls.tigl.close()
        cls.tixi.close()

    def test_accessibility(self):

        deck = self.uid_mgr.get_geometric_component("deck1")
        self.assertIsInstance(deck, geometry.ITiglGeometricComponent)
        self.assertIsInstance(deck, configuration.CCPACSDeck)

        # Cabin geometry
        cabinGeometry = deck.get_cabin_geometry()
        self.assertIsInstance(cabinGeometry, configuration.CPACSCabinGeometry)
        self.assertIsInstance(
            cabinGeometry.get_contours(), configuration.CPACSCabinGeometryContours
        )
        contour = cabinGeometry.get_contours().get_contour(1)
        self.assertIsInstance(
            contour,
            configuration.CPACSCabinGeometryContour,
        )
        self.assertIsInstance(
            contour.get_y(),
            configuration.CPACSDoubleVectorBase,
        )

        # Deck components
        self.assertIsInstance(deck.get_seat_modules(), configuration.CPACSSeatModules)
        self.assertIsInstance(
            deck.get_seat_modules().get_seat_module(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_sidewall_panels(), configuration.CPACSSidewallPanels
        )
        self.assertIsInstance(
            deck.get_sidewall_panels().get_sidewall_panel(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_luggage_compartments(), configuration.CPACSLuggageCompartments
        )
        self.assertIsInstance(
            deck.get_luggage_compartments().get_luggage_compartment(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_ceiling_panels(), configuration.CPACSCeilingPanels
        )
        self.assertIsInstance(
            deck.get_ceiling_panels().get_ceiling_panel(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_galleys(), configuration.CPACSGalleys
        )
        self.assertIsInstance(
            deck.get_galleys().get_galley(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_generic_floor_modules(), configuration.CPACSGenericFloorModules
        )
        self.assertIsInstance(
            deck.get_generic_floor_modules().get_generic_floor_module(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_lavatories(), configuration.CPACSLavatories
        )
        self.assertIsInstance(
            deck.get_lavatories().get_lavatory(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_class_dividers(), configuration.CPACSClassDividers
        )
        self.assertIsInstance(
            deck.get_class_dividers().get_class_divider(1),
            configuration.CCPACSDeckComponentBase,
        )

        self.assertIsInstance(
            deck.get_cargo_containers(), configuration.CPACSCargoContainers
        )
        self.assertIsInstance(
            deck.get_cargo_containers().get_cargo_container(1),
            configuration.CCPACSDeckComponentBase,
        )


if __name__ == "__main__":
    unittest.main()
