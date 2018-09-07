from OCC.BRep import BRep_Tool_Curve
from OCC.Display.SimpleGui import init_display
from tigl3.geometry import curve_network_to_surface

from tigl3.occ_helpers.topology import read_brep, iter_edges


def main():
    # load curves from files
    guides_shape = read_brep("data/wing_guides.brep")
    profiles_shape = read_brep("data/wing_profiles.brep")

    # Create array of curves
    guide_curves = []
    for edge in iter_edges(guides_shape):
        curve, _, _ = BRep_Tool_Curve(edge)
        guide_curves.append(curve)

    profile_curves = []
    for edge in iter_edges(profiles_shape):
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
