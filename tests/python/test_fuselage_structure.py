#############################################################################
# Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
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

from OCC.Core.TopoDS import TopoDS_Shape

from tigl3 import configuration, geometry
from tigl3.tigl3wrapper import Tigl3
from tixi3.tixi3wrapper import Tixi3


class TestFuselageStructureBindings(unittest.TestCase):
    """
    Regression test for https://github.com/DLR-SC/tigl/issues/1037 :
    CCPACSFuselageStructure.GetSkin() and the classes reachable from it
    (CCPACSSkin, CPACSSkinSegments, CCPACSSkinSegment, ...) as well as the
    other boost::optional-typed structure members were missing from the
    Python bindings.
    """

    CPACS_FILE = Path("TestData/fuselage_structure-v3.xml")
    CONFIGURATION_UID = "FuselageStructureTest"

    FUSELAGE_UID = "Fuselage"
    SKIN_SEGMENT_UID = "skinSegment1"
    CARGO_DOOR_UID = "cargoDoor1"

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

        cls.fuselage = cls.uid_manager.get_geometric_component(cls.FUSELAGE_UID)
        cls.structure = cls.fuselage.get_structure()

    @classmethod
    def _close_configuration(cls) -> None:
        try:
            if cls._tigl_is_open:
                cls.tigl.close()
        finally:
            if cls._tixi_is_open:
                cls.tixi.close()

    def test_skin_accessible(self) -> None:
        # CCPACSFuselageStructure.get_skin() used to return an empty/unusable
        # object because CCPACSSkin was not registered with SWIG at all.
        skin = self.structure.get_skin()

        self.assertIsInstance(skin, configuration.CCPACSSkin)

        skin_segments = skin.get_skin_segments()
        self.assertIsInstance(skin_segments, configuration.CPACSSkinSegments)

    def test_skin_segment_geometry(self) -> None:
        skin_segments = self.structure.get_skin().get_skin_segments()

        self.assertEqual(skin_segments.get_skin_segment_count(), 1)

        skin_segment = skin_segments.get_skin_segment(self.SKIN_SEGMENT_UID)
        self.assertIsInstance(
            skin_segment,
            configuration.CCPACSSkinSegment,
        )

        # CCPACSSkinSegment.get_geometry() is inherited from
        # CTiglStringerFrameBorderedObject, which used to be %ignore'd
        # entirely in the SWIG config.
        geom = skin_segment.get_geometry()
        self.assertIsInstance(geom, TopoDS_Shape)
        self.assertFalse(geom.IsNull())

    def test_cargo_door_geometry(self) -> None:
        # CCPACSDoorAssemblyPosition also derives from
        # CTiglStringerFrameBorderedObject and was affected the same way.
        cargo_doors = self.structure.get_cargo_doors()
        self.assertIsInstance(cargo_doors, configuration.CPACSCargoDoorsAssembly)

        door = cargo_doors.get_cargo_door(self.CARGO_DOOR_UID)
        self.assertIsInstance(door, configuration.CCPACSDoorAssemblyPosition)

        geom = door.get_geometry()
        self.assertIsInstance(geom, TopoDS_Shape)
        self.assertFalse(geom.IsNull())

        # cargo doors also implement ITiglGeometricComponent, so they must be
        # reachable (as their concrete type) via the UID manager factory too.
        door_via_uidmanager = self.uid_manager.get_geometric_component(self.CARGO_DOOR_UID)
        self.assertIsInstance(door_via_uidmanager, geometry.ITiglGeometricComponent)
        self.assertIsInstance(door_via_uidmanager, configuration.CCPACSDoorAssemblyPosition)

    def test_other_optional_structure_members(self) -> None:
        # Other boost::optional<...> members of CCPACSFuselageStructure were
        # reported as broken alongside the skin.
        stringers = self.structure.get_stringers()
        self.assertIsInstance(stringers, configuration.CCPACSStringersAssembly)
        self.assertGreater(stringers.get_stringer_count(), 0)

        frames = self.structure.get_frames()
        self.assertIsInstance(frames, configuration.CCPACSFramesAssembly)
        self.assertGreater(frames.get_frame_count(), 0)


if __name__ == "__main__":
    unittest.main()
