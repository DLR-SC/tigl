from pathlib import Path
import unittest

from OCC.Core.gp import gp_Vec2d
from OCC.Core.TopoDS import TopoDS_Edge

from tixi3.tixi3wrapper import Tixi3
from tigl3.tigl3wrapper import Tigl3
from tigl3.configuration import (
    CCPACSConfigurationManager_get_instance,
    CPACSNacaProfile,
    CTiglNACACalculator,
    NACA4DigitCode,
)


class TestNACA(unittest.TestCase):
    CPACS_FILE = Path("TestData/naca_test.cpacs.xml")
    PROFILE_UID = "NACA0012"
    ABS_TOLERANCE = 1e-6

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()

        if not cls.CPACS_FILE.is_file():
            raise FileNotFoundError(f"File not found: {cls.CPACS_FILE.resolve()}.")

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

        manager = CCPACSConfigurationManager_get_instance()
        cls.aircraft_config = manager.get_configuration(cls.tigl._handle.value)

        profiles = cls.aircraft_config.get_wing_profiles()
        cls.wing_profile = profiles.get_profile(cls.PROFILE_UID)

    @classmethod
    def _close_configuration(cls) -> None:
        try:
            if cls._tigl_is_open:
                cls.tigl.close()
        finally:
            if cls._tixi_is_open:
                cls.tixi.close()

    def test_profile_type(self) -> None:
        naca_profile = self.wing_profile.get_naca_profile_choice4()

        self.assertIsInstance(naca_profile, CPACSNacaProfile)

    def test_edges(self) -> None:
        self.assertIsInstance(
            self.wing_profile.get_upper_wire(),
            TopoDS_Edge,
        )
        self.assertIsInstance(
            self.wing_profile.get_lower_wire(),
            TopoDS_Edge,
        )
        self.assertIsInstance(
            self.wing_profile.get_trailing_edge(),
            TopoDS_Edge,
        )

    def test_trailing_edge_point(self) -> None:
        upper_point = self.wing_profile.get_upper_point(1.0)

        self.assertAlmostEqual(
            upper_point.Z(),
            0.0,
            delta=self.ABS_TOLERANCE,
        )

    def test_upper_curve_type(self) -> None:
        trailing_edge_thickness = 0.000945

        calculator = CTiglNACACalculator(
            NACA4DigitCode("0009"),
            trailing_edge_thickness,
        )

        self.assertIsInstance(calculator, CTiglNACACalculator)

        upper_curve_point = calculator.upper_curve(0.5)

        self.assertIsInstance(upper_curve_point, gp_Vec2d)


if __name__ == "__main__":
    unittest.main()
