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

#include "CTiglBSplineFit.h"

#include <Standard_Version.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <math_Matrix.hxx>
#include <math_Gauss.hxx>

#include <BSplCLib.hxx>
#include <Geom_BSplineCurve.hxx>

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

BSplineFit::BSplineFit( int deg, int ncp)
    : _degree(deg)
    , _ncp(ncp)
    , _knots(1, ncp + deg + 1)
{
    computeKnots();
}


/**
 * @brief Copies the curve points
 */
void BSplineFit::copyPoints( const TColgp_Array1OfPnt& points )
{
    unsigned int nPoints = points.Length();

    // curve parameter in form of a arc length
    //t.resize( nPoints );

    // curve coordinates to be fitted by B-spline
    _px.resize( nPoints );
    _py.resize( nPoints );
    _pz.resize( nPoints );

    for (unsigned int i=0; i<nPoints; ++i) {
        gp_Pnt p = points.Value(i+1);
        _px[i] = p.X();
        _py[i] = p.Y();
        _pz[i] = p.Z();
    }
}


/**
 * @brief Initializes linear equation system for least square fit
 */
void BSplineFit::initSystem(math_Matrix& A, math_Vector& rhsx, math_Vector& rhsy, math_Vector& rhsz)
{

    // init matrices and vectors
    A.Init(0.);
    rhsx.Init(0.);
    rhsy.Init(0.);
    rhsz.Init(0.);

    int n_vars = rhsx.Length();

    // order of the B-spline system
    int order = _degree + 1;

    math_Matrix bspl_basis(1, 1, 1, order);

    unsigned int nPoints = _px.size();

    // loop over all curve points which have to be fitted
    for (unsigned int k=0; k<nPoints; ++k ) {

        // set curve parameter
        double tk = t[k];

        int basis_start_index;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,1,0)
        BSplCLib::EvalBsplineBasis(0, order, _knots, tk, basis_start_index, bspl_basis);
#else
        BSplCLib::EvalBsplineBasis(1, 0, order, _knots, tk, basis_start_index, bspl_basis);
#endif
        basis_start_index--;

        int start = std::max(1, basis_start_index);
        int stop  = std::min(n_vars + 1, max(1, basis_start_index + order));

        // first and last basis element
        double N0 = 0.;
        double N1 = 0.;

        if (basis_start_index == 0) {
            N0 = bspl_basis.Value(1,1);
        }

        if (basis_start_index >= _ncp-order) {
            N1 = bspl_basis.Value(1, order);
        }

        // compute matrix values
        for (int i=start; i < stop; ++i ) {
            for (int j=i; j < stop; ++j ) {
                A(i, j) += bspl_basis.Value(1, i+1-basis_start_index)*bspl_basis.Value(1, j+1-basis_start_index);
            }
        }

        // right hand side - the interpolation of end points is taken care of
        for (int i=start; i < stop; i++) {
            rhsx(i) += ( _px[k] -  _px.front() * N0 - _px.back() * N1 ) * bspl_basis.Value(1,i+1-basis_start_index);
            rhsy(i) += ( _py[k] -  _py.front() * N0 - _py.back() * N1 ) * bspl_basis.Value(1,i+1-basis_start_index);
            rhsz(i) += ( _pz[k] -  _pz.front() * N0 - _pz.back() * N1 ) * bspl_basis.Value(1,i+1-basis_start_index);
        }

    } //  loop over all curve points

    // symmetrize the matrix of the LES
    for (int i=2; i <= n_vars; ++i ) {
        for (int j=1; j < i; ++j ) {
            A(i, j) = A(j, i);
        }
    }
}


/**
 * @brief Computes uniform knot vector
 */
void BSplineFit::computeKnots()
{
    int order = _degree + 1;

    // fill multiplicity at start
    for (int i=1; i<=order; ++i ) {
        _knots.SetValue(i, 0.0);
    }

    // number of knots between the multiplicities
    int N = _ncp - order;
    // set uniform knot distribution
    for (int i=1; i<=N; ++i ) {
        _knots.SetValue(order + i, double(i)/double(N+1));
    }

    // fill multiplicity at end
    for (int i=1; i<=order; ++i ) {
        _knots.SetValue(order + N + i, 1.0);
    }
}



 /**
  * @brief Calculates curve parameter t_k [0, 1], which
  * corresponds to the arc lengths
  */
 void BSplineFit::computeParameters(double alpha)
 {
     double sum = 0.0;

     size_t nPoints = _px.size();
     t.resize(nPoints, 0.);
     t[0]=0.0;

     // calc total arc length: dt^2 = dx^2 + dy^2
     for (size_t i=0; i<nPoints-1; i++) {
         double len2 = (_px[i]-_px[i+1])*(_px[i]-_px[i+1])
                 + (_py[i]-_py[i+1])*(_py[i]-_py[i+1])
                 + (_pz[i]-_pz[i+1])*(_pz[i]-_pz[i+1]);
         sum += pow(len2, alpha/2.);
         t[i+1] = sum;
     }

     // normalize parameter with maximum
     double tmax = t[nPoints-1];
     for (size_t i=0; i<nPoints; i++) {
         t[i] /= tmax;
     }

     // reset start and end value to achieve a better accuracy
     t[0] = 0.0;
     t[nPoints-1] = 1.0;
 }

 /**
  * @brief Recalculates the curve parameters t_k after the
  * control points are fitted to achieve an even better fit.
  */
 void BSplineFit::optimizeParameters()
 {
     const int maxIter = 500;  // maximum No of iterations
     const double eps = 1.0E-6;  // accuracy of arc length parameter

     double max_error = 0.;

     unsigned int nPoints = _px.size();

     // optimize each inner parameter t_k
     for (unsigned int k=1; k<nPoints-2;  k++) {
         double f = 0;

         // get old curve parameter value
         double tk = t[k];

         // newton step
         double dt = 0;

         int iter = 0; // iteration counter
         do {  // Newton iteration to get a better t parameter

             // Get the derivatives of the spline wrt parameter t
             gp_Vec p   = _curve->DN(tk, 0);
             gp_Vec dp  = _curve->DN(tk, 1);
             gp_Vec d2p = _curve->DN(tk, 2);

             // compute objective function and their derivative
             f = sqrt((p.X()-_px[k])*(p.X()-_px[k])
                    + (p.Y()-_py[k])*(p.Y()-_py[k])
                    + (p.Z()-_pz[k])*(p.Z()-_pz[k]));

             double df  = (p.X()-_px[k])*dp.X()
                        + (p.Y()-_py[k])*dp.Y()
                        + (p.Z()-_pz[k])*dp.Z();

             double d2f = (p.X()-_px[k])*d2p.X() + (p.Y()-_py[k])*d2p.Y() + (p.Z()-_pz[k])*d2p.Z()
                        + dp.X()*dp.X() + dp.Y()*dp.Y() + dp.Z()*dp.Z();

             // newton iterate
             dt = df/d2f;
             tk -= dt;

             // if parameter out of range reset it to the start value
             if ( tk<t[0] || tk>t[nPoints-1]) {
                 tk = t[k];
             }

             iter++;
         }
         while (fabs(dt)>eps && iter<maxIter);

         if (f > max_error) {
             max_error = f;
         }

         // store optimised parameter
         t[k] = tk;
     }
 }


/** computes maximum error of the fit */
double BSplineFit::GetMaxError()
{
    double max_error = 0.0;

    unsigned int nPoints = _px.size();

    // loop over all fitted curve points
    for (unsigned int k=0; k<nPoints; k++ ) {

        // get curve parameter
        double tk = t[k];
        // get corresponding point from Bezier spline
        gp_Pnt x = _curve->Value(tk);

        double err =  (_px[k]-x.X())*(_px[k]-x.X()) + (_py[k]-x.Y())*(_py[k]-x.Y()) + (_pz[k]-x.Z())*(_pz[k]-x.Z());
        err = sqrt(err);

        if (err > max_error) {
            max_error = err;
        }

    }
    return max_error;
}


/** Fits given points by a Bezier spline with boundary conditions: match boundary points */
BSplineFit::error BSplineFit::Fit( const TColgp_Array1OfPnt& points, const std::vector<double>& parameters)
{
    copyPoints( points );
    t = parameters;

    return fitCurve();

}

BSplineFit::error BSplineFit::Fit(const TColgp_Array1OfPnt &points, double alpha)
{
    copyPoints(points);
    computeParameters(alpha);

    return fitCurve();
}

BSplineFit::error BSplineFit::FitOptimal(const TColgp_Array1OfPnt &points, double alpha, double tolerance, int maxIter)
{
    int iter = 0;
    double residuum = 0.;
    double cumSum = 0., cumSumOld = 0.;

    copyPoints(points);
    computeParameters(alpha);

    do { // outer iteration for curve parameter t
        BSplineFit::error err = fitCurve();

        if (err != NoError) {
            return err;
        }

        // compute residuum
        cumSum = 0.0;
        for (int i=2; i < _curve->NbPoles(); ++i ) {
            gp_Pnt p = _curve->Pole(i);
            cumSum += fabs(p.X() + p.Y() +p.Z());
        }

        residuum = fabs(cumSumOld - cumSum);
        cumSumOld = cumSum;

        optimizeParameters();

        iter++;

    }
    while (residuum > tolerance && iter < maxIter );

    return NoError;
}


Handle_Geom_BSplineCurve BSplineFit::Curve() const
{
    return _curve;
}


/**
 * Fits given points by a Bezier spline
 */
BSplineFit::error BSplineFit::fitCurve()
{
    // check input values
    if (_ncp <= _degree) {
        return InvalidInput;
    }

    if (_ncp >= static_cast<int>(_px.size())) {
        return InvalidInput;
    }

    // copy knots from flat knot vector
    int nknots = _ncp - _degree + 1;
    TColStd_Array1OfReal knots_compact(1, nknots);
    TColStd_Array1OfInteger mults(1, nknots);

    for (int i = 1; i <= nknots; ++i) {
        knots_compact.SetValue(i, _knots.Value(i + _degree));
        mults.SetValue(i, 1);
    }
    mults.SetValue(1, _degree+1);
    mults.SetValue(nknots, _degree+1);

    // control points
    TColgp_Array1OfPnt poles(1, _ncp);

    // interpolate end points
    poles.SetValue(1, gp_Pnt(_px.front(), _py.front(), _pz.front()));
    poles.SetValue(_ncp, gp_Pnt(_px.back(), _py.back(), _pz.back()));

    // we omit the outermost points as we
    // want to interpolate them
    int n_vars = _ncp - 2;

    // solution of the solver
    math_Vector X(1,n_vars);
    math_Vector Y(1,n_vars);
    math_Vector Z(1,n_vars);

    // rhs of the solver
    math_Vector cx(1,n_vars);
    math_Vector cy(1,n_vars);
    math_Vector cz(1,n_vars);

    math_Matrix A(1, n_vars, 1, n_vars);

    initSystem(A, cx, cy, cz);

    math_Gauss solver(A);

    solver.Solve(cx, X);
    if (!solver.IsDone()) {
        return MatrixSingular;
    }

    solver.Solve(cy, Y);
    if (!solver.IsDone()) {
        return MatrixSingular;
    }

    solver.Solve(cz, Z);
    if (!solver.IsDone()) {
        return MatrixSingular;
    }

    // copy solution to control point vector
    for (int i=1; i<_ncp-1; ++i ) {
        gp_Pnt p(X(i), Y(i), Z(i));
        poles.SetValue(i+1, p);
    }

    _curve = new Geom_BSplineCurve(poles, knots_compact, mults, _degree);


    return NoError;
}
