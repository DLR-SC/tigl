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


#ifndef TIGLMATHFUNCTIONS_H
#define TIGLMATHFUNCTIONS_H

#include "tigl_internal.h"
#include <vector>

namespace tigl 
{

class CTiglPoint;

/**
 * @brief Computes the binomial coefficient (n,k)
 */
TIGL_EXPORT int binom(int n, int k);

/**
 * @brief Computes the factorial n! in a non recursive fashion
 */
TIGL_EXPORT int factorial(int n);

/**
 * @brief Computes the power x^n
 * @param x Basis
 * @param n Exponent 
 * @return x^n
 */
TIGL_EXPORT double pow_int(double x, int n);

/** 
 * @brief Computes the values of the i-th bernstein polynome 
 * with degree n at position x
 *
 *@param i Number of the polynome with 0 <= i <= n
 *@param n Degree of the polynome, n > 0
 *@param x Value the polynom should be evaluated at (normally 0 <= x <= 1, but not necessarily)
*/
TIGL_EXPORT double bernstein_poly(int i, int n, double x);

/** 
 * @brief Computes the values of the k-th derivative of the 
 * i-th bernstein polynome with degree n at position x
 *
 *@param k Order of derivative
 *@param i Number of the polynome with 0 <= i <= n
 *@param n Degree of the polynome, n > 0
 *@param x Value the polynom should be evaluated at (normally 0 <= x <= 1, but not necessarily)
*/
TIGL_EXPORT double bernstein_poly_deriv(int k, int i, int n, double x);

/**
 * @brief Calculated the area of a quadrilateral defined by the 4 corner points A,B,C,D
 */
TIGL_EXPORT double quadrilateral_area(const CTiglPoint& A, const CTiglPoint& B, const CTiglPoint& C, const CTiglPoint& D);

/**
 * @brief Computes the nth derivative of x^k
 */
TIGL_EXPORT double pow_deriv(double x, double k, int n);

/** @brief defines the class function
 * C(psi) = psi^N1 * (1-psi)^N2
 * for a CST air profile curve
 */
TIGL_EXPORT double class_function(const double& N1, const double& N2, const double& x);

/** @brief defines the derivative of the class function
 * C(psi) = psi^N1 * (1-psi)^N2
 * for a CST air profile curve
 */
TIGL_EXPORT double class_function_deriv(const double& N1, const double& N2, const int& n, const double& x);

/** @brief defines the shape function
 * S(psi)=sum_i=1^N B_i * P_i^n(psi)
 * for a CST air profile curve. The P_i^n are the Bernstein polynomials.
 */
TIGL_EXPORT double shape_function(const std::vector<double>& B, const double& x);

/** @brief defines the derivative of the shape function
 * S(psi)=sum_i=1^N B_i * P_i^n(psi)
 * for a CST air profile curve. The P_i^n are the Bernstein polynomials.
 */
TIGL_EXPORT double shape_function_deriv(const std::vector<double>& B, const int& n, const double& x);

/** @brief defines the CST air profile curve according to
 * "CST" Universal Paramteric Geometry Representation Method
 * with Applications to Supersonic Aircraft, B.M. Kulfan 2007
 * 
 * CST(psi)=C(psi)*S(psi)
 *
 * N1, N2 are the paramters of the class function C(psi) = psi^N1 * (1-psi)^N2
 * B is the vector of coefficients for the bernstein polynomials P_i^n(psi) 
 * inside the shape function S(psi)=sum_i=1^N B_i * p_i^n(psi)
 * The order of the Bernstein polynomials N is defined by the length of the B vector
 */
TIGL_EXPORT double cstcurve(const double& N1, const double& N2, const std::vector<double>& B, const double& x);

/** @brief defines the derivative of the CST air profile curve 
 * CST(psi)=C(psi)*S(psi)
 */
TIGL_EXPORT double cstcurve_deriv(const double& N1, const double& N2, const std::vector<double>& B, const int& n, const double& x);


} // namespace tigl

#endif // TIGLMATHFUNCTIONS_H
