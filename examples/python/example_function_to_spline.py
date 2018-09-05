"""
    This is an example, where we fit a b-spline
    to the analytic function y = x^2

    Technically we derive the Python Class Parabola from
    a C++ Interface MathFunc3d and hand over this object
    to the tigl function CFunctionToBspline
"""

from tigl3.geometry import CFunctionToBspline
from tigl3.tmath import MathFunc3d

import numpy as np
from matplotlib import pyplot as plt


def parabola_function(x):
    return x*x


class TiGLFunctionWrapper(MathFunc3d):

    def __init__(self, function_to_approx):
        self.function = function_to_approx
        super(TiGLFunctionWrapper, self).__init__()

    def value_x(self, t):
        return t

    def value_y(self, t):
        return self.function(t)

    def value_z(self, t):
        return 0.


def fit_bspline_to_parabola():
    """
    Do the actual fitting
    """

    # create the function we want to approximate
    tigl_parabola = TiGLFunctionWrapper(parabola_function)

    # approximate the parabola from x = -1 ... 1 with a degree 3 B-spline
    approx_algo = CFunctionToBspline(tigl_parabola, umin=-1., umax=1., degree=3, tolerance=1e-3)

    # we get a Geom_BSplineCurve as a result
    bspline_curve = approx_algo.curve().GetObject()
    # get the error of the fir
    error = approx_algo.approx_error()

    print("Approximation error:", error)

    # get the control points
    cp = np.zeros((bspline_curve.NbPoles(), 2))
    for icp in range(0, cp.shape[0]):
        pnt = bspline_curve.Pole(icp+1)
        cp[icp, :] = [pnt.X(), pnt.Y()]

    # plot the control points and the parabola
    plt.figure()
    x = np.linspace(-1, 1, 100)
    y = parabola_function(x)
    plt.plot(x, y, 'b-')
    plt.plot(cp[:, 0], cp[:, 1], 'rx-')

    plt.show()


if __name__ == '__main__':
    fit_bspline_to_parabola()
