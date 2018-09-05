from tigl3.geometry import CTiglCurvesToSurface


def interpolate_curves(curve_list, params=None, degree=3, close_continuous=False):
    """
    Creates a surface loft that passes through the curve array through B-spline surface interpolation.


    :param curve_list: List of Geom_Curves, that are created using e.g. tigl3.curve_factories
    :param params: Optional surface parameters (list of floats), at which the surfaces passes trough the curves.
                   This has a strong effect on the final surface shape.
    :param degree: Maximum degree that is used for interpolation. When degree=1, a linear loft is created.
    :param close_continuous: If the first and last curve are equal close_continuous and is true,
                             the loft is c2 continuous at the junction between first and last curve.
    :return: The final surface (Handle to Geom_BSplineSurface)
    """

    if params is None:
        surface_builder = CTiglCurvesToSurface(curve_list, close_continuous)
    else:
        if len(params) != len(curve_list):
            raise RuntimeError("Number of parameters don't match number of curves")
        surface_builder = CTiglCurvesToSurface(curve_list, params, close_continuous)

    surface_builder.set_max_degree(degree)
    surface = surface_builder.surface()
    return surface
