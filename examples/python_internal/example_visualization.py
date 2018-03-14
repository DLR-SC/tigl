from __future__ import print_function

from tixi3 import tixi3wrapper
from tigl3 import tigl3wrapper
import tigl3.configuration, tigl3.geometry, tigl3.boolean_ops, tigl3.exports
from OCC.Quantity import Quantity_NOC_RED
import os

def display_configuration(tigl_handle):
    """
    This is an example how to use the internal tigl/pyocc API
    to display all wing and fuselage segments
    """

    from OCC.Display.SimpleGui import init_display

    # get the configuration manager
    mgr = tigl3.configuration.CCPACSConfigurationManager_get_instance()

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

        display.DisplayShape(wing.get_loft().shape(), update=True, transparency=0.3)

        mirrored_shape = wing   .get_mirrored_loft()
        if mirrored_shape is not None:
            display.DisplayShape(mirrored_shape.shape(), update=True)

        for i_comp_seg in range(1, wing.get_component_segment_count()+1):

            cs = wing.get_component_segment(i_comp_seg)
            structure = cs.get_structure()

            if structure is not None:
                # draw spars
                for ispar in range(1, structure.get_spar_segment_count() + 1):
                    spar = structure.get_spar_segment(ispar)
                    sparGeom = spar.get_spar_geometry()
                    display.DisplayShape(sparGeom, color=Quantity_NOC_RED, update=True)

                # draw ribs
                for irib in range(1,structure.get_ribs_definition_count()+1):
                    rib = structure.get_ribs_definition(irib)
                    try:
                        ribGeom = rib.get_ribs_geometry()
                        display.DisplayShape(ribGeom, color=Quantity_NOC_RED, update=True)
                    except RuntimeError as e:
                        print (e)



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
    tixi_h = tixi3wrapper.Tixi3()
    tigl_h = tigl3wrapper.Tigl3()

    dir_path = os.path.dirname(os.path.realpath(__file__))
    tixi_h.open(dir_path + "/../../tests/unittests/TestData/D150_v30.xml")
    tigl_h.open(tixi_h, "")

    display_configuration(tigl_h)
