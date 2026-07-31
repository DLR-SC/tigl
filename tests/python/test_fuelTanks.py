#############################################################################
# Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
#
# Created: 2023-12-29 Marko Alder <marko.alder@dlr.de>
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

from OCC.Core.TopoDS import TopoDS_Face

from tigl3 import configuration
from tigl3.tigl3wrapper import Tigl3
from tixi3.tixi3wrapper import Tixi3


class TestFuelTanks(unittest.TestCase):
    CPACS_FILE = Path("TestData/simpletest-fuelTanks.cpacs.xml")

    TANK_UID = "tank1"
    SEGMENT_VESSEL_UID = "tank1_outerVessel"
    GUIDE_VESSEL_UID = "tank2_outerVessel"
    SPHERICAL_VESSEL_UID = "tank3_sphericalDome"
    ELLIPSOID_VESSEL_UID = "tank3_ellipsoidDome1"
    TORISPHERICAL_VESSEL_UID = "tank4_torisphericalDome"
    ISOTENSOID_VESSEL_UID = "tank5_isotensoidDome"

    GEOMETRY_TOLERANCE = 1e-2
    POINT_TOLERANCE = 1e-5

    INVALID_VESSEL_TYPE_MESSAGE = (
        "This method is only available for vessels with segments. " "No segment found."
    )

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

        tigl_result = cls.tigl.open(cls.tixi, "")
        if tigl_result is not None:
            raise AssertionError(f"Tigl3.open() unexpectedly returned {tigl_result!r}")
        cls._tigl_is_open = True

        manager = configuration.CCPACSConfigurationManager.get_instance()
        cls.aircraft_config = manager.get_configuration(cls.tigl._handle.value)
        cls.uid_manager = cls.aircraft_config.get_uidmanager()

        # Cache representative segment-based and parametric vessels.
        cls.fuel_tank = cls.uid_manager.get_geometric_component(cls.TANK_UID)
        cls.segment_vessel = cls.uid_manager.get_geometric_component(
            cls.SEGMENT_VESSEL_UID
        )
        cls.guide_vessel = cls.uid_manager.get_geometric_component(cls.GUIDE_VESSEL_UID)
        cls.spherical_vessel = cls.uid_manager.get_geometric_component(
            cls.SPHERICAL_VESSEL_UID
        )
        cls.ellipsoid_vessel = cls.uid_manager.get_geometric_component(
            cls.ELLIPSOID_VESSEL_UID
        )
        cls.torispherical_vessel = cls.uid_manager.get_geometric_component(
            cls.TORISPHERICAL_VESSEL_UID
        )
        cls.isotensoid_vessel = cls.uid_manager.get_geometric_component(
            cls.ISOTENSOID_VESSEL_UID
        )

    @classmethod
    def _close_configuration(cls) -> None:
        try:
            if cls._tigl_is_open:
                cls.tigl.close()
        finally:
            if cls._tixi_is_open:
                cls.tixi.close()

    def test_configuration(self) -> None:
        self.assertEqual(
            self.aircraft_config.get_fuel_tank_count(),
            8,
        )

        # Verify the index- and UID-based overloads exposed by SWIG.
        self.assertIsInstance(
            self.aircraft_config.get_fuel_tank(1),
            configuration.CPACSFuelTank,
        )
        self.assertIsInstance(
            self.aircraft_config.get_fuel_tank(self.TANK_UID),
            configuration.CCPACSFuelTank,
        )
        self.assertEqual(
            self.aircraft_config.get_fuel_tank_index(self.TANK_UID),
            1,
        )
        self.assertIsInstance(
            self.aircraft_config.get_fuel_tanks(),
            configuration.CPACSFuelTanks,
        )

    def test_fuel_tanks(self) -> None:
        fuel_tanks = self.fuel_tank.get_parent()

        self.assertIsInstance(
            fuel_tanks,
            configuration.CPACSFuelTanks,
        )
        self.assertIsInstance(
            fuel_tanks.get_fuel_tank(1),
            configuration.CCPACSFuelTank,
        )
        self.assertEqual(
            fuel_tanks.get_fuel_tank(1).get_defaulted_uid(),
            self.TANK_UID,
        )
        self.assertIsInstance(
            fuel_tanks.get_fuel_tank(self.TANK_UID),
            configuration.CPACSFuelTank,
        )
        self.assertEqual(
            fuel_tanks.get_fuel_tank_index(self.TANK_UID),
            1,
        )
        self.assertEqual(
            fuel_tanks.get_fuel_tank_count(),
            8,
        )

    def test_fuel_tank(self) -> None:
        self.assertIsInstance(
            self.fuel_tank.get_vessels(),
            configuration.CPACSVessels,
        )
        self.assertEqual(
            self.fuel_tank.get_name(),
            "Simple tank 1",
        )

    def test_vessels(self) -> None:
        vessels = self.segment_vessel.get_parent()

        self.assertIsInstance(
            vessels.get_vessel(1),
            configuration.CPACSVessel,
        )
        self.assertEqual(
            vessels.get_vessel(1).get_defaulted_uid(),
            self.SEGMENT_VESSEL_UID,
        )
        self.assertIsInstance(
            vessels.get_vessel(self.SEGMENT_VESSEL_UID),
            configuration.CPACSVessel,
        )
        self.assertEqual(
            vessels.get_vessel_index(self.SEGMENT_VESSEL_UID),
            1,
        )
        self.assertEqual(
            vessels.get_vessel_count(),
            2,
        )

        self.assertIsInstance(
            vessels.get_parent(),
            configuration.CPACSFuelTank,
        )

    def test_vessel_accessors(self) -> None:
        vessel_configuration = self.segment_vessel.get_configuration()

        self.assertEqual(
            vessel_configuration.get_uid(),
            "testAircraft",
        )
        self.assertIsInstance(
            vessel_configuration,
            configuration.CCPACSConfiguration,
        )
        self.assertEqual(
            self.segment_vessel.get_defaulted_uid(),
            self.SEGMENT_VESSEL_UID,
        )
        self.assertEqual(
            self.segment_vessel.get_name(),
            "Outer vessel",
        )

    def test_vessel_types(self) -> None:
        # Vessel geometry uses either segments or parametric design data.
        self.assertTrue(self.segment_vessel.is_vessel_via_segments())
        self.assertFalse(self.segment_vessel.is_vessel_via_design_parameters())
        self.assertFalse(self.spherical_vessel.is_vessel_via_segments())
        self.assertTrue(self.spherical_vessel.is_vessel_via_design_parameters())

        # A spherical dome is an ellipsoid with a half-axis fraction of 1.
        self.assertFalse(self.segment_vessel.has_spherical_dome())
        self.assertFalse(self.segment_vessel.has_ellipsoid_dome())
        self.assertFalse(self.segment_vessel.has_torispherical_dome())
        self.assertFalse(self.segment_vessel.has_isotensoid_dome())

        self.assertTrue(self.spherical_vessel.has_spherical_dome())
        self.assertTrue(self.spherical_vessel.has_ellipsoid_dome())
        self.assertFalse(self.spherical_vessel.has_torispherical_dome())
        self.assertFalse(self.spherical_vessel.has_isotensoid_dome())

        self.assertFalse(self.ellipsoid_vessel.has_spherical_dome())
        self.assertTrue(self.ellipsoid_vessel.has_ellipsoid_dome())
        self.assertFalse(self.ellipsoid_vessel.has_torispherical_dome())
        self.assertFalse(self.ellipsoid_vessel.has_isotensoid_dome())

        self.assertFalse(self.torispherical_vessel.has_spherical_dome())
        self.assertFalse(self.torispherical_vessel.has_ellipsoid_dome())
        self.assertTrue(self.torispherical_vessel.has_torispherical_dome())
        self.assertFalse(self.torispherical_vessel.has_isotensoid_dome())

        self.assertFalse(self.isotensoid_vessel.has_spherical_dome())
        self.assertFalse(self.isotensoid_vessel.has_ellipsoid_dome())
        self.assertFalse(self.isotensoid_vessel.has_torispherical_dome())
        self.assertTrue(self.isotensoid_vessel.has_isotensoid_dome())

    def test_vessel_sections(self) -> None:
        # Parametric vessels have no sections and reject section-only access.
        self.assertEqual(
            self.segment_vessel.get_section_count(),
            3,
        )
        self.assertEqual(
            self.spherical_vessel.get_section_count(),
            0,
        )
        self.assertIsInstance(
            self.segment_vessel.get_section(1),
            configuration.CCPACSFuselageSection,
        )

        with self.assertRaises(RuntimeError) as context:
            self.spherical_vessel.get_section(1)

        self.assertEqual(
            str(context.exception),
            self.INVALID_VESSEL_TYPE_MESSAGE,
        )

        # Section faces are reconstructed from segment boundary wires.
        self.assertIsInstance(
            self.segment_vessel.get_section_face("outerVessel_section3"),
            TopoDS_Face,
        )

        with self.assertRaises(RuntimeError) as context:
            self.spherical_vessel.get_section_face("outerVessel_section3")

        self.assertEqual(
            str(context.exception),
            self.INVALID_VESSEL_TYPE_MESSAGE,
        )

    def test_vessel_segments(self) -> None:
        # Parametric vessels have no segments and reject segment access.
        self.assertEqual(
            self.segment_vessel.get_segment_count(),
            2,
        )
        self.assertEqual(
            self.spherical_vessel.get_segment_count(),
            0,
        )
        self.assertIsInstance(
            self.segment_vessel.get_segment(1),
            configuration.CCPACSFuselageSegment,
        )

        with self.assertRaises(RuntimeError) as context:
            self.spherical_vessel.get_segment(1)

        self.assertEqual(
            str(context.exception),
            self.INVALID_VESSEL_TYPE_MESSAGE,
        )

    def test_guide_curves(self) -> None:
        point = self.guide_vessel.get_guide_curve_points()[1]

        self.assertAlmostEqual(
            point.X(),
            3.5,
            delta=self.GEOMETRY_TOLERANCE,
        )
        self.assertAlmostEqual(
            point.Y(),
            0.0,
            delta=self.POINT_TOLERANCE,
        )
        self.assertAlmostEqual(
            point.Z(),
            -0.65,
            delta=self.GEOMETRY_TOLERANCE,
        )

        with self.assertRaises(RuntimeError) as context:
            self.spherical_vessel.get_guide_curve_points()

        self.assertEqual(
            str(context.exception),
            self.INVALID_VESSEL_TYPE_MESSAGE,
        )

        guide_curve_segment = self.guide_vessel.get_guide_curve_segment(
            "tank2_seg1_upper"
        )

        self.assertEqual(
            guide_curve_segment.get_guide_curve_profile_uid(),
            "gc_upper",
        )

        with self.assertRaises(RuntimeError) as context:
            self.spherical_vessel.get_guide_curve_segment("tank2_seg1_upper")

        self.assertEqual(
            str(context.exception),
            self.INVALID_VESSEL_TYPE_MESSAGE,
        )

    def test_loft_evaluation(self) -> None:
        self.assertAlmostEqual(
            self.segment_vessel.get_geometric_volume(),
            6.57,
            delta=self.GEOMETRY_TOLERANCE,
        )
        self.assertAlmostEqual(
            self.spherical_vessel.get_geometric_volume(),
            18.1,
            delta=self.GEOMETRY_TOLERANCE,
        )
        self.assertAlmostEqual(
            self.segment_vessel.get_surface_area(),
            19.94,
            delta=self.GEOMETRY_TOLERANCE,
        )
        self.assertAlmostEqual(
            self.spherical_vessel.get_surface_area(),
            36.19,
            delta=self.GEOMETRY_TOLERANCE,
        )
        self.assertAlmostEqual(
            self.segment_vessel.get_circumference(1, 0.5),
            7.43,
            delta=self.GEOMETRY_TOLERANCE,
        )

        with self.assertRaises(RuntimeError) as context:
            self.spherical_vessel.get_circumference(1, 0.5)

        self.assertEqual(
            str(context.exception),
            self.INVALID_VESSEL_TYPE_MESSAGE,
        )

    def test_structure(self) -> None:
        # Vessel structures expose optional frame, stringer, and wall assemblies.
        wall_structure = self.isotensoid_vessel.get_structure()
        reinforced_structure = self.segment_vessel.get_structure()

        self.assertIsInstance(
            reinforced_structure.get_frames(),
            configuration.CCPACSFramesAssembly,
        )
        self.assertIsInstance(
            reinforced_structure.get_stringers(),
            configuration.CCPACSStringersAssembly,
        )
        self.assertIsInstance(
            wall_structure.get_walls(),
            configuration.CCPACSWalls,
        )


if __name__ == "__main__":
    unittest.main()
