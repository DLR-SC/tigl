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

#include "tiglMatrix.h"

#include <math_Recipes.hxx>
#include "CTiglLogging.h"
#include "tiglcommonfunctions.h"

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

void PolarDecomposition(tiglMatrix const&A, tiglMatrix& U, tiglMatrix& P)
{
    // calculate SVD
    tiglMatrix u(1, 3, 1, 3);
    tiglMatrix s(1, 3, 1, 3);
    tiglMatrix v(1, 3, 1, 3);
    SVD(A, u, s, v);

    // U = uv*
    U = u*v.Transposed();

    // P = vsv*
    P = v*s*v.Transposed();
}

void SVD(tiglMatrix const& A, tiglMatrix& U, tiglMatrix& S, tiglMatrix& V)
{

    tiglVector Sv(1,3);
    U = A;

    SVD_Decompose(U, Sv, V);

    S(1,1) = Sv(1);
    S(1,2) = 0.;
    S(1,3) = 0.;

    S(2,1) = 0.;
    S(2,2) = Sv(2);
    S(2,3) = 0.;

    S(3,1) = 0.;
    S(3,2) = 0.;
    S(3,3) = Sv(3);
}

bool isNear(double a, double b, double epsilon)
{
    return (fabs(a - b) <= epsilon);
}

bool IsProperRotationMatrix(tiglMatrix& R)
{
    tiglMatrix Rt(1, 3, 1, 3);
    Rt = R.Transposed();
    tiglMatrix shouldBeI(1, 3, 1, 3);
    shouldBeI = Rt * R;
    tiglMatrix I(1, 3, 1, 3, 0.0);
    I(1, 1) = 1;
    I(2, 2) = 1;
    I(3, 3) = 1;

    // check if Rt*R is identity,
    bool identity = true;
    for (int i = 1; i < 4; i++) {
         for (int j = 1; j < 4; j++) {
            if (! isNear(I(i, j), shouldBeI(i, j)) ) {
                identity = false;
            }
        }
    }

    double det = R.Determinant();
    return (isNear(det, 1) && identity);
}

void DiagonalizeMatrixByJacobi(tiglMatrix S, tiglMatrix &D, tiglMatrix &V)
{

    tiglVector d(1, 3);
    Standard_Integer nrot;
    Jacobi(S, d, V, nrot);

    D.Init(0.0);
    D(1, 1) = d(1);
    D(2, 2) = d(2);
    D(3, 3) = d(3);

    tiglMatrix res(1, 3, 1, 3, 0);
}

CTiglPoint RotMatrixToIntrinsicXYZVector(tiglMatrix& R)
{

    // calculate intrinsic Euler angles from rotation matrix R
    //
    // This implementation is based on http://www.gregslabaugh.net/publications/euler.pdf, where the same argumentation
    // is used for intrinsic x,y',z'' angles and the rotatation matrix
    //
    //                |                        cos(y)*cos(z) |               -        cos(y)*cos(z) |         sin(y) |
    // U = Rx*Ry*Rz = | cos(x)*sin(z) + sin(x)*sin(y)*cos(z) | cos(x)*cos(z) - sin(x)*sin(y)*sin(z) | -sin(x)*cos(y) |
    //                | sin(x)*sin(z) - cos(x)*sin(y)*cos(z) | sin(x)*cos(z) + cos(x)*sin(y)*sin(z) |  cos(x)*cos(y) |
    //

    // cas sin(y) = +/- 1
    //                |                        -            |          -                           |         sin(y) |
    // U = 0*Ry*Rz =  |                 sin(z)              |                cos(z)                | -              |
    //                |
    //


    // rather than extrinsic angles and the Rotation matrix mentioned in that pdf.

    if (!IsProperRotationMatrix(R)) {
        LOG(ERROR) << "RotMatrixToIntrinsicXYZVector: The given rotation matrix seems not to be a proper rotation "
                      "matrix, the rotation will be computed but the result is probably false.";
    }

    CTiglPoint rotation(0,0,0);

    if (fabs(fabs(R(1, 3)) - 1) > 1e-10) {
        rotation.y     = asin(R(1, 3));
        double cosTheta = cos(rotation.y);
        rotation.x    = -atan2(R(2, 3) / cosTheta, R(3, 3) / cosTheta);
        rotation.z     = -atan2(R(1, 2) / cosTheta, R(1, 1) / cosTheta);
    }
    else {
        rotation.x = 0;
        if ( R(1, 3)  < 0 )  {  // r(1,3) == -1
            rotation.z = atan2(R(2, 1), R(2, 2));
            rotation.y = -M_PI / 2;
        }
        else {      // r(1,3) == 1
            rotation.z = atan2(R(2, 1), R(2, 2));
            rotation.y = M_PI / 2;
        }
    }

    rotation.x = Degrees(rotation.x);
    rotation.y = Degrees(rotation.y);
    rotation.z = Degrees(rotation.z);

    return rotation;
}



CTiglPoint FindOrthogonalVectorToDirection(CTiglPoint d)
{
    d.normalize();
    if (fabs(d.z) >= fabs(d.x) && fabs(d.z) >= fabs(d.y)) {
        return CTiglPoint(1,1,-(d.x + d.y)/d.z );
    }
    else if (fabs(d.y) >= fabs(d.x) && fabs(d.y) >= fabs(d.z)) {
        return CTiglPoint(1, -(d.x + d.z)/d.y ,1);
    }
    else {
        return CTiglPoint( -(d.y +d.z)/d.x ,1,1) ;
    }

}

void RotationRounding(CTiglPoint& rotation, double epsilon)
{
    // round up 0
    Rounding(rotation.x, 0, epsilon);
    Rounding(rotation.y, 0, epsilon);
    Rounding(rotation.z, 0, epsilon);

    Rounding(rotation.x, 90, epsilon);
    Rounding(rotation.y, 90, epsilon);
    Rounding(rotation.z, 90, epsilon);

    Rounding(rotation.x, -90, epsilon);
    Rounding(rotation.y, -90, epsilon);
    Rounding(rotation.z, -90, epsilon);

    Rounding(rotation.x, 180, epsilon);
    Rounding(rotation.y, 180, epsilon);
    Rounding(rotation.z, 180, epsilon);

    Rounding(rotation.x, -180, epsilon);
    Rounding(rotation.y, -180, epsilon);
    Rounding(rotation.z, -180, epsilon);

    Rounding(rotation.x, 270, epsilon);
    Rounding(rotation.y, 270, epsilon);
    Rounding(rotation.z, 270, epsilon);

    Rounding(rotation.x, 360, epsilon);
    Rounding(rotation.y, 360, epsilon);
    Rounding(rotation.z, 360, epsilon);

    // change 360 to 0
    if (rotation.x == 360) {
        rotation.x = 0;
    }
    if (rotation.y == 360) {
        rotation.y = 0;
    }
    if (rotation.z == 360) {
        rotation.z = 0;
    }

    Rounding(rotation.x, -360, epsilon);
    Rounding(rotation.y, -360, epsilon);
    Rounding(rotation.z, -360, epsilon);

    // change -360 to 0
    if (rotation.x == -360) {
        rotation.x = 0;
    }
    if (rotation.y == -360) {
        rotation.y = 0;
    }
    if (rotation.z == -360) {
        rotation.z = 0;
    }

}

void ScalingRounding(CTiglPoint& scaling,  double epsilon)
{
    // round up 0
    Rounding(scaling.x,0,epsilon);
    Rounding(scaling.y,0,epsilon);
    Rounding(scaling.z,0,epsilon);

    Rounding(scaling.x,1,epsilon);
    Rounding(scaling.y,1,epsilon);
    Rounding(scaling.z,1,epsilon);

    Rounding(scaling.x,-1,epsilon);
    Rounding(scaling.y,-1,epsilon);
    Rounding(scaling.z,-1,epsilon);

}

void TranslationRounding(CTiglPoint& translation, double epsilon)
{
    // round up 0
    Rounding(translation.x, 0, epsilon);
    Rounding(translation.y, 0, epsilon);
    Rounding(translation.z, 0, epsilon);
}

void Rounding(double& number, double roundingValue, double delta) 
{
    if ( fabs(number-roundingValue) < delta) {
        number = roundingValue; 
    }
}


} // namespace tigl
