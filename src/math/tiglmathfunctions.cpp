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

#include "CTiglError.h"

#include <cassert>
#include <algorithm>
#include <cmath>

namespace tigl {

/**
 * @brief Computes the binomial coefficient (n,k)
 */
int binom(int n, int k) {
    // this is an efficient implementation, taken from wikipedia
    if (k == 0)
        return 1;

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
int factorial(int n){
    assert(n >= 0);
    
    int result = 1;
    for(int i = 2; i <= n; ++i)
        result *= i;
    
    return result;
}

/**
 * @brief Computes the power x^n
 */
double pow_int(double x, int n) {
    assert(n >= 0);
    
    double result = 1.;
    for(int i = 0; i < n; ++i)
        result *= x;
    
    return result;
}

/** 
 * @brief Computes the values of the i-th bernstein polynome 
 * with degree n at position x
*/
double bernstein_poly(int i, int n, double x) {
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
double bernstein_poly_deriv(int k, int i, int n, double x) {
    if (k <= 0)
        return bernstein_poly(i, n, x);

    // The k-th derivative of a polynom at degree n < k is always zero
    if (k > n)
        return 0.;
    
    int jmin = std::max(0, i + k - n);
    int jmax = std::min(i, k);
    
    double result = 0.;
    for (int j = jmin; j <= jmax; ++j)
        result += pow_int(-1., j + k) * binom(k, j) * bernstein_poly(i - j, n - k, x);
    result *= factorial(n)/factorial(n-k);
    return result;
}

// computes the nth derivative of x^k
double pow_deriv(double x, double k, int n) {
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

} // namespace tigl
