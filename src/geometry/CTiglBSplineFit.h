/**
 * Copyright (C) 2015 DLR-SC
 *
 * Author: Martin Siggel
 *
 * A algorithm that fits a bspline to the given data points.
 *
 * This iterative algorithm works as follows:
 *
 * 0) Initialize parameters t_k with uniform distance
 * 1) Find control points, by minimizing the distance
 *    C(t_k) - p_k for all k using a least square fit.
 * 2) Optimize all t_k, such that the distance C(t_k) - p_k
 *    becomes smaller. Go back to (1)
 */

#ifndef BSPLINEFIT_H
#define BSPLINEFIT_H

#include "tigl_internal.h"

#include <vector>
#include <TColStd_Array1OfReal.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>

class BSplineFit
{

public:
    /**
     * @brief BSplineFit Class to fit a B-spline to data points
     * @param deg Degree of the resulting B-spline
     * @param ncp Number of control points of the B-spline (ncp > deg)
     * @param eps Tolerance of the iterative method
     * @param maxIter Maximum number of iterations
     */
    TIGL_EXPORT BSplineFit(int deg, int ncp);

    enum error
    {
        NoError = 0,
        MatrixSingular,
        InvalidInput
    };


    /// Fits the given points by a B-spline
    TIGL_EXPORT error Fit(const TColgp_Array1OfPnt& points, const std::vector<double>& parameters);

    /// Fits the given points by a B-spline using the centripetal paramterization scheme (alpha=0.5)
    TIGL_EXPORT error Fit(const TColgp_Array1OfPnt& points, double alpha=1.0);

    /// Fits, by optimizing the fit parameters
    /// This is an iterative algorithm and requires more time than the other fit algorithms
    TIGL_EXPORT error FitOptimal(const TColgp_Array1OfPnt& points, double alpha=1.0, double eps=1.0E-3, int maxIter=100);

    /// Returns the resulting curve. Returns
    /// Null in case of an error
    TIGL_EXPORT Handle_Geom_BSplineCurve Curve() const;

    /// Computes the maximum error of the fit
    TIGL_EXPORT double GetMaxError();


private:
    /// Copies the points to be fitted
    void copyPoints( const TColgp_Array1OfPnt& );


    /// Computes the matrix and the right hand side of the system to be solved
    void initSystem(class math_Matrix& A, class math_Vector& rhsx, class math_Vector& rhsy, class math_Vector& rhsz);

    /// Computes an uniform knot vector
    void computeKnots();

    /// Calculates curve parameter t_k [0, 1], which corresponds to the arc lengths
    void computeParameters(double alpha);

    /// Recalculates the curve parameters t_k after the
    /// control points are fitted to achieve an even better fit.
    void optimizeParameters();

    /// Fits the given points by a Bezier spline
    error fitCurve();

    /// The resulting B-spline curve
    Handle_Geom_BSplineCurve _curve;

    /// degree of the B-spline
    int _degree;

    /// Number of control points of the B-spline
    int _ncp;

    /// uniform knot vector of the B-spline
    TColStd_Array1OfReal _knots;

    /// parameters at which the curve is sampled
    std::vector<double> t;

    /// curve coordinates to be fitted by the B-spline
    std::vector<double> _px;
    std::vector<double> _py;
    std::vector<double> _pz;

};

#endif
