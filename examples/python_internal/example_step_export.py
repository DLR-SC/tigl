from __future__ import print_function

from tixi import tixiwrapper
from tigl import tiglwrapper
import tigl.configuration, tigl.geometry, tigl.boolean_ops, tigl.exports
import os

def export_fused_step(tigl_handle):
    """
    This example uses the opencascade fusing algorithm to fuse wing
    and fuselage and writes the resulting geometry to a step file
    """
    print ("Computing fused geometry...")

    # get the configuration manager
    mgr = tigl.configuration.CCPACSConfigurationManager_get_instance()

    # get the CPACS configuration, defined by the tigl handle
    # we need to access the underlying tigl handle (that is used in the C/C++ API)
    config = mgr.get_configuration(tigl_handle._handle.value)

    fuselageShape = config.get_fuselage(1).get_loft()
    wingShape = config.get_wing(1).get_loft()
    wingMShape = config.get_wing(1).get_mirrored_loft()

    # set another name
    wingMShape.set_name("cool_2nd_wing")

    # fuse the shapes
    shape_list = tigl.boolean_ops.ListPNamedShape()
    shape_list.push_back(wingShape)
    shape_list.push_back(wingMShape)
    shape = tigl.boolean_ops.CFuseShapes(fuselageShape, shape_list).named_shape()

    print ("Done!")

    # write step file
    step_writer = tigl.exports.CTiglExportStep()
    step_writer.add_shape(shape)
    step_writer.write("simpletest.stp")


if __name__ == '__main__':
    tixi_h = tixiwrapper.Tixi()
    tigl_h = tiglwrapper.Tigl()

    dir_path = os.path.dirname(os.path.realpath(__file__))
    tixi_h.open(dir_path + "/../../tests/TestData/simpletest.cpacs.xml")
    tigl_h.open(tixi_h, "")

    export_fused_step(tigl_h)
