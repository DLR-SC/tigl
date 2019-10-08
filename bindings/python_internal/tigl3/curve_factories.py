from tigl3.geometry import CTiglPointsToBSplineInterpolation
from tigl3.occ_helpers.containers import float_array, int_array, point_array
from OCC.Geom import Geom_BSplineCurve

def interpolate_points(points, params=None, degree=3, close_continuous=False):
    """
    Creates a b-spline that passes through the given points
    using b-spline interpolation.

    :param points: Array of points (numpy array also works!). First dimension over number of points, second must be 3!
    :param params: Optional list of parameters (list of floats), at which the points should be interpolated.
                   This has a strong effect on the final curve shape.
    :param degree: Polynomial degree of the resulting b-spline curve.
    :param close_continuous: If True, the start and end of the curve will be continuous
        (only, if first and last point is equal!)
    :return: The resulting curve (Handle to Geom_BSplineCurve)
    """

    occ_points_array = point_array(points)
    if params is None:
        interp = CTiglPointsToBSplineInterpolation(occ_points_array.GetHandle(), degree, close_continuous)
    else:
        if len(params) != len(points):
            raise RuntimeError("Number of parameters don't match number of points")
        interp = CTiglPointsToBSplineInterpolation(occ_points_array.GetHandle(), params, degree, close_continuous)

    curve = interp.curve()
    return curve

def bspline_curve(cp, knots, mults, degree):
    """
    Creates a BSplineCurve from the control points, knots, multiplicites and degree

    :param points: Array of points (numpy array also works!). First dimension over number of points, second must be 3!
    :param knots: Knot vector (not flattened)
    :param mults: Multiplicity vector. Each entry defined the multiplicity of the corresponding knot
    :param degree: Polynomial degree of the resulting b-spline curve.

    :return: The resulting curve (Handle to Geom_BSplineCurve)
    """

    assert(len(knots) == len(mults))
    assert(degree >= 1)

    occ_cp = point_array(cp)
    occ_knots = float_array(knots)
    occ_mults = int_array(mults)
    curve = Geom_BSplineCurve(occ_cp.Array1(), occ_knots.Array1(), occ_mults.Array1(), degree).GetHandle()

    return curve
