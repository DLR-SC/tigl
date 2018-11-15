/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-07-24 Martin Siggel <Martin.Siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "tiglmathfunctions.h"
#include "CTiglError.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <vector>

#include <CTiglPoint.h>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <math_Matrix.hxx>

namespace tigl 
{

/**
 * @brief Computes the binomial coefficient (n,k)
 */
int binom(int n, int k) 
{
    // this is an efficient implementation, taken from wikipedia
    if (k == 0) {
        return 1;
    }

    if (2*k > n) {
        return binom(n, n-k);
    }
    else {
        int result = n-k+1;
        for (int i = 2; i <= k; ++i){
            result *= (n - k + i);
            result /= i;
        }
        return result;
    }
}

/**
 * @brief Computes the factorial n! in a non recursive fashion
 */
int factorial(int n)
{
    assert(n >= 0);
    
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    
    return result;
}

/**
 * @brief Computes the power x^n
 */
double pow_int(double x, int n) 
{
    assert(n >= 0);
    
    double result = 1.;
    for (int i = 0; i < n; ++i) {
        result *= x;
    }
    
    return result;
}

/** 
 * @brief Computes the values of the i-th bernstein polynome 
 * with degree n at position x
*/
double bernstein_poly(int i, int n, double x) 
{
    assert(i <= n);
    return (double)binom(n,i) * pow_int(x, i) * pow_int(1.-x, n-i);
}

/** 
 * @brief Computes the values of the k-th derivative of the 
 * i-th bernstein polynome with degree n at position x
 *
 * Equation taken from: 
 * Doha et al (2011): On the Derivatives of Bernstein Polynomials: An Application for
 * the Solution of High Even-Order Differential Equations
*/
double bernstein_poly_deriv(int k, int i, int n, double x) 
{
    if (k <= 0) {
        return bernstein_poly(i, n, x);
    }

    // The k-th derivative of a polynom at degree n < k is always zero
    if (k > n) {
        return 0.;
    }
    
    int jmin = std::max(0, i + k - n);
    int jmax = std::min(i, k);
    
    double result = 0.;
    for (int j = jmin; j <= jmax; ++j) {
        result += pow_int(-1., j + k) * binom(k, j) * bernstein_poly(i - j, n - k, x);
    }
    result *= factorial(n)/factorial(n-k);
    return result;
}

/**
 * @brief Calculated the area of a quadrilateral defined by the 4 corner points A,B,C,D
 */
double quadrilateral_area(const CTiglPoint& A, const CTiglPoint& B, const CTiglPoint& C, const CTiglPoint& D) 
{
    gp_Vec AC(A.Get_gp_Pnt(),C.Get_gp_Pnt());
    gp_Vec BD(B.Get_gp_Pnt(),D.Get_gp_Pnt());

    return 0.5 * AC.CrossMagnitude(BD);
}

/**
 * @brief Calculates the distance of a point P from the line defined by a point X0 and direction DX
 */
double distance_point_from_line(const CTiglPoint& P, const CTiglPoint& X0, const CTiglPoint& DX)
{
    double lenDX = DX.norm2();
    assert(lenDX > 0.);

    return CTiglPoint::cross_prod(DX, X0-P).norm2()/lenDX;
}

/** 
 * @brief Computes the nth derivative of x^k
 */
double pow_deriv(double x, double k, int n) 
{
    assert(n >= 0);

    if (n == 0) {
        return pow(x,k);
    }

    double eps = 1e-15;

    // check if k is a natural number. if yes, we have to return 0 if n > k
    if ( fabs(k - int(k)) < eps && n > k) {
        return 0.;
    }
    else {
        double fact = 1.;
        for (int i = 0; i < n; ++i){
            fact *= (k-i);
        }
        return fact * pow(x, k - (double)n);
    }
}

/** @brief defines the class function
 * C(psi) = psi^N1 * (1-psi)^N2
 * for a CST air profile curve
 */
double class_function(const double& N1, const double& N2, const double& x)
{
    return pow(x,N1) * pow(1-x,N2);
}
/** @brief defines the derivative of the class function
 * C(psi) = psi^N1 * (1-psi)^N2
 * for a CST air profile curve
 */
double class_function_deriv(const double& N1, const double& N2, const int& n, const double& x)
{
    double res = 0.;
    for (int i = 0; i <= n; i++) {
        res += tigl::binom(n,i)
             * tigl::pow_deriv(x,N1,i)
             * tigl::pow_deriv(1-x, N2, n-i)
             * tigl::pow_int(-1., n-i);
    }
    return res;
}
/** @brief defines the shape function
 * S(psi)=sum_i=1^N B_i * P_i^n(psi)
 * for a CST air profile curve. The P_i^n are the Bernstein polynomials.
 */
double shape_function(const std::vector<double>& B, const double& x)
{
    double ret = 0.;
    int order = static_cast<int>(B.size()) - 1;
    int i = 0;
    for (std::vector<double>::const_iterator bIT = B.begin(); bIT != B.end(); ++bIT, ++i) {
        ret += *bIT * bernstein_poly(i, order, x);
    }
    return ret;
}
/** @brief defines the derivative of the shape function
 * S(psi)=sum_i=1^N B_i * P_i^n(psi)
 * for a CST air profile curve. The P_i^n are the Bernstein polynomials.
 */
double shape_function_deriv(const std::vector<double>& B, const int& n, const double& x)
{
    double ret = 0.;
    int order = static_cast<int>(B.size()) - 1;
    int i = 0;
    for (std::vector<double>::const_iterator bIT = B.begin(); bIT != B.end(); ++bIT, ++i) {
        ret += *bIT * tigl::bernstein_poly_deriv(n, i, order, x);
    }
    return ret;
}
/** @brief defines the CST air profile curve according to
 * "CST" Universal Paramteric Geometry Representation Method
 * with Applications to Supersonic Aircraft, B.M. Kulfan 2007
 * 
 * CST(psi)=C(psi)*S(psi)
 *
 * N1, N2 are the paramters of the class function C(psi) = psi^N1 * (1-psi)^N2
 * B is the vector of coefficients for the bernstein polynomials P_i^n(psi) 
 * T is the trailing edge thickness
 * inside the shape function S(psi)=sum_i=1^N B_i * p_i^n(psi)
 * The order of the Bernstein polynomials N is defined by the length of the B vector
 */
double cstcurve(const double& N1, const double& N2, const std::vector<double>& B, const double& T, const double& x)
{
        return class_function(N1, N2, x) * shape_function(B, x) + x*T;
}
/** @brief defines the derivative of the CST air profile curve 
 * CST(psi)=C(psi)*S(psi)
 */
double cstcurve_deriv(const double& N1, const double& N2, const std::vector<double>& B, const double& T, const int& n, const double& x)
{
    double res = 0.;
    for (int i= 0; i<= n; i++) {
        res += tigl::binom(n,i)
             * class_function_deriv(N1, N2, i, x)
             * shape_function_deriv(B, n-i, x);
    }
    if (n == 1) {
        res += T;
    }
    
    return res;
}

/**
 * @brief Chebycheff approximation of a function f(x), x in [a,b]
 *
 * @param func The 1D funktion to approximate
 * @param parms Additional data passed to the function func (may be NULL)
 * @param N Number of function evaluations ( = polynomial degree + 1)
 * @param a First parameter of the function to approximate
 * @param b Last  parameter of the function to approximate
 * @return Chebycheff coefficients
 */
math_Vector cheb_approx(MathFunc1d& func, int N, double a, double b)
{
    if (N <= 0) {
        throw CTiglError("N <= 0 in cheb_approx", TIGL_MATH_ERROR);
    }
    math_Vector c(0, N-1);
    
    double * fx = new double[N];
    for (int k = 1; k <= N; ++k) {
        double x = cos(M_PI * ((double)k - 0.5) / (double)N);
        // shift to correct intervall
        x = (x + 1.) / 2. *(b-a) + a;
        
        // evaluate function at x
        fx[k-1] = func.value(x);
    }
    
    for (int j = 0; j < N; ++j) {
        double cj = 0;
        for (int k = 1; k <= N; ++k) {
            double v = fx[k-1] * cos(M_PI * (double)j * ((double)k - 0.5)/ (double)N);
            cj += 2./(double) N * v;
        }
        c(j) = cj;
    }
    delete[] fx;
    
    c(0) *= 0.5;
    
    return c;
}

/// computes the chebycheff to monomial transformation matrix
math_Matrix cheb_to_monomial(int N)
{
    math_Matrix matrix(0,N-1, 0, N-1, 0.);
    
    matrix(0,0) =1.;
    matrix(0,1) =-1.;
    matrix(1,1) =2.;
    
    for (int j = 2; j < N; ++j) {
        for (int i = 0; i <= j; ++i) {
            double val = 0.;
            if (i > 0) {
                val += 4.*matrix(i-1, j-1);
            }
            val -= 2.*matrix(i,j-1) + matrix(i,j-2);
            
            matrix(i,j)= val;
        }
    }
    return matrix;
}

/**
* @brief Monomial to Bernstein conversion matrix
*
* Returns a NxN matrix to be multiplied with
* a vector of Monomial coefficients,
* generating a vector of Bernstein coefficients.
*/
math_Matrix monimial_to_bezier(int N)
{
    math_Matrix matrix(0,N-1, 0, N-1, 0.);
    
    for (int j=0; j < N; ++j) {
        for (int i = j; i < N; ++i) {
            double div1= binom(i, i-j);
            double div2= binom(N-1, j);
            matrix(i,j) = div1 / div2;
        }
    }
    
    return matrix;
}

} // namespace tigl
