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

#include "test.h"
#include "math/tiglmathfunctions.h"
#include <vector>

#include "CTiglPoint.h"
#include "CTiglTransformation.h"

#include "CCPACSTransformation.h"


TEST(TiglMath, factorial)
{
    ASSERT_EQ(1, tigl::factorial(0));
    ASSERT_EQ(1, tigl::factorial(1));
    ASSERT_EQ(2, tigl::factorial(2));
    ASSERT_EQ(6, tigl::factorial(3));
    ASSERT_EQ(24, tigl::factorial(4));
}

TEST(TiglMath, binom)
{
    ASSERT_EQ(1, tigl::binom(0,0));
    
    ASSERT_EQ(1, tigl::binom(3, 0));
    ASSERT_EQ(3, tigl::binom(3, 1));
    ASSERT_EQ(3, tigl::binom(3, 2));
    ASSERT_EQ(1, tigl::binom(3, 3));
    
    ASSERT_EQ(1,  tigl::binom(5, 0));
    ASSERT_EQ(5,  tigl::binom(5, 1));
    ASSERT_EQ(10, tigl::binom(5, 2));
    ASSERT_EQ(10, tigl::binom(5, 3));
    ASSERT_EQ(5,  tigl::binom(5, 4));
    ASSERT_EQ(1,  tigl::binom(5, 5));
    
    ASSERT_EQ(1,  tigl::binom(7, 0));
    ASSERT_EQ(7,  tigl::binom(7, 1));
    ASSERT_EQ(21, tigl::binom(7, 2));
    ASSERT_EQ(35, tigl::binom(7, 3));
    ASSERT_EQ(35, tigl::binom(7, 4));
    ASSERT_EQ(21, tigl::binom(7, 5));
    ASSERT_EQ(7,  tigl::binom(7, 6));
    ASSERT_EQ(1,  tigl::binom(7, 7));
    
    ASSERT_EQ(330, tigl::binom(11, 4));
}

TEST(TiglMath, BernsteinPoly)
{
    ASSERT_NEAR(1.0, tigl::bernstein_poly(0,1,0.0), 1e-7);
    ASSERT_NEAR(0.5, tigl::bernstein_poly(0,1,0.5), 1e-7);
    ASSERT_NEAR(0.0, tigl::bernstein_poly(0,1,1.0), 1e-7);
    
    ASSERT_NEAR(0.0, tigl::bernstein_poly(1,1,0.0), 1e-7);
    ASSERT_NEAR(0.5, tigl::bernstein_poly(1,1,0.5), 1e-7);
    ASSERT_NEAR(1.0, tigl::bernstein_poly(1,1,1.0), 1e-7);
    
    ASSERT_NEAR(1.0, tigl::bernstein_poly(0,4,0.), 1e-7);
    ASSERT_NEAR(0.0, tigl::bernstein_poly(0,4,1.), 1e-7);
    
    ASSERT_NEAR(0.0, tigl::bernstein_poly(4,4,0.), 1e-7);
    ASSERT_NEAR(1.0, tigl::bernstein_poly(4,4,1.), 1e-7);
    
    ASSERT_NEAR(0.375, tigl::bernstein_poly(2,4,0.5), 1e-7);
}

TEST(TiglMath, QuadrilateralArea)
{
    // set square with side length 3
    using tigl::CTiglPoint;
    CTiglPoint A(0,0,0);
    CTiglPoint B(3,0,0);
    CTiglPoint C(3,3,0);
    CTiglPoint D(0,3,0);
    ASSERT_NEAR(9., tigl::quadrilateral_area(A,B,C,D), 1e-7);

    // rectangle
    C.y = 1.5;
    D.y = 1.5;
    ASSERT_NEAR(4.5, tigl::quadrilateral_area(A,B,C,D), 1e-7);

    // trapezoid
    A = CTiglPoint(0,0,0);
    B = CTiglPoint(4,0,0);
    C = CTiglPoint(3,1,0);
    D = CTiglPoint(2,1,0);
    ASSERT_NEAR(2.5, tigl::quadrilateral_area(A,B,C,D), 1e-7);

    // parallelogram
    A = CTiglPoint(0,0,0);
    B = CTiglPoint(4,0,0);
    C = CTiglPoint(5,2,0);
    D = CTiglPoint(1,2,0);
    ASSERT_NEAR(8., tigl::quadrilateral_area(A,B,C,D), 1e-7);

    // triangle
    A = CTiglPoint(0,0,0);
    B = CTiglPoint(4,0,0);
    C = CTiglPoint(2,2,0);
    D = CTiglPoint(0,0,0);
    ASSERT_NEAR(4., tigl::quadrilateral_area(A,B,C,D), 1e-7);
}

TEST(TiglMath, CSTCurve)
{
    // Sample coefficients for shape function
    std::vector<double> Br;
    Br.push_back(0.4);
    Br.push_back(1.0);
    Br.push_back(0.8);
    Br.push_back(10.2);
    
    // Constant sample coefficients for shape function
    std::vector<double> B1(10,1.0);
    // Sample exponents for class function
    double N1=0.1;
    double N2=0.5;

    // check that shape function is constant for B constant
    ASSERT_NEAR(1.0, tigl::shape_function(B1, 0.0), 1e-7);
    ASSERT_NEAR(1.0, tigl::shape_function(B1, 0.5), 1e-7);
    ASSERT_NEAR(1.0, tigl::shape_function(B1, 1.0), 1e-7);

    // check cst curve at some points
    ASSERT_NEAR(0.0, tigl::cstcurve(N1, N2, Br, 0.0, 0.0), 1e-7);
    ASSERT_NEAR(0.568964089203402, tigl::cstcurve(N1, N2, Br, 0., 0.2), 1e-7);
    ASSERT_NEAR(2.325867218509732, tigl::cstcurve(N1, N2, Br, 0., 0.75), 1e-7);
    ASSERT_NEAR(0.0, tigl::cstcurve(N1, N2, Br, 0., 1.0), 1e-7);

    // Sample exponents >1 for derivative 
    N1=1.1;
    N2=4.5;
    // check 1st derivative of cst curve at some points
    ASSERT_NEAR(0.0, tigl::cstcurve_deriv(N1, N2, Br, 0., 1, 0.0), 1e-7);
    ASSERT_NEAR(0.0, tigl::cstcurve_deriv(N1, N2, Br, 0., 1, 1.0), 1e-7);
    // check 1st derivative of cst curve at maximum of cstcurve (found numerically)
    ASSERT_NEAR(0.0, tigl::cstcurve_deriv(N1, N2, Br, 0., 1, 0.322954559162619), 1e-7);
}

TEST(TiglMath, Tchebycheff2Bezier_N3)
{
    math_Matrix Mmt = tigl::cheb_to_monomial(3);
    math_Matrix Mbm = tigl::monimial_to_bezier(3);
    
    // see Watkins 1988: degree reduction of bezier curves, p. 403
    
    math_Matrix M = Mbm * Mmt;
    M.Transpose();
    ASSERT_NEAR(1., M.Value(0,0), 1e-12);
    ASSERT_NEAR(1., M.Value(0,1), 1e-12);
    ASSERT_NEAR(1., M.Value(0,2), 1e-12);
    
    ASSERT_NEAR(-1., M.Value(1,0), 1e-12);
    ASSERT_NEAR( 0., M.Value(1,1), 1e-12);
    ASSERT_NEAR( 1., M.Value(1,2), 1e-12);
    
    ASSERT_NEAR( 1., M.Value(2,0), 1e-12);
    ASSERT_NEAR(-3., M.Value(2,1), 1e-12);
    ASSERT_NEAR( 1., M.Value(2,2), 1e-12);
}

TEST(TiglMath, Tchebycheff2Bezier_N4)
{
    math_Matrix Mmt = tigl::cheb_to_monomial(4);
    math_Matrix Mbm = tigl::monimial_to_bezier(4);
    
    // see Watkins 1988: degree reduction of bezier curves, p. 403
    
    math_Matrix M = Mbm * Mmt * 3;
    M.Transpose();
    ASSERT_NEAR(3., M.Value(0,0), 1e-12);
    ASSERT_NEAR(3., M.Value(0,1), 1e-12);
    ASSERT_NEAR(3., M.Value(0,2), 1e-12);
    ASSERT_NEAR(3., M.Value(0,3), 1e-12);
    
    ASSERT_NEAR(-3., M.Value(1,0), 1e-12);
    ASSERT_NEAR(-1., M.Value(1,1), 1e-12);
    ASSERT_NEAR(1.,  M.Value(1,2), 1e-12);
    ASSERT_NEAR(3.,  M.Value(1,3), 1e-12);
    
    ASSERT_NEAR(3.,  M.Value(2,0), 1e-12);
    ASSERT_NEAR(-5., M.Value(2,1), 1e-12);
    ASSERT_NEAR(-5., M.Value(2,2), 1e-12);
    ASSERT_NEAR(3.,  M.Value(2,3), 1e-12);
    
    ASSERT_NEAR(-3.,  M.Value(3,0), 1e-12);
    ASSERT_NEAR(15.,  M.Value(3,1), 1e-12);
    ASSERT_NEAR(-15., M.Value(3,2), 1e-12);
    ASSERT_NEAR(3.,   M.Value(3,3), 1e-12);
}

TEST(TiglMath, Tchebycheff2Bezier_N5)
{
    math_Matrix Mmt = tigl::cheb_to_monomial(5);
    math_Matrix Mbm = tigl::monimial_to_bezier(5);
    
    // see Watkins 1988: degree reduction of bezier curves p. 403
    
    math_Matrix M = Mbm * Mmt * 6;
    M.Transpose();
    ASSERT_NEAR(6., M.Value(0,0), 1e-12);
    ASSERT_NEAR(6., M.Value(0,1), 1e-12);
    ASSERT_NEAR(6., M.Value(0,2), 1e-12);
    ASSERT_NEAR(6., M.Value(0,3), 1e-12);
    ASSERT_NEAR(6., M.Value(0,4), 1e-12);
    
    ASSERT_NEAR(-6., M.Value(1,0), 1e-12);
    ASSERT_NEAR(-3., M.Value(1,1), 1e-12);
    ASSERT_NEAR(0.,  M.Value(1,2), 1e-12);
    ASSERT_NEAR(3.,  M.Value(1,3), 1e-12);
    ASSERT_NEAR(6.,  M.Value(1,4), 1e-12);
    
    ASSERT_NEAR(6.,   M.Value(2,0), 1e-12);
    ASSERT_NEAR(-6.,  M.Value(2,1), 1e-12);
    ASSERT_NEAR(-10., M.Value(2,2), 1e-12);
    ASSERT_NEAR(-6.,  M.Value(2,3), 1e-12);
    ASSERT_NEAR(6,    M.Value(2,4), 1e-12);
    
    ASSERT_NEAR(-6., M.Value(3,0), 1e-12);
    ASSERT_NEAR(21., M.Value(3,1), 1e-12);
    ASSERT_NEAR(0.,  M.Value(3,2), 1e-12);
    ASSERT_NEAR(-21., M.Value(3,3), 1e-12);
    ASSERT_NEAR(6.,   M.Value(3,4), 1e-12);
    
    ASSERT_NEAR(6.,   M.Value(4,0), 1e-12);
    ASSERT_NEAR(-42., M.Value(4,1), 1e-12);
    ASSERT_NEAR(70.,  M.Value(4,2), 1e-12);
    ASSERT_NEAR(-42., M.Value(4,3), 1e-12);
    ASSERT_NEAR(6.,   M.Value(4,4), 1e-12);
}

TEST(TiglMath, Tchebycheff2Bezier_N6)
{
    math_Matrix Mmt = tigl::cheb_to_monomial(6);
    math_Matrix Mbm = tigl::monimial_to_bezier(6);
    
    // see Watkins 1988: degree reduction of bezier curves p. 404
    
    math_Matrix M = Mbm * Mmt * 5;
    M.Transpose();
    ASSERT_NEAR(5., M.Value(0,0), 1e-12);
    ASSERT_NEAR(5., M.Value(0,1), 1e-12);
    ASSERT_NEAR(5., M.Value(0,2), 1e-12);
    ASSERT_NEAR(5., M.Value(0,3), 1e-12);
    ASSERT_NEAR(5., M.Value(0,4), 1e-12);
    ASSERT_NEAR(5., M.Value(0,5), 1e-12);

    ASSERT_NEAR(-5., M.Value(1,0), 1e-12);
    ASSERT_NEAR(-3., M.Value(1,1), 1e-12);
    ASSERT_NEAR(-1., M.Value(1,2), 1e-12);
    ASSERT_NEAR( 1., M.Value(1,3), 1e-12);
    ASSERT_NEAR( 3., M.Value(1,4), 1e-12);
    ASSERT_NEAR( 5., M.Value(1,5), 1e-12);
    
    ASSERT_NEAR( 5., M.Value(2,0), 1e-12);
    ASSERT_NEAR(-3., M.Value(2,1), 1e-12);
    ASSERT_NEAR(-7., M.Value(2,2), 1e-12);
    ASSERT_NEAR(-7., M.Value(2,3), 1e-12);
    ASSERT_NEAR(-3., M.Value(2,4), 1e-12);
    ASSERT_NEAR( 5., M.Value(2,5), 1e-12);
    
    ASSERT_NEAR(-5., M.Value(3,0), 1e-12);
    ASSERT_NEAR(13., M.Value(3,1), 1e-12);
    ASSERT_NEAR( 7., M.Value(3,2), 1e-12);
    ASSERT_NEAR(-7., M.Value(3,3), 1e-12);
    ASSERT_NEAR(-13., M.Value(3,4), 1e-12);
    ASSERT_NEAR( 5., M.Value(3,5), 1e-12);
    
    ASSERT_NEAR(  5., M.Value(4,0), 1e-12);
    ASSERT_NEAR(-27., M.Value(4,1), 1e-12);
    ASSERT_NEAR( 21., M.Value(4,2), 1e-12);
    ASSERT_NEAR( 21., M.Value(4,3), 1e-12);
    ASSERT_NEAR(-27., M.Value(4,4), 1e-12);
    ASSERT_NEAR(  5., M.Value(4,5), 1e-12);
    
    ASSERT_NEAR(  -5., M.Value(5,0), 1e-12);
    ASSERT_NEAR(  45., M.Value(5,1), 1e-12);
    ASSERT_NEAR(-105., M.Value(5,2), 1e-12);
    ASSERT_NEAR( 105., M.Value(5,3), 1e-12);
    ASSERT_NEAR( -45., M.Value(5,4), 1e-12);
    ASSERT_NEAR(   5., M.Value(5,5), 1e-12);
}

TEST(TiglMath, DistancePointFromLine)
{
    tigl::CTiglPoint x0(0., 0., 0.);
    tigl::CTiglPoint dx(1., 1., 0.);
    
    ASSERT_NEAR(0., tigl::distance_point_from_line(tigl::CTiglPoint(0., 0., 0.), x0, dx), 1e-10);
    ASSERT_NEAR(0., tigl::distance_point_from_line(tigl::CTiglPoint(3., 3., 0.), x0, dx), 1e-10);
    ASSERT_NEAR(sqrt(0.5), tigl::distance_point_from_line(tigl::CTiglPoint(1., 0., 0.), x0, dx), 1e-10);
}

TEST(TiglMath, CTiglTransformation_Multiply)
{
    tigl::CTiglTransformation a;
    tigl::CTiglTransformation b;
    
    a.AddTranslation(2., 0., 0);
    
    b.AddScaling(0.4, 0.4, 0.4);
    b.AddTranslation(-0.1, 0.9, -0.3);
    
    tigl::CTiglTransformation c = a * b;
    EXPECT_NEAR(0.4, c.GetValue(0, 0), 1e-10);
    EXPECT_NEAR(0.0, c.GetValue(0, 1), 1e-10);
    EXPECT_NEAR(0.0, c.GetValue(0, 2), 1e-10);
    EXPECT_NEAR(1.9, c.GetValue(0, 3), 1e-10);
    
    EXPECT_NEAR(0.0, c.GetValue(1, 0), 1e-10);
    EXPECT_NEAR(0.4, c.GetValue(1, 1), 1e-10);
    EXPECT_NEAR(0.0, c.GetValue(1, 2), 1e-10);
    EXPECT_NEAR(0.9, c.GetValue(1, 3), 1e-10);
    
    EXPECT_NEAR(0.0, c.GetValue(2, 0), 1e-10);
    EXPECT_NEAR(0.0, c.GetValue(2, 1), 1e-10);
    EXPECT_NEAR(0.4, c.GetValue(2, 2), 1e-10);
    EXPECT_NEAR(-0.3, c.GetValue(2, 3), 1e-10);
    
    EXPECT_NEAR(0.0, c.GetValue(3, 0), 1e-10);
    EXPECT_NEAR(0.0, c.GetValue(3, 1), 1e-10);
    EXPECT_NEAR(0.0, c.GetValue(3, 2), 1e-10);
    EXPECT_NEAR(1.0, c.GetValue(3, 3), 1e-10);
}

TEST(TiglMath, CTiglTransform_Decompose)
{
    // trivial test
    tigl::CTiglTransformation transformation;

    tigl::CTiglPoint S,R,T;

    transformation.Decompose(S, R, T);

    EXPECT_NEAR(S.x, 1., 1e-8);
    EXPECT_NEAR(S.y, 1., 1e-8);
    EXPECT_NEAR(S.z, 1., 1e-8);
    EXPECT_NEAR(R.x, 0., 1e-8);
    EXPECT_NEAR(R.y, 0., 1e-8);
    EXPECT_NEAR(R.z, 0., 1e-8);
    EXPECT_NEAR(T.x, 0., 1e-8);
    EXPECT_NEAR(T.y, 0., 1e-8);
    EXPECT_NEAR(T.z, 0., 1e-8);

    // Example of matrices that can not be decompose:

    // Create a shear matrix by rotation and scaling
    // This case can happens if the is a none-uniform scaling in the section

    double shearY = 0.5; // 1.28 - (1/1.28) = 0.5
    double a = 1.28;
    double angle = 38;

    tigl::CTiglTransformation tE,tS, shear, expectedShear;
    tE.SetIdentity();
    tE.AddRotationZ(-angle);
    tS.SetIdentity();
    tS.AddScaling(a,1/a, 1);
    tS.AddRotationZ(90-angle);
    shear = tS *tE;

    expectedShear.SetIdentity();
    expectedShear.SetValue(1,0,0.5);

    for(int row = 0; row < 4; row ++ ){
        for ( int col = 0; col < 4; col++) {
            EXPECT_NEAR(expectedShear.GetValue(row,col), shear.GetValue(row,col), 0.01); // approximation error of .128 and -38
        }
    }

    // it's impossible to decompose a shear matrix properly in R,S,T
    expectedShear.Decompose(S,R,T);

    // todo add a return value to decompose to check if the decomposition can be performed
    EXPECT_TRUE(true);


}

TEST(TiglMath, CTiglTransform_Decompose2)
{
    // Simulate the case where a cpacs transformation as a rotation RX:0;RY:30;RZ:20
    // Remember that cpacs transformation has intrinsic rotation X,Y',Z'' so it corresponding to extrinsic rotation Z,Y,X
    // This above process is similar at the one used at CCPACSTransformation::updateMatrix
    tigl::CTiglTransformation rot;
    rot.AddRotationZ(20);
    rot.AddRotationY(30);
    rot.AddRotationX(0);

    // So now, as we can expected rotating the x basis vector (1,0,0) will output (0.81379768134, 0.34202014332 , -0.46984631039);
    gp_Pnt resultV = rot.Transform(gp_Pnt(1., 0. ,0.));
    gp_Pnt expectV = gp_Pnt(0.81379768134, 0.34202014332 , -0.46984631039);
    EXPECT_NEAR(resultV.X(), expectV.X(), 1e-8 );
    EXPECT_NEAR(resultV.Y(), expectV.Y(), 1e-8 );
    EXPECT_NEAR(resultV.Z(), expectV.Z(), 1e-8 );

    // decomposing the rotation should output the X,Y,Z extrinsic angle
    tigl::CTiglPoint S,R,T;
    rot.Decompose(S, R, T);

    // so if we put back this value in transformation
    tigl::CTiglTransformation rot2;
    rot2.AddRotationZ(R.z);
    rot2.AddRotationY(R.y);
    rot2.AddRotationX(R.x);

    // we get the expected result
    resultV = rot2.Transform(gp_Pnt(1., 0., 0.));
    EXPECT_NEAR(resultV.X(), expectV.X(), 1e-8 );
    EXPECT_NEAR(resultV.Y(), expectV.Y(), 1e-8 );
    EXPECT_NEAR(resultV.Z(), expectV.Z(), 1e-8 );
}

TEST(TiglMath, CTiglTransform_Decompose3)
{

    tigl::CTiglPoint S,R,T;

    tigl::CTiglTransformation rot, rotP;
    rot.AddRotationZ(142);
    rot.AddRotationY(0);
    rot.AddRotationX(-180);

    rot.Decompose(S, R, T);

    rotP.AddRotationZ(R.z);
    rotP.AddRotationY(R.y);
    rotP.AddRotationX(R.x);

    EXPECT_TRUE(rot.IsNear(rotP));


    rot.SetIdentity();
    rot.AddScaling(1.28,0.78,1);
    rot.AddRotationZ(-128);
    rot.AddRotationY(0);
    rot.AddRotationX(-180);

    rot.Decompose(S, R, T);
    rotP.SetIdentity();
    rotP.AddScaling(S.x, S.y,S.z);
    rotP.AddRotationZ(R.z);
    rotP.AddRotationY(R.y);
    rotP.AddRotationX(R.x);

    EXPECT_TRUE(rot.IsNear(rotP));
}

TEST(TiglMath, CTiglTransform_DecomposeTRSRS)
{

    tigl::CTiglTransformation initial, res;

    // Shear matrix test
    initial.SetIdentity();
    initial.SetValue(1, 0, 0.5);

    tigl::CTiglPoint trans, rot2, diag2, rot1, diag1;


    initial.DecomposeTRSRS(diag1, rot1, diag2, rot2, trans);

    res.SetIdentity();
    res.AddScaling(diag1.x, diag1.y, diag1.z);
    res.AddRotationIntrinsicXYZ(rot1.x, rot1.y, rot1.z);
    res.AddScaling(diag2.x, diag2.y, diag2.z);
    res.AddRotationIntrinsicXYZ(rot2.x, rot2.y, rot2.z);
    res.AddTranslation(trans.x, trans.y, trans.z);

    EXPECT_TRUE(initial.IsNear(res));

    // Identity

    initial.SetIdentity();

    initial.DecomposeTRSRS(diag1, rot1, diag2, rot2, trans);

    res.SetIdentity();
    res.AddScaling(diag1.x, diag1.y, diag1.z);
    res.AddRotationIntrinsicXYZ(rot1.x, rot1.y, rot1.z);
    res.AddScaling(diag2.x, diag2.y, diag2.z);
    res.AddRotationIntrinsicXYZ(rot2.x, rot2.y, rot2.z);
    res.AddTranslation(trans.x, trans.y, trans.z);

    EXPECT_TRUE(initial.IsNear(res));

    // Pseudo random

    initial.SetIdentity();
    initial.AddScaling(3.4, 23, 0.9);
    initial.AddRotationIntrinsicXYZ(3.4, 23, 0.9);
    initial.AddTranslation(3.3, -2, 4);

    initial.AddScaling(23, 0.5, 0.9);
    initial.AddRotationIntrinsicXYZ(23, 55, 77);
    initial.AddTranslation(12, -90, 12);

    initial.AddScaling(23, 0.5, 0.9);
    initial.AddRotationIntrinsicXYZ(-3, -9, -7);
    initial.AddTranslation(7, -9, 12);

    initial.DecomposeTRSRS(diag1, rot1, diag2, rot2, trans);

    res.SetIdentity();
    res.AddScaling(diag1.x, diag1.y, diag1.z);
    res.AddRotationIntrinsicXYZ(rot1.x, rot1.y, rot1.z);
    res.AddScaling(diag2.x, diag2.y, diag2.z);
    res.AddRotationIntrinsicXYZ(rot2.x, rot2.y, rot2.z);
    res.AddTranslation(trans.x, trans.y, trans.z);

    EXPECT_TRUE(initial.IsNear(res));

    // Pseudo random case

    initial.SetIdentity();
    initial.AddScaling(3.21, 2, 9);
    initial.AddRotationIntrinsicXYZ(123.4, 223, 321);
    initial.AddTranslation(31, -1, 1);

    initial.AddScaling(23, 0.5, 0.9);
    initial.AddRotationIntrinsicXYZ(23, 55, 77);

    initial.AddScaling(11, 30.5, 18);
    initial.AddRotationIntrinsicXYZ(23, 55, 77);
    initial.AddTranslation(12, -90, 12);
    initial.AddTranslation(13, -3, 12);

    initial.AddScaling(-12, -12, -32);
    initial.AddRotationIntrinsicXYZ(-321, -922, -722);
    initial.AddTranslation(7, -9, 2);

    initial.DecomposeTRSRS(diag1, rot1, diag2, rot2, trans);

    res.SetIdentity();
    res.AddScaling(diag1.x, diag1.y, diag1.z);
    res.AddRotationIntrinsicXYZ(rot1.x, rot1.y, rot1.z);
    res.AddScaling(diag2.x, diag2.y, diag2.z);
    res.AddRotationIntrinsicXYZ(rot2.x, rot2.y, rot2.z);
    res.AddTranslation(trans.x, trans.y, trans.z);

    EXPECT_TRUE(initial.IsNear(res));

    // 0 scaling case

    initial.SetIdentity();
    initial.AddScaling(0, 2, 9);

    initial.DecomposeTRSRS(diag1, rot1, diag2, rot2, trans);

    res.SetIdentity();
    res.AddScaling(diag1.x, diag1.y, diag1.z);
    res.AddRotationIntrinsicXYZ(rot1.x, rot1.y, rot1.z);
    res.AddScaling(diag2.x, diag2.y, diag2.z);
    res.AddRotationIntrinsicXYZ(rot2.x, rot2.y, rot2.z);
    res.AddTranslation(trans.x, trans.y, trans.z);

    EXPECT_TRUE(initial.IsNear(res));

    //    Fail due the scaling
    //
    //    initial.SetIdentity();
    //    initial.AddScaling(0,2,9);
    //    initial.AddRotationIntrinsicXYZ(30,50,70);
    //
    //    initial.DecomposeTRSRS(diag1, rot1, diag2, rot2, trans);
    //
    //    res.SetIdentity();
    //    res.AddScaling(diag1.x,diag1.y,diag1.z);
    //    res.AddRotationIntrinsicXYZ(rot1.x,rot1.y,rot1.z);
    //    res.AddScaling(diag2.x,diag2.y,diag2.z);
    //    res.AddRotationIntrinsicXYZ(rot2.x,rot2.y,rot2.z);
    //    res.AddTranslation(trans.x,trans.y,trans.z);
    //
    //
    //    EXPECT_TRUE( initial.IsNear(res));
}

TEST(TiglMath, CTiglTransform_setTransformationMatrix)
{
    double scale[3] = {2., 4., 8.};
    double rot[3]   = {50., 70., 10.};
    double trans[3] = {1., 2., 3.};

    // create CPACS-conform tigl-transformation (i.e. scaling -> euler-xyz-Rotation -> translation)
    tigl::CTiglTransformation tiglTrafo;
    tiglTrafo.AddScaling(scale[0], scale[1], scale[2]);
    tiglTrafo.AddRotationIntrinsicXYZ(rot[0], rot[1], rot[2]);
    tiglTrafo.AddTranslation(trans[0], trans[1], trans[2]);

    tigl::CCPACSTransformation cpacsTrafo(NULL);
    cpacsTrafo.setTransformationMatrix(tiglTrafo);

    EXPECT_NEAR(*cpacsTrafo.GetScaling()->GetX(), scale[0], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetScaling()->GetY(), scale[1], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetScaling()->GetZ(), scale[2], 1e-8);

    EXPECT_NEAR(*cpacsTrafo.GetRotation()->GetX(), rot[0], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetRotation()->GetY(), rot[1], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetRotation()->GetZ(), rot[2], 1e-8);

    EXPECT_NEAR(*cpacsTrafo.GetTranslation()->GetX(), trans[0], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetTranslation()->GetY(), trans[1], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetTranslation()->GetZ(), trans[2], 1e-8);
}


TEST(TiglMath, CTiglTransform_getRotationToAlignAToB)
{

    tigl::CTiglTransformation yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint(0, 1,0),tigl::CTiglPoint(1, 0,0));

    tigl::CTiglPoint scale,rot,trans;
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot.x, 0, 0.01);
    EXPECT_NEAR(rot.y, 0, 0.01);
    EXPECT_NEAR(rot.z, -90, 0.01);

    EXPECT_NEAR(scale.x, 1, 0.01);
    EXPECT_NEAR(scale.x, 1, 0.01);
    EXPECT_NEAR(scale.z, 1, 0.01);

    EXPECT_NEAR(trans.x, 0, 0.01);
    EXPECT_NEAR(trans.y, 0, 0.01);
    EXPECT_NEAR(trans.z, 0, 0.01);


    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint(0.5,0.5,0), tigl::CTiglPoint(1, 0 ,0));
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot.x, 0, 0.01);
    EXPECT_NEAR(rot.y, 0, 0.01);
    EXPECT_NEAR(rot.z, -45, 0.01);

    EXPECT_NEAR(scale.x, 1, 0.01);
    EXPECT_NEAR(scale.y, 1, 0.01);
    EXPECT_NEAR(scale.z, 1, 0.01);

    EXPECT_NEAR(trans.x, 0, 0.01);
    EXPECT_NEAR(trans.y, 0, 0.01);
    EXPECT_NEAR(trans.z, 0, 0.01);


    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint(1, 0 ,0), tigl::CTiglPoint(0.5,0.5,0));
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot.x, 0, 0.01);
    EXPECT_NEAR(rot.y, 0, 0.01);
    EXPECT_NEAR(rot.z, 45, 0.01);

    EXPECT_NEAR(scale.x, 1, 0.01);
    EXPECT_NEAR(scale.y, 1, 0.01);
    EXPECT_NEAR(scale.z, 1, 0.01);

    EXPECT_NEAR(trans.x, 0, 0.01);
    EXPECT_NEAR(trans.y, 0, 0.01);
    EXPECT_NEAR(trans.z, 0, 0.01);

    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint(-0.5,-0.5,0),  tigl::CTiglPoint(1, 0 ,0) );
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot.x, 0, 0.01);
    EXPECT_NEAR(rot.y, 0, 0.01);
    EXPECT_NEAR(rot.z, 135, 0.01);

    EXPECT_NEAR(scale.x, 1, 0.01);
    EXPECT_NEAR(scale.y, 1, 0.01);
    EXPECT_NEAR(scale.z, 1, 0.01);

    EXPECT_NEAR(trans.x, 0, 0.01);
    EXPECT_NEAR(trans.y, 0, 0.01);
    EXPECT_NEAR(trans.z, 0, 0.01);


    tigl::CTiglPoint b =  tigl::CTiglPoint(0.81379768134, 0.34202014332 , -0.46984631039); // Ry:30, rz:20
    tigl::CTiglPoint a = tigl::CTiglPoint(1,0,0);
    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(a, b);

    tigl::CTiglPoint result = yToZ * a;
    EXPECT_NEAR(result.x, b.x, 0.01 );
    EXPECT_NEAR(result.y, b.y, 0.01 );
    EXPECT_NEAR(result.z, b.z, 0.01 );


    // Special case where the vector are identical
    b =  tigl::CTiglPoint(1,0,0); // Ry:30, rz:20
    a = tigl::CTiglPoint(1,0,0);
    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(a, b);

    result = yToZ * a;
    EXPECT_NEAR(result.x, b.x, 0.01 );
    EXPECT_NEAR(result.y, b.y, 0.01 );
    EXPECT_NEAR(result.z, b.z, 0.01 );


    b =  tigl::CTiglPoint(-1,0,0); // Ry:30, rz:20
    a = tigl::CTiglPoint(1,0,0);
    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(a, b);

    result = yToZ * a;
    EXPECT_NEAR(result.x, b.x, 0.01 );
    EXPECT_NEAR(result.y, b.y, 0.01 );
    EXPECT_NEAR(result.z, b.z, 0.01 );

}



TEST(TiglMath, CTiglTransform_GetRotationFromAxisRotation)
{

    tigl::CTiglTransformation res;
    tigl::CTiglTransformation expectedR;
    tigl::CTiglPoint axis;
    double angle;

    expectedR.SetValue(0,0,0);
    expectedR.SetValue(0,1,0);
    expectedR.SetValue(0,2,1);
    expectedR.SetValue(1,0,0);
    expectedR.SetValue(1,1,1);
    expectedR.SetValue(1,2,0);
    expectedR.SetValue(2,0,-1);
    expectedR.SetValue(2,1,0);
    expectedR.SetValue(2,2,0);

    axis = tigl::CTiglPoint(0,-1,0);
    angle = -90;
    res = tigl::CTiglTransformation::GetRotationFromAxisRotation(axis, angle);
    EXPECT_TRUE(res.IsNear(expectedR));


    expectedR.SetValue(0,0, 0.9698463);
    expectedR.SetValue(0,1,-0.0301537);
    expectedR.SetValue(0,2,-0.2418448);

    expectedR.SetValue(1,0,-0.0301537);
    expectedR.SetValue(1,1,0.9698463);
    expectedR.SetValue(1,2,-0.2418448);

    expectedR.SetValue(2,0, 0.2418448);
    expectedR.SetValue(2,1, 0.2418448);
    expectedR.SetValue(2,2,0.9396926 );

    axis = tigl::CTiglPoint(1,-1,0);
    angle = 20;
    res = tigl::CTiglTransformation::GetRotationFromAxisRotation(axis, angle);
    EXPECT_TRUE(res.IsNear(expectedR));

}



TEST(TiglMath, CTiglTransform_isNear)
{
    tigl::CTiglTransformation a,b;

    EXPECT_TRUE(a.IsNear(b));
    EXPECT_TRUE(b.IsNear(a));
    EXPECT_TRUE(a.IsNear(b,0));
    EXPECT_TRUE(b.IsNear(a,0));

    a.AddTranslation(12,13,14);
    EXPECT_FALSE(a.IsNear(b));
    EXPECT_FALSE(b.IsNear(a));
    EXPECT_TRUE(a.IsNear(b,100));

    b.AddTranslation(12,13,14);
    EXPECT_TRUE(a.IsNear(b));
    EXPECT_TRUE(b.IsNear(a));

    b.AddRotationX(0.1);
    EXPECT_FALSE(a.IsNear(b));
    a.AddRotationX(0.1);
    EXPECT_TRUE(a.IsNear(b));

    a.AddScaling(3,3.3,4);
    EXPECT_FALSE(a.IsNear(b));
    b.AddScaling(3,3.3,4);
    EXPECT_TRUE(a.IsNear(b));

}



TEST(TiglMath, SVD)
{
    tigl::tiglMatrix A(1, 3, 1, 3);
    tigl::tiglMatrix U(1, 3, 1, 3);
    tigl::tiglMatrix S(1, 3, 1, 3);
    tigl::tiglMatrix V(1, 3, 1, 3);
    tigl::tiglMatrix USVt(1, 3, 1, 3);

    // trivial test
    A(1,1)=1.; A(1,2) = 0.; A(1,3) = 0.;
    A(2,1)=0.; A(2,2) = 1.; A(2,3) = 0.;
    A(3,1)=0.; A(3,2) = 0.; A(3,3) = 1.;

    tigl::SVD(A, U, S, V);
    USVt = U*S*V.Transposed();
    EXPECT_NEAR(USVt(1,1), A(1,1), 1e-8);
    EXPECT_NEAR(USVt(1,2), A(1,2), 1e-8);
    EXPECT_NEAR(USVt(1,3), A(1,3), 1e-8);
    EXPECT_NEAR(USVt(2,1), A(2,1), 1e-8);
    EXPECT_NEAR(USVt(2,2), A(2,2), 1e-8);
    EXPECT_NEAR(USVt(2,3), A(2,3), 1e-8);
    EXPECT_NEAR(USVt(3,1), A(3,1), 1e-8);
    EXPECT_NEAR(USVt(3,2), A(3,2), 1e-8);
    EXPECT_NEAR(USVt(3,3), A(3,3), 1e-8);

    // nontrivial test
    A(1,1)=1.; A(1,2) = 2.; A(1,3) = 3.;
    A(2,1)=4.; A(2,2) = 5.; A(2,3) = 6.;
    A(3,1)=7.; A(3,2) = 8.; A(3,3) = 9.;

    tigl::SVD(A, U, S, V);
    USVt = U*S*V.Transposed();
    EXPECT_NEAR(USVt(1,1), A(1,1), 1e-8);
    EXPECT_NEAR(USVt(1,2), A(1,2), 1e-8);
    EXPECT_NEAR(USVt(1,3), A(1,3), 1e-8);
    EXPECT_NEAR(USVt(2,1), A(2,1), 1e-8);
    EXPECT_NEAR(USVt(2,2), A(2,2), 1e-8);
    EXPECT_NEAR(USVt(2,3), A(2,3), 1e-8);
    EXPECT_NEAR(USVt(3,1), A(3,1), 1e-8);
    EXPECT_NEAR(USVt(3,2), A(3,2), 1e-8);
    EXPECT_NEAR(USVt(3,3), A(3,3), 1e-8);


}

TEST(TiglMath, DiagonalizeMatrixByJacobi)
{

    tigl::tiglMatrix I(1, 3, 1, 3);
    I(1, 1) = 1.;
    I(1, 2) = 0.;
    I(1, 3) = 0.;
    I(2, 1) = 0.;
    I(2, 2) = 1.;
    I(2, 3) = 0.;
    I(3, 1) = 0.;
    I(3, 2) = 0.;
    I(3, 3) = 1.;

    tigl::tiglMatrix M(1, 3, 1, 3);
    tigl::tiglMatrix D(1, 3, 1, 3);
    tigl::tiglMatrix expectedD(1, 3, 1, 3);
    tigl::tiglMatrix V(1, 3, 1, 3);
    tigl::tiglMatrix check(1, 3, 1, 3);

    double tolerance = 0.001;

    // trivial test
    // todo: The V vector is not the identity, why?
    M = I;
    tigl::DiagonalizeMatrixByJacobi(M, D, V);

    expectedD = I;
    check     = V * D * V.Transposed();

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            EXPECT_NEAR(D(r, c), expectedD(r, c), tolerance);
            EXPECT_NEAR(check(r, c), M(r, c), tolerance);
        }
    }

    // none trivial test 1

    M(1, 1) = 1.;
    M(1, 2) = 2.;
    M(1, 3) = 3.;
    M(2, 1) = 2.;
    M(2, 2) = 1.;
    M(2, 3) = 2.;
    M(3, 1) = 3.;
    M(3, 2) = 2.;
    M(3, 3) = 1.;

    tigl::DiagonalizeMatrixByJacobi(M, D, V);

    expectedD(1, 1) = 5.702;
    expectedD(1, 2) = 0.;
    expectedD(1, 3) = 0.;
    expectedD(2, 1) = 0.;
    expectedD(2, 2) = -0.702;
    expectedD(2, 3) = 0.;
    expectedD(3, 1) = 0.;
    expectedD(3, 2) = 0.;
    expectedD(3, 3) = -2.;

    check = V * D * V.Transposed();

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            EXPECT_NEAR(D(r, c), expectedD(r, c), tolerance);
            EXPECT_NEAR(check(r, c), M(r, c), tolerance);
        }
    }

    // none trivial test 2

    M(1, 1) = 4.3;
    M(1, 2) = 1.;
    M(1, 3) = 3.3;
    M(2, 1) = 1.;
    M(2, 2) = -8.;
    M(2, 3) = 9.;
    M(3, 1) = 3.3;
    M(3, 2) = 9.;
    M(3, 3) = 3.;

    tigl::DiagonalizeMatrixByJacobi(M, D, V);

    expectedD(1, 1) = 10.034;
    expectedD(1, 2) = 0.;
    expectedD(1, 3) = 0.;
    expectedD(2, 1) = 0.;
    expectedD(2, 2) = 2.347;
    expectedD(2, 3) = 0.;
    expectedD(3, 1) = 0.;
    expectedD(3, 2) = 0.;
    expectedD(3, 3) = -13.08;

    check = V * D * V.Transposed();

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            EXPECT_NEAR(D(r, c), expectedD(r, c), tolerance);
            EXPECT_NEAR(check(r, c), M(r, c), tolerance);
        }
    }
}

TEST(TiglMath, RotMatrixToIntrinsicXYZVector)
{

    tigl::tiglMatrix rM(1, 3, 1, 3);
    tigl::CTiglPoint rV(1, 3);
    tigl::CTiglPoint inputRV(1, 3);
    tigl::CTiglTransformation rT, rEquivalent;

    double tolerance = 0.001;

    // trivial test

    inputRV.x = 0;
    inputRV.y = 0;
    inputRV.z = 0;
    rT.SetIdentity();
    rT.AddRotationIntrinsicXYZ(inputRV.x, inputRV.y, inputRV.z);

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            rM(r, c) = rT.GetValue(r - 1, c - 1);
        }
    }

    rV = tigl::RotMatrixToIntrinsicXYZVector(rM);


    EXPECT_NEAR(rV.x, inputRV.x, tolerance);
    EXPECT_NEAR(rV.y, inputRV.y, tolerance);
    EXPECT_NEAR(rV.z, inputRV.z, tolerance);


    //  normal case test

    inputRV.x = 20;
    inputRV.y = 40;
    inputRV.z = 30;
    rT.SetIdentity();
    rT.AddRotationIntrinsicXYZ(inputRV.x, inputRV.y, inputRV.z);

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            rM(r, c) = rT.GetValue(r - 1, c - 1);
        }
    }

    rV = tigl::RotMatrixToIntrinsicXYZVector(rM);

    EXPECT_NEAR(rV.x, inputRV.x, tolerance);
    EXPECT_NEAR(rV.y, inputRV.y, tolerance);
    EXPECT_NEAR(rV.z, inputRV.z, tolerance);



    // negative rotation case

    inputRV.x = -20;
    inputRV.y = -40;
    inputRV.z = 30;
    rT.SetIdentity();
    rT.AddRotationIntrinsicXYZ(inputRV.x, inputRV.y, inputRV.z);

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            rM(r, c) = rT.GetValue(r - 1, c - 1);
        }
    }

    rV = tigl::RotMatrixToIntrinsicXYZVector(rM);

    EXPECT_NEAR(rV.x, inputRV.x, tolerance);
    EXPECT_NEAR(rV.y, inputRV.y, tolerance);
    EXPECT_NEAR(rV.z, inputRV.z, tolerance);


    // case when RY = 90
    inputRV.x = 0;
    inputRV.y = 90;
    inputRV.z = 30;
    rT.SetIdentity();
    rT.AddRotationIntrinsicXYZ(inputRV.x, inputRV.y, inputRV.z);

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            rM(r, c) = rT.GetValue(r - 1, c - 1);
        }
    }

    rV = tigl::RotMatrixToIntrinsicXYZVector(rM);

    EXPECT_NEAR(rV.x, inputRV.x, tolerance);
    EXPECT_NEAR(rV.y, inputRV.y, tolerance);
    EXPECT_NEAR(rV.z, inputRV.z, tolerance);

    // case when RY = -90
    inputRV.x = 0;
    inputRV.y = -90;
    inputRV.z = 30;
    rT.SetIdentity();
    rT.AddRotationIntrinsicXYZ(inputRV.x, inputRV.y, inputRV.z);

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            rM(r, c) = rT.GetValue(r - 1, c - 1);
        }
    }

    rV = tigl::RotMatrixToIntrinsicXYZVector(rM);

    EXPECT_NEAR(rV.x, inputRV.x, tolerance);
    EXPECT_NEAR(rV.y, inputRV.y, tolerance);
    EXPECT_NEAR(rV.z, inputRV.z, tolerance);


    // case when RY = -90 with RX -> deadlock ->transformation of equivalent input
    inputRV.x = 10;
    inputRV.y = -90;
    inputRV.z = 30;
    rT.SetIdentity();
    rT.AddRotationIntrinsicXYZ(inputRV.x, inputRV.y, inputRV.z);

    for (int r = 1; r < 4; r++) {
        for (int c = 1; c < 4; c++) {
            rM(r, c) = rT.GetValue(r - 1, c - 1);
        }
    }

    rV = tigl::RotMatrixToIntrinsicXYZVector(rM);

    rEquivalent.SetIdentity();
    rEquivalent.AddRotationIntrinsicXYZ(rV.x, rV.y, rV.z);
    EXPECT_TRUE(rEquivalent.IsNear(rT, 0.001));

}


TEST(TiglMath, FindVectorPerpendicularToDirection)
{
    tigl::CTiglPoint i , res;
    double check  ;

    i = tigl::CTiglPoint(0,-1,0);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(0, 0,-1);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(-1,0,0);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(1,0,0);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(0,1,0);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(0,0,1);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(12,3,-45);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(0.2,1,0.2);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


    i = tigl::CTiglPoint(45,45,45);
    res = tigl::FindOrthogonalVectorToDirection(i);
    check = tigl::CTiglPoint::inner_prod(res, i);
    EXPECT_NEAR(check , 0, 0.00001);


}

TEST(TiglMath, Rounding)
{
    double number;
    double roundingValue;
    double delta;

    number        = 0.001;
    roundingValue = 0;
    delta         = 0.1;
    tigl::Rounding(number, roundingValue, delta);
    EXPECT_EQ(number, roundingValue);

    number        = 0.1;
    roundingValue = 0;
    delta         = 0.01;
    tigl::Rounding(number, roundingValue, delta);
    EXPECT_EQ(number, 0.1);

    number        = 1e-9;
    roundingValue = 0;
    tigl::Rounding(number, roundingValue); // Precision::Confusion() is used by default for delta
    EXPECT_EQ(number, 0);

    number        = 1e-6;
    roundingValue = 0;
    tigl::Rounding(number, roundingValue);
    EXPECT_EQ(number, 1e-6);

    number        = 1.01;
    roundingValue = 1;
    delta         = 0.1;
    tigl::Rounding(number, roundingValue, delta);
    EXPECT_EQ(number, 1);

    number        = -1.01;
    roundingValue = -1;
    delta         = 0.1;
    tigl::Rounding(number, roundingValue, delta);
    EXPECT_EQ(number, -1);

    // rotation rounding
    tigl::CTiglPoint rotation;

    rotation = tigl::CTiglPoint(1e-9, 1e-9, 1e-9);
    tigl::RotationRounding(rotation);
    EXPECT_EQ(rotation.x, 0);
    EXPECT_EQ(rotation.y, 0);
    EXPECT_EQ(rotation.z, 0);

    rotation = tigl::CTiglPoint(90 - 1e-9, 90 - 1e-9, 90 + 1e-9);
    tigl::RotationRounding(rotation);
    EXPECT_EQ(rotation.x, 90);
    EXPECT_EQ(rotation.y, 90);
    EXPECT_EQ(rotation.z, 90);

    rotation = tigl::CTiglPoint(360 + 1e-9, 360 - 1e-9, 360 + 1e-9);
    tigl::RotationRounding(rotation);
    EXPECT_EQ(rotation.x, 0);
    EXPECT_EQ(rotation.y, 0);
    EXPECT_EQ(rotation.z, 0);

    // scaling rounding;
    tigl::CTiglPoint scaling;

    scaling = tigl::CTiglPoint(1.1, 1.1, 1.1);
    tigl::ScalingRounding(scaling, 0.2);
    EXPECT_EQ(scaling.x, 1);
    EXPECT_EQ(scaling.y, 1);
    EXPECT_EQ(scaling.z, 1);

    scaling = tigl::CTiglPoint(-1 + 1e-9, 1.1, -1 - +1e-9);
    tigl::ScalingRounding(scaling);
    EXPECT_EQ(scaling.x, -1);
    EXPECT_EQ(scaling.y, 1.1);
    EXPECT_EQ(scaling.z, -1);

    // translation rounding
    tigl::CTiglPoint translation;

    translation = tigl::CTiglPoint(-1e-9, 1e-9, 0.2);
    tigl::ScalingRounding(translation);
    EXPECT_EQ(translation.x, 0);
    EXPECT_EQ(translation.y, 0);
    EXPECT_EQ(translation.z, 0.2);
}
