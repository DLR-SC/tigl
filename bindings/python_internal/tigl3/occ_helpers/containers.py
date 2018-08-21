from OCC.TColStd import TColStd_HArray1OfReal, TColStd_HArray1OfInteger
from OCC.TColgp import TColgp_HArray1OfPnt
from OCC.TColGeom import TColGeom_HArray1OfBSplineCurve
from OCC.gp import gp_Pnt


def float_array(float_list):
    """
    Creates an OpenCASCADE TColStd_HArray1OfReal from
    a list of floats
    :param float_list: List of integer values
    :return: TColStd_HArray1OfReal
    """

    result = TColStd_HArray1OfReal(1, len(float_list))
    for i, value in enumerate(float_list):
        result.SetValue(i+1, value)
    return result


def int_array(int_list):
    """
    Creates an OpenCASCADE TColStd_HArray1OfInteger from
    a list of integers
    :param int_list: List of integer values
    :return: TColStd_HArray1OfInteger
    """
    result = TColStd_HArray1OfInteger(1, len(int_list))
    for i, value in enumerate(int_list):
        result.SetValue(i, value)
    return result


def point_array(points):
    """
    Returns a TColgp_HArray1OfPnt from a list
    of points given as arrays / tuples

    Ex. points = [(0,0,0), (1,2,3)]

    Note, each point of the array must be 3-dimensional!

    :param points: list of points
    :return: TColgp_HArray1OfPnt
    """

    pnts = TColgp_HArray1OfPnt(1, len(points))
    for i, py_pnt in enumerate(points):
        pnt = gp_Pnt(*py_pnt)
        pnts.SetValue(i+1, pnt)
    return pnts


def bspline_array(bspline_list):
    """
    Create a TColGeom_HArray1OfBSplineCurve from a list of Geom_BSplineCurves
    :param bspline_list: list of TColGeom_HArray1OfBSplineCurve
    :return: TColGeom_HArray1OfBSplineCurve
    """

    result = TColGeom_HArray1OfBSplineCurve(1, len(bspline_list))
    for i, curve in enumerate(bspline_list):
        result.SetValue(i, curve)
    return result


def iter_array(occ_array):
    """
    Provides a generator for an OpenCASCADE array
    """

    if hasattr(occ_array, "GetObject"):
        occ_array = occ_array.GetObject()

    for i in range(occ_array.Lower(), occ_array.Upper() + 1):
        yield(occ_array.Value(i))
