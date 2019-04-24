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

    double S[3] = {0., 0., 0.};
    double R[3] = {0., 0., 0.};
    double T[3] = {0., 0., 0.};
    transformation.Decompose(S, R, T);

    EXPECT_NEAR(S[0], 1., 1e-8);
    EXPECT_NEAR(S[0], 1., 1e-8);
    EXPECT_NEAR(S[0], 1., 1e-8);
    EXPECT_NEAR(R[0], 0., 1e-8);
    EXPECT_NEAR(R[0], 0., 1e-8);
    EXPECT_NEAR(R[0], 0., 1e-8);
    EXPECT_NEAR(T[0], 0., 1e-8);
    EXPECT_NEAR(T[0], 0., 1e-8);
    EXPECT_NEAR(T[0], 0., 1e-8);


    // todo
/*
    // Simulate the case where a cpacs transformation as a rotation RX:0;RY:30;RZ:20
    // Remember that cpacs transformation has intrinsic rotation X,Y',Z'' so it corresponding to extrinsic rotation Z,Y,X
    // This above process is similar at the one used at CCPACSTransformation::updateMatrix
    tigl::CTiglTransformation rot;
    rot.AddRotationZ(20);
    rot.AddRotationY(30);
    rot.AddRotationX(0);

    // So now, as we can expected rotating the x basis vector (1,0,0) will output (0.81379768134, 0.34202014332 , -0.46984631039);
    tigl::CTiglPoint resultV = rot*tigl::CTiglPoint(1,0,0);
    tigl::CTiglPoint expectV = tigl::CTiglPoint(0.81379768134, 0.34202014332 , -0.46984631039);
    EXPECT_NEAR(resultV.x, expectV.x, 1e-8 );
    EXPECT_NEAR(resultV.y, expectV.y, 1e-8 );
    EXPECT_NEAR(resultV.z, expectV.z, 1e-8 );

    // but decomposing the rotation seems to output the X,Y,Z extrinsic angle
    rot.Decompose(S, R, T);

    // so if we put back this value in transformation
    tigl::CTiglTransformation rot2;
    rot2.AddRotationZ(R[2]);
    rot2.AddRotationY(R[1]);
    rot2.AddRotationX(R[0]);

    // we do not get the expected result
    resultV = rot2 * tigl::CTiglPoint(1,0,0);
    EXPECT_NEAR(resultV.x, expectV.x, 1e-8 );
    EXPECT_NEAR(resultV.y, expectV.y, 1e-8 );
    EXPECT_NEAR(resultV.z, expectV.z, 1e-8 );

    // but the correct result can be created by passing the compute angle in X Y Z extrinsic order
    *//*
       Uncomment these lines of code if you want to verfy the above statement
    tigl::CTiglTransformation rot3;
    rot3.AddRotationX(R[0]);
    rot3.AddRotationY(R[1]);
    rot3.AddRotationZ(R[2]);

    resultV = rot3 * tigl::CTiglPoint(1,0,0);
    EXPECT_NEAR(resultV.x, expectV.x, 1e-8 );
    EXPECT_NEAR(resultV.y, expectV.y, 1e-8 );
    EXPECT_NEAR(resultV.z, expectV.z, 1e-8 );*//*



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
    EXPECT_TRUE(true);*/


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

    // but decomposing the rotation seems to output the X,Y,Z extrinsic angle
    double S[3] = {0., 0., 0.};
    double R[3] = {0., 0., 0.};
    double T[3] = {0., 0., 0.};
    rot.Decompose(S, R, T);

    // so if we put back this value in transformation
    tigl::CTiglTransformation rot2;
    rot2.AddRotationZ(R[2]);
    rot2.AddRotationY(R[1]);
    rot2.AddRotationX(R[0]);

    // we do not get the expected result
    resultV = rot2.Transform(gp_Pnt(1., 0., 0.));
    EXPECT_NEAR(resultV.X(), expectV.X(), 1e-8 );
    EXPECT_NEAR(resultV.Y(), expectV.Y(), 1e-8 );
    EXPECT_NEAR(resultV.Z(), expectV.Z(), 1e-8 );
}

TEST(TiglMath, CTiglTransform_setTransformationMatrix)
{
    double scale[3] = {2., 4., 8.};
    double rot[3]   = {50., 70., 10.};
    double trans[3] = {1., 2., 3.};

    // create CPACS-conform tigl-transformation (i.e. scaling -> euler-xyz-Rotation -> translation)
    tigl::CTiglTransformation tiglTrafo;
    tiglTrafo.AddScaling(scale[0], scale[1], scale[2]);
    tiglTrafo.AddRotationZ(rot[2]);
    tiglTrafo.AddRotationY(rot[1]);
    tiglTrafo.AddRotationX(rot[0]);
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

    double scale[3] = { -1 , -1 , -1 };
    double rot[3]   = { -1 , -1 , -1};
    double trans[3] = { -1 , -1 , -1};
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot[0], 0, 0.01);
    EXPECT_NEAR(rot[1], 0, 0.01);
    EXPECT_NEAR(rot[2], -90, 0.01);

    EXPECT_NEAR(scale[0], 1, 0.01);
    EXPECT_NEAR(scale[1], 1, 0.01);
    EXPECT_NEAR(scale[2], 1, 0.01);

    EXPECT_NEAR(trans[0], 0, 0.01);
    EXPECT_NEAR(trans[1], 0, 0.01);
    EXPECT_NEAR(trans[2], 0, 0.01);


    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint(0.5,0.5,0), tigl::CTiglPoint(1, 0 ,0));
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot[0], 0, 0.01);
    EXPECT_NEAR(rot[1], 0, 0.01);
    EXPECT_NEAR(rot[2], -45, 0.01);

    EXPECT_NEAR(scale[0], 1, 0.01);
    EXPECT_NEAR(scale[1], 1, 0.01);
    EXPECT_NEAR(scale[2], 1, 0.01);

    EXPECT_NEAR(trans[0], 0, 0.01);
    EXPECT_NEAR(trans[1], 0, 0.01);
    EXPECT_NEAR(trans[2], 0, 0.01);


    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint(1, 0 ,0), tigl::CTiglPoint(0.5,0.5,0));
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot[0], 0, 0.01);
    EXPECT_NEAR(rot[1], 0, 0.01);
    EXPECT_NEAR(rot[2], 45, 0.01);

    EXPECT_NEAR(scale[0], 1, 0.01);
    EXPECT_NEAR(scale[1], 1, 0.01);
    EXPECT_NEAR(scale[2], 1, 0.01);

    EXPECT_NEAR(trans[0], 0, 0.01);
    EXPECT_NEAR(trans[1], 0, 0.01);
    EXPECT_NEAR(trans[2], 0, 0.01);

    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint(-0.5,-0.5,0),  tigl::CTiglPoint(1, 0 ,0) );
    yToZ.Decompose(scale, rot, trans);

    EXPECT_NEAR(rot[0], 0, 0.01);
    EXPECT_NEAR(rot[1], 0, 0.01);
    EXPECT_NEAR(rot[2], 135, 0.01);

    EXPECT_NEAR(scale[0], 1, 0.01);
    EXPECT_NEAR(scale[1], 1, 0.01);
    EXPECT_NEAR(scale[2], 1, 0.01);

    EXPECT_NEAR(trans[0], 0, 0.01);
    EXPECT_NEAR(trans[1], 0, 0.01);
    EXPECT_NEAR(trans[2], 0, 0.01);


    tigl::CTiglPoint b = tigl::CTiglPoint( 0.92541657839, 0.34202014332 , -0.16317591116);
    tigl::CTiglPoint a = tigl::CTiglPoint(1,0,0);

    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(a, b);

    // todo wait for a correct decomposition
//    yToZ.Decompose(scale, rot, trans);
//
//    EXPECT_NEAR(rot[0], 0, 0.01);
//    EXPECT_NEAR(rot[1], 10, 0.01);
//    EXPECT_NEAR(rot[2], 20, 0.01);
//
//    EXPECT_NEAR(scale[0], 1, 0.01);
//    EXPECT_NEAR(scale[1], 1, 0.01);
//    EXPECT_NEAR(scale[2], 1, 0.01);
//
//    EXPECT_NEAR(trans[0], 0, 0.01);
//    EXPECT_NEAR(trans[1], 0, 0.01);
//    EXPECT_NEAR(trans[2], 0, 0.01);

    tigl::CTiglPoint result = yToZ * a;
    EXPECT_NEAR(result.x, b.x, 0.01 );
    EXPECT_NEAR(result.y, b.y, 0.01 );
    EXPECT_NEAR(result.z, b.z, 0.01 );



    b =  tigl::CTiglPoint(0.81379768134, 0.34202014332 , -0.46984631039); // Ry:30, rz:20
    a = tigl::CTiglPoint(1,0,0);
    yToZ = tigl::CTiglTransformation::GetRotationToAlignAToB(a, b);

    result = yToZ * a;
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
