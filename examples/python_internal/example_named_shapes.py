from __future__ import print_function

from tixi3 import tixi3wrapper
from tigl3 import tigl3wrapper
from tigl3.exports import CTiglExportIges
from tigl3.configuration import CCPACSConfigurationManager_get_instance
from tigl3.geometry import CNamedShape
from OCC.BRepPrimAPI import BRepPrimAPI_MakeBox
import os


def export_iges(named_shapes, filename):
    """
    Exports the named shapes array to igs
    """

    # write iges file
    iges_writer = CTiglExportIges()
    for shape in named_shapes:
        iges_writer.add_shape(shape)
    iges_writer.write(filename)


def create_box():
    # build a box with opencascade, this results in a TopoDS_Shape
    box = BRepPrimAPI_MakeBox(0.5, 0.5, 0.5).Shape()

    # make a named shape, required for named exports
    box_ns = CNamedShape(box, "MyBoxObj")

    # the face traits numbering starts at 0!
    box_ns.get_face_traits(4).set_name("BOX_BOTT")
    box_ns.get_face_traits(5).set_name("BOX_TOP")

    return box_ns


def get_wing_shape(tigl_handle, wing_idx):
    """
    Gets the wing from the cpacs config
    """
    print ("Computing wing geometry...")

    # get the configuration manager, this is a singleton!
    # it manages multiple opened cpacs files
    mgr = CCPACSConfigurationManager_get_instance()

    # get the CPACS configuration, defined by the tigl3 handle
    # we need to access the underlying tigl3 handle (that is used in the C/C++ API)
    aircraft_config = mgr.get_configuration(tigl_handle._handle.value)

    wing_ns = aircraft_config.get_wing(wing_idx).get_loft()

    # rename the shape, you can even set long and short names!
    wing_ns.set_name("My_Long_Wing1_Name")
    wing_ns.set_short_name("Wing1Obj")

    # set the name of the second surface. Attention, in iges this name might be cut of due to 8 character rule
    wing_ns.get_face_traits(0).set_name("CoolFac1")
    wing_ns.get_face_traits(1).set_name("CoolFac2")

    return wing_ns


def main():
    tixi_handle = tixi3wrapper.Tixi3()
    tigl_handle = tigl3wrapper.Tigl3()

    print ("Using TiGL Version:", tigl_handle.version)

    dir_path = os.path.dirname(os.path.realpath(__file__))
    tixi_handle.open(dir_path + "/../../tests/TestData/simpletest.cpacs.xml")
    tigl_handle.open(tixi_handle, "")

    # cpacs indexing starts at 1
    wing_shape = get_wing_shape(tigl_handle, 1)
    box_shape = create_box()
    export_iges([wing_shape, box_shape], "simpletest.igs")

if __name__ == '__main__':
    main()