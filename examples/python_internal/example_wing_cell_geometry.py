from __future__ import print_function

from tixi3 import tixi3wrapper
from tigl3 import tigl3wrapper
import tigl3.configuration
import os


def display_wing_cell_geom(configuration):
    """
    This is an example how to use the internal tigl/pyocc API
    to display all wing and fuselage segments
    """

    from OCC.Display.SimpleGui import init_display
    display, start_display, add_menu, add_function_to_menu = init_display()
    display.Context.SetDeviationCoefficient(0.0001)

    uid_mgr = configuration.get_uidmanager()
    wing = uid_mgr.get_geometric_component("Wing")
    display.DisplayShape(wing.get_loft().shape(), transparency=0.7)

    # display cell geometry
    cell = uid_mgr.get_geometric_component("Wing_CS_upperShell_Cell1")
    display.DisplayShape(cell.get_loft().shape(), transparency=0.3, color="green")

    # display ribs and spar
    ribs = uid_mgr.get_geometric_component("Wing_CS_RibDef1")
    display.DisplayShape(ribs.get_loft().shape(),color="blue")

    spar = uid_mgr.get_geometric_component("Wing_CS_spar1")
    display.DisplayShape(spar.get_loft().shape(),color="blue")

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