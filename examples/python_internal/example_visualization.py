from __future__ import print_function

from tixi import tixiwrapper
from tigl import tiglwrapper
import tigl.configuration, tigl.geometry, tigl.boolean_ops, tigl.exports
import os

def display_configuration(tigl_handle):
    """
    This is an example how to use the internal tigl/pyocc API
    to display all wing and fuselage segments
    """

    from OCC.Display.SimpleGui import init_display

    # get the configuration manager
    mgr = tigl.configuration.CCPACSConfigurationManager_get_instance()

    # get the CPACS configuration, defined by the tigl handle
    # we need to access the underlying tigl handle (that is used in the C/C++ API)
    config = mgr.get_configuration(tigl_handle._handle.value)

    display, start_display, add_menu, add_function_to_menu = init_display()

    for ifuse in range(1, config.get_fuselage_count() + 1):
        fuselage = config.get_fuselage(ifuse)
        for isegment in range(1, fuselage.get_segment_count() + 1):
            segment = fuselage.get_segment(isegment)
            display.DisplayShape(segment.get_loft().shape(), update=True)

            mirrored_shape = segment.get_mirrored_loft()
            if mirrored_shape is not None:
                display.DisplayShape(mirrored_shape.shape(), update=True)

    for iwing in range(1, config.get_wing_count() + 1):
        wing = config.get_wing(iwing)

        for isegment in range(1, wing.get_segment_count() + 1):
            segment = wing.get_segment(isegment)

            display.DisplayShape(segment.get_loft().shape(), update=True)

            mirrored_shape = segment.get_mirrored_loft()
            if mirrored_shape is not None:
                display.DisplayShape(mirrored_shape.shape(), update=True)

    for iobj in range(1, config.get_external_object_count()+1):
        obj = config.get_external_object(iobj)
        shape = obj.get_loft()

        if shape is not None:
            display.DisplayShape(shape.shape(), update=True)

        mirrored_shape = obj.get_mirrored_loft()

        if mirrored_shape is not None:
            display.DisplayShape(mirrored_shape.shape(), update=True)

    display.FitAll()

    start_display()


if __name__ == '__main__':
    tixi_h = tixiwrapper.Tixi()
    tigl_h = tiglwrapper.Tigl()

    dir_path = os.path.dirname(os.path.realpath(__file__))
    tixi_h.open(dir_path + "/../../tests/TestData/D150_v201.xml")
    tigl_h.open(tixi_h, "")

    display_configuration(tigl_h)
