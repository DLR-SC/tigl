from __future__ import print_function

from tixi3 import tixi3wrapper
from tigl3 import tigl3wrapper
import tigl3.configuration
from OCC.Quantity import Quantity_NOC_RED, Quantity_NOC_GREEN, Quantity_NOC_BLUE1
import os


def display_wing_cell_geom(configuration):
    """
    This is an example how to use the internal tigl/pyocc API
    to display all wing and fuselage segments
    """

    from OCC.Display.SimpleGui import init_display
    display, start_display, add_menu, add_function_to_menu = init_display()

    wing = configuration.get_wing(1)
    display.DisplayShape(wing.get_loft().shape(), transparency=0.7)

    # display cell geometry
    cs = wing.get_component_segment(1)
    structure = cs.get_structure()
    cell = structure.get_upper_shell().get_cell(1)
    cell_shape = cell.get_loft()
    display.DisplayShape(cell_shape.shape(), color=Quantity_NOC_RED, transparency=0.3)

    # display spars and ribs
    spar = structure.get_spar_segment(1)
    spar_shape = spar.get_spar_geometry()
    display.DisplayShape(spar_shape, color=Quantity_NOC_GREEN, update=True)

    # draw ribs
    for i_rib in range(1, structure.get_ribs_definition_count() + 1):
        rib = structure.get_ribs_definition(i_rib)
        rib_shape = rib.get_ribs_geometry()
        display.DisplayShape(rib_shape, color=Quantity_NOC_BLUE1, update=True)

    display.FitAll()

    start_display()


if __name__ == '__main__':
    tixi_h = tixi3wrapper.Tixi3()
    tigl_h = tigl3wrapper.Tigl3()

    dir_path = os.path.dirname(os.path.realpath(__file__))
    tixi_h.open(dir_path + "/../../tests/unittests/TestData/cell_rib_spar_test.xml")
    tigl_h.open(tixi_h, "")

    # get the configuration manager
    mgr = tigl3.configuration.CCPACSConfigurationManager_get_instance()

    # get the CPACS configuration, defined by the tigl handle
    # we need to access the underlying tigl handle (that is used in the C/C++ API)
    config = mgr.get_configuration(tigl_h._handle.value)
    display_wing_cell_geom(config)