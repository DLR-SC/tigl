import numpy as np
from tigl3.curve_factories import bspline_curve
from tigl3.geometry import intersect_bsplines

if __name__ == "__main__":
    """
    This example shows, how to compute the intersection points between two b-spline curves
    """

    cp1 = np.array([[0., 0., 0.],
                    [1., 5., 0.],
                    [2., 0., 0.],
                    [3., 5., 0.],
                    [4., 0., 0.],
                    [5., 5., 0.]])

    cp2 = np.array([[0., 0., 0.],
                    [10., 1., 0.],
                    [-1.62, 2., 0.],
                    [4.78, 3., 0.],
                    [0., 4., 0.],
                    [5., 5., 0.]])

    knots1 = np.array([0., 0.25, 0.5, 0.75, 1.]) * 5.
    mults = [3, 1, 1, 1, 3]

    curve1 = bspline_curve(cp1, knots1, mults, 2)
    curve2 = bspline_curve(cp2, knots1, mults, 2)

    results = intersect_bsplines(curve1, curve2, 0.03)
    print("Number of intersections: " + str(len(results)))

    for result in results:
        p = result.point
        print([p.x, p.y, p.z])

