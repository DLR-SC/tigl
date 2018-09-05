from tigl3.geometry import CTiglCurvesToSurface, CTiglInterpolateCurveNetwork


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


def interpolate_curve_network(profiles, guides, tolerance=1e-4):
    """
    Interpolates a network of curves with a B-spline surface.
    Internally, this is done with a Gordon surface.

    Gordon surfaces normally require the input curves to have
    compatible intersections (i.e. intersections at the same parameter).
    Here, we overcome this limitation by re-parametrization of the
    input curves.

    :param profiles: List of profiles (List of Geom_Curves)
    :param guides: List of guides (List of Geom_Curves)
    :param tolerance: Maximum allowed distance between each guide and profile
                     (in theory they must intersect and the distance is zero)
    :return: The final surface (Handle to Geom_BSplineSurface)
    """
    interpolator = CTiglInterpolateCurveNetwork(profiles, guides, tolerance)
    return interpolator.surface()
