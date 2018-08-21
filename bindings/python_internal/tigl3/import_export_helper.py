import os
from tigl3.exports import TriangulatedExportOptions, create_exporter as tigl_create_exporter
from tigl3.geometry import CNamedShape

from OCC.TopoDS import TopoDS_Shape


def export_shapes(shapes, filename, deflection=0.001):
    """
    Exports the shapes to a file
    :param shapes: Array of shapes
    :param filename: Name of the file to write
    :param deflection: Triangulation accuracy. The smaller the value,
                       the more triangles are written (only for triangulated file formats)
    :return: True, if export was successful
    """
    file_type = os.path.splitext(filename)[1][1:]
    print("Exporting %s file..." % file_type)

    unnamed_shape_counter = 0

    # write step file
    writer = tigl_create_exporter(file_type)
    for shape in shapes:
        if isinstance(shape, CNamedShape):
            writer.add_shape(shape, TriangulatedExportOptions(deflection))
        elif isinstance(shape, TopoDS_Shape):
            unnamed_shape_counter += 1
            named_shape = CNamedShape(shape, "Unnamed%d" % unnamed_shape_counter)
            writer.add_shape(named_shape, TriangulatedExportOptions(deflection))
        else:
            raise RuntimeError("Cannot export %s to file!" % type(shape))
    return writer.write(filename)
