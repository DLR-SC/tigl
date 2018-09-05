from __future__ import print_function

from tixi3 import tixi3wrapper

from tigl3 import tigl3wrapper
from tigl3.configuration import CCPACSConfigurationManager_get_instance
from tigl3.boolean_ops import CFuseShapes
from tigl3.import_export_helper import export_shapes
import os


def get_fused_shape(tigl_handle):
    """
    This example uses the opencascade fusing algorithm to fuse wing
    and fuselage and writes the resulting geometry to a step file
    """
    print ("Computing fused geometry...")

    # get the configuration manager
    mgr = CCPACSConfigurationManager_get_instance()

    # get the CPACS configuration, defined by the tigl handle
    # we need to access the underlying tigl handle (that is used in the C/C++ API)
    config = mgr.get_configuration(tigl_handle._handle.value)

    fuselage_shape = config.get_fuselage(1).get_loft()
    wing_shape = config.get_wing(1).get_loft()
    wing_m_shape = config.get_wing(1).get_mirrored_loft()

    # fuse the shapes
    fused_shape = CFuseShapes(fuselage_shape, [wing_shape, wing_m_shape]).named_shape()

    print ("Done!")

    return fused_shape


if __name__ == '__main__':
    tixi_h = tixi3wrapper.Tixi3()
    tigl_h = tigl3wrapper.Tigl3()

    dir_path = os.path.dirname(os.path.realpath(__file__))
    tixi_h.open(dir_path + "/../../tests/unittests/TestData/simpletest.cpacs.xml")
    tigl_h.open(tixi_h, "")

    fused_ac = get_fused_shape(tigl_h)
    export_shapes([fused_ac], "simpletest.stp")
