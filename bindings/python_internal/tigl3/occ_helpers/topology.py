from OCC.Core.TopExp import TopExp_Explorer
from OCC.Core.BRepTools import BRepTools_WireExplorer, breptools_Read, breptools_Write
from OCC.Core.TopoDS import topods_Edge, topods_Wire, TopoDS_Compound, TopoDS_Shape
from OCC.Core.BRep import BRep_Builder
from OCC.Core.TopAbs import TopAbs_EDGE, TopAbs_WIRE
from pathlib import Path


def iter_edges(shape):
    """
    Generator / Iterator over the edges of a shape
    """

    exp = TopExp_Explorer(shape, TopAbs_EDGE)
    while exp.More():
        yield topods_Edge(exp.Current())
        exp.Next()


def iter_wires(shape):
    """
    Generator / Iterator over the wire of a shape
    """

    exp = TopExp_Explorer(shape, TopAbs_WIRE)
    while exp.More():
        yield topods_Wire(exp.Current())
        exp.Next()


def iter_edges_from_wire(wire):
    """
    Generator / Iterator over the edges of a wire

    Compared to iter_edges, it preserves the order
    of the edges.
    """
    exp = BRepTools_WireExplorer(wire)
    while exp.More():
        yield topods_Edge(exp.Current())
        exp.Next()


def make_compound(shape_array):
    """
    Creates a TopoDS_Compund from a list of TopoDS_Shapes

    :param shape_array: list of shapes
    :return: TopoDS_Compound
    """

    b = BRep_Builder()
    c = TopoDS_Compound()
    b.MakeCompound(c)
    for shape in shape_array:
        b.Add( c, shape)
    return c


def read_brep(filename):
    """
    Reads in a brep file

    :return: The shape
    """
    if not Path(filename).is_file():
        print ("File not found: " + filename)
        raise FileNotFoundError(filename)

    b = BRep_Builder()
    shape = TopoDS_Shape()
    if not breptools_Read(shape, filename, b):
        raise RuntimeError("Cannot read brep file: " + filename)
    return shape


def write_brep(shape, filename):
    """
    Writes a TopoDS_Shape to a brep file
    :param shape: The shape given as a PythonOCC TopoDS_Shape
    :param filename: Filename to be written
    """
    if not breptools_Write(shape, filename):
        raise RuntimeError("Cannot write brep file: " + filename)
