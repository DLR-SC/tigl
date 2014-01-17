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
#include<vector>

#include "CTiglPoint.h"

TEST(TiglMath, factorial){
    ASSERT_EQ(1, tigl::factorial(0));
    ASSERT_EQ(1, tigl::factorial(1));
    ASSERT_EQ(2, tigl::factorial(2));
    ASSERT_EQ(6, tigl::factorial(3));
    ASSERT_EQ(24, tigl::factorial(4));
}

TEST(TiglMath, binom) {
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

TEST(TiglMath, BernsteinPoly) {
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

TEST(TiglMath, QuadrilateralArea) {
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
    ASSERT_NEAR(0.0, tigl::cstcurve(N1, N2, Br, 0.0), 1e-7);
    ASSERT_NEAR(0.568964089203402, tigl::cstcurve(N1, N2, Br, 0.2), 1e-7);
    ASSERT_NEAR(2.325867218509732, tigl::cstcurve(N1, N2, Br, 0.75), 1e-7);
    ASSERT_NEAR(0.0, tigl::cstcurve(N1, N2, Br, 1.0), 1e-7);

    // Sample exponents >1 for derivative 
    N1=1.1;
    N2=4.5;
    // check 1st derivative of cst curve at some points
    ASSERT_NEAR(0.0, tigl::cstcurve_deriv(N1, N2, Br, 1, 0.0), 1e-7);
    ASSERT_NEAR(0.0, tigl::cstcurve_deriv(N1, N2, Br, 1, 1.0), 1e-7);
    // check 1st derivative of cst curve at maximum of cstcurve (found numerically)
    ASSERT_NEAR(0.0, tigl::cstcurve_deriv(N1, N2, Br, 1, 0.322954559162619), 1e-7);
}
