from OCC.BRepTools import breptools_Read
from OCC.BRep import BRep_Builder, BRep_Tool_Curve
from OCC.TopoDS import TopoDS_Shape, topods_Edge
from OCC.TopExp import TopExp_Explorer
from OCC.TopAbs import TopAbs_EDGE
from tigl3.geometry import curve_network_to_surface
from pathlib import Path

from OCC.Display.SimpleGui import init_display


def read_brep(filename):
    """
    Reads in a brep file
    """
    if not Path(filename).is_file():
        print ("File not found: " + filename)
        raise FileNotFoundError(filename)

    b = BRep_Builder()
    shape = TopoDS_Shape()
    breptools_Read(shape, filename, b)
    return shape



def get_edges(shape):
    """
    Returns the edges of the shape
    """

    exp = TopExp_Explorer(shape, TopAbs_EDGE)
    while exp.More():
        yield topods_Edge(exp.Current())
        exp.Next()


def main():
    guides_shape = read_brep("data/wing_guides.brep")
    profiles_shape = read_brep("data/wing_profiles.brep")

    # Create array of curves
    guide_curves = []
    for edge in get_edges(guides_shape):
        curve, _, _ = BRep_Tool_Curve(edge)
        guide_curves.append(curve)

    profile_curves = []
    for edge in get_edges(profiles_shape):
        curve, _, _ = BRep_Tool_Curve(edge)
        profile_curves.append(curve)

    # create the gordon surface
    surface = curve_network_to_surface(profile_curves, guide_curves, 1.e-4)

    # display curves and resulting surface
    display, start_display, add_menu, add_function_to_menu = init_display()
    display.Context.SetDeviationCoefficient(0.0001)

    for curve in guide_curves:
        display.DisplayShape(curve)

    for curve in profile_curves:
        display.DisplayShape(curve)

    display.DisplayShape(surface)
    display.FitAll()

    start_display()


if __name__ == "__main__":
    main()
