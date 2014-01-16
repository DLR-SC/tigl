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


namespace tigl {

class CTiglPoint;

/**
 * @brief Computes the binomial coefficient (n,k)
 */
int binom(int n, int k);

/**
 * @brief Computes the factorial n! in a non recursive fashion
 */
int factorial(int n);

/**
 * @brief Computes the power x^n
 * @param x Basis
 * @param n Exponent 
 * @return x^n
 */
double pow_int(double x, int n);

/** 
 * @brief Computes the values of the i-th bernstein polynome 
 * with degree n at position x
 *
 *@param i Number of the polynome with 0 <= i <= n
 *@param n Degree of the polynome, n > 0
 *@param x Value the polynom should be evaluated at (normally 0 <= x <= 1, but not necessarily)
*/
double bernstein_poly(int i, int n, double x);

/** 
 * @brief Computes the values of the k-th derivative of the 
 * i-th bernstein polynome with degree n at position x
 *
 *@param k Order of derivative
 *@param i Number of the polynome with 0 <= i <= n
 *@param n Degree of the polynome, n > 0
 *@param x Value the polynom should be evaluated at (normally 0 <= x <= 1, but not necessarily)
*/
double bernstein_poly_deriv(int k, int i, int n, double x);

/**
 * @brief Calculated the area of a quadrilateral defined by the 4 corner points A,B,C,D
 */
double quadrilateral_area(const CTiglPoint& A, const CTiglPoint& B, const CTiglPoint& C, const CTiglPoint& D);

} // namespace tigl

#endif // TIGLMATHFUNCTIONS_H
