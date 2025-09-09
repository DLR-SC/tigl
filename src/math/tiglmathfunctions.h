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
#include <math_Vector.hxx>
#include <Precision.hxx>
#include "tiglMatrix.h"

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
 * @brief Calculates the distance of a point P from the line defined by a point X0 and direction DX
 */
TIGL_EXPORT double distance_point_from_line(const CTiglPoint& P, const CTiglPoint& X0, const CTiglPoint& DX);

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
 * N1, N2 are the parameters of the class function C(psi) = psi^N1 * (1-psi)^N2
 * B is the vector of coefficients for the bernstein polynomials P_i^n(psi) 
 * T is the trailing edge thickness
 * inside the shape function S(psi)=sum_i=1^N B_i * p_i^n(psi)
 * The order of the Bernstein polynomials N is defined by the length of the B vector
 */
TIGL_EXPORT double cstcurve(const double& N1, const double& N2, const std::vector<double>& B, const double& T, const double& x);

/** @brief defines the derivative of the CST air profile curve 
 * CST(psi)=C(psi)*S(psi)
 */
TIGL_EXPORT double cstcurve_deriv(const double& N1, const double& N2, const std::vector<double>& B, const double& T, const int& n, const double& x);

/**
 * Return true if the value of a is similar to b
 * @param a
 * @param b
 * @param epsilon
 * @return
 */
TIGL_EXPORT bool isNear(double a, double b, double epsilon = Precision::Confusion());

/**
 * 1D Function interface accepting one parameter t and returning
 * the function value
 */
class MathFunc1d
{
public:
    TIGL_EXPORT virtual ~MathFunc1d(){}
    virtual double value(double t) = 0;
};

/**
 * 3D Function interface accepting one parameter t and returning
 * the function value
 */
class MathFunc3d
{
public:
    TIGL_EXPORT virtual ~MathFunc3d(){}
    virtual double valueX(double t) = 0;
    virtual double valueY(double t) = 0;
    virtual double valueZ(double t) = 0;
};

/**
 * @brief Chebycheff approximation of a function f(x), x in [a,b]
 *
 * @throws CTiglError (TIGL_MATH_ERROR) if N <= 0 
 * 
 * @param func The 1D function to approximate
 * @param N Number of function evaluations ( = polynomial degree + 1)
 * @param a First parameter of the function to approximate
 * @param b Last  parameter of the function to approximate
 * @return Chebycheff coefficients
 */
TIGL_EXPORT math_Vector cheb_approx(MathFunc1d& func, int N, double a, double b);


/** 
 * @brief Computes the chebycheff to monomial transformation matrix
 * 
 * Use this NxN matrix to translate chebychev coefficients into monomial
 * coefficients of polynomes.
 * 
 * @param N Dimension of the matrix
 */
TIGL_EXPORT math_Matrix cheb_to_monomial(int N);

/**
* @brief Monomial to Bernstein conversion matrix
*
 * Use this NxN matrix to translate monimial coefficients into bezier
 * coefficients (control points).
*/
TIGL_EXPORT math_Matrix monimial_to_bezier(int N);

/**
* @brief  A function that calculates the polar decomposition of a 3x3 matrix A.
*
* PolarDecomposition(A,U,P) calculates the polar decomposition U,P of the input
* matrix A, such that A = U x P, where U is a unitary matrix and P is a positive
* semi-definite Hermitian matrix. U can be interpreted as a rotation and P as a
* spatial scaling, possibly including off-diaogonal shearing terms
*
* @param A input matrix
* @param U unitary matrix U
* @param P hermitian matrix P
*/
TIGL_EXPORT void PolarDecomposition(tiglMatrix const& A, tiglMatrix& U, tiglMatrix& P);

TIGL_EXPORT void SVD(tiglMatrix const& A, tiglMatrix& U, tiglMatrix& S, tiglMatrix& V);

/**
 * Linear interpolation in of xdata<->ydata array at position x
 */
TIGL_EXPORT double Interpolate(const std::vector<double>& xdata, const std::vector<double>& ydata, double x);

/**
 * @brief Returns true if the matrix is a proper rotation matrix.
 *
 * Check if the matrix is orthogonal and if its determinant is 1.
 *
 * @param R, the 3x3 matrix to check
 * @return true or false
 */
TIGL_EXPORT bool IsRotationMatrix(const tiglMatrix& R);

/**
 * @brief Diagonalize the given matrix using the jacobi method.
 *
 * The symmetric input matrix, M, is decomposed into V, an improper rotation, and D, a diagonal matrix.
 * It is M = V^T*D*V.
 * The value of the diagonal D are the eigenvalues of M and the columns of V are the normalized eigenvectors
 *
 * @remark This method works only for a real symmetric matrix.
 *
 * @param M: the 3x3 matrix to decompose
 * @param D: the 3x3 diagonal result matrix
 * @param V: the 3x3 improper rotation result matrix
 */
TIGL_EXPORT void DiagonalizeMatrixByJacobi(const tiglMatrix& M, tiglMatrix &D, tiglMatrix &V);


/**
 * @brief Return the intrinsic x y' z'' rotation vector of the rotation matrix.
 *
 * @remark If the matrix is not a rotation matrix, a warning is logged, but no error will arise.
 *
 * @param R: the 3x3 rotation matrix
 * @return vector that contains x, y',z'' in degrees
 */
TIGL_EXPORT CTiglPoint RotMatrixToIntrinsicXYZVector(const tiglMatrix& R );

/**
 * Return a vector orthogonal to the direction
 * @param direction
 * @return
 */
TIGL_EXPORT CTiglPoint FindOrthogonalVectorToDirection(CTiglPoint direction);


TIGL_EXPORT CTiglPoint SnapRotation(CTiglPoint rotation, double epsilon = Precision::Confusion());
TIGL_EXPORT double* SnapRotation(double rotation[3], double epsilon = Precision::Confusion());

/**
 * Snaps up the value "number" up to "snapValue" if fabs(number-snapValue) <= dela
 * @param number 
 * @param snapValue
 * @param delta 
 */
TIGL_EXPORT double SnapValue(double number, double snapValue, double delta  = Precision::Confusion());

/**
 * Format the number as a degree angle.
 * This means that the return value is between ]-180,180]
 * and the number is rounded up near the values 0, 90,180,-90.
 *
 * @param degrees
 * @param epsilon
 */
TIGL_EXPORT double SnapAngle(double degrees, double epsilon = Precision::Confusion() );

/**
 * Format the number for scale or length double value,.
 * Simply, round up the number near the values: 0, 1, -1.
 *
 * @param number
 * @param epsilon
 */
TIGL_EXPORT double SnapUnitInterval(double number, double epsilon = Precision::Confusion());
TIGL_EXPORT CTiglPoint SnapUnitInterval(CTiglPoint scaling,  double epsilon = Precision::Confusion());
TIGL_EXPORT double* SnapUnitInterval(double scaling[3],  double epsilon = Precision::Confusion());

} // namespace tigl

#endif // TIGLMATHFUNCTIONS_H
