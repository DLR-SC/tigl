from __future__ import print_function

from tixi3 import tixi3wrapper
from tigl3 import tigl3wrapper
import tigl3.configuration
import os

def display_fuselage_structure(tigl_handle):
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
    uid_manager = config.get_uidmanager()

    f1 = uid_manager.get_geometric_component("Fuselage")
    d1 = uid_manager.get_geometric_component("cargoDoor1")
    d2 = uid_manager.get_geometric_component("cargoDoor2")
    ccb = uid_manager.get_geometric_component("cargoCrossBeam1")
    cbs = uid_manager.get_geometric_component("cargoCrossBeamStrut1")
    lfb = uid_manager.get_geometric_component("longFloorBeam1")


    display, start_display, add_menu, add_function_to_menu = init_display()

    display.DisplayShape(f1.get_loft().shape(), transparency=0.5, update=True)
    display.DisplayShape(d1.get_loft().shape(), transparency=0.1, update=True, color="blue1")
    display.DisplayShape(d2.get_loft().shape(), transparency=0.1, update=True, color="blue1")
    display.DisplayShape(ccb.get_loft().shape(),transparency=0.0, update=True)
    display.DisplayShape(cbs.get_loft().shape(), transparency=0.0, update=True)
    display.DisplayShape(lfb.get_loft().shape(), transparency=0.0, update=True)


    display.FitAll()

    start_display()


if __name__ == '__main__':
    tixi_h = tixi3wrapper.Tixi3()
    tigl_h = tigl3wrapper.Tigl3()

    dir_path = os.path.dirname(os.path.realpath(__file__))
    tixi_h.open(dir_path + "/../../tests/unittests/TestData/fuselage_structure-v3.xml")
    tigl_h.open(tixi_h, "")

    display_fuselage_structure(tigl_h)
