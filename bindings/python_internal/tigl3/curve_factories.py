import tigl3.occ_helpers.containers
from tigl3.geometry import CTiglPointsToBSplineInterpolation


def points_to_curve(points, params=None, degree=3, close_continuous=False):
    """
    Creates a b-spline that passes through the given points
    using b-spline interpolation.

    :param points: Array of points (numpy array also works!). First dimension over number of points, second must be 3!
    :param params: Optional list of parameters, at which the points should be interpolated.
    :param degree: Polynomial degree of the resulting b-spline curve.
    :param close_continuous: If True, the start and end of the curve will be continuous
        (only, if first and last point is equal!)
    :return: Geom_BSplineCurve
    """

    parray = tigl3.occ_helpers.containers.point_array(points)
    if params is None:
        interp = CTiglPointsToBSplineInterpolation(parray.GetHandle(), degree, close_continuous)
    else:
        if len(params) != len(points):
            raise RuntimeError("Number of parameters don't match number of points")
        interp = CTiglPointsToBSplineInterpolation(parray.GetHandle(), params, degree, close_continuous)

    curve = interp.curve()
    return curve
