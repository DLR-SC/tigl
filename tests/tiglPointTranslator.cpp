/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-13 Martin Siggel <Martin.Siggel@dlr.de>
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
/**
* @file 
* @brief  tests point translator class
*/

#include "test.h"
#include "tigl.h"
#include "math/CTiglPointTranslator.h"
#include <ctime>
#include <cstdio>
#include "Precision.hxx"

using namespace tigl;

static double abs_error = Precision::Confusion();

TEST(TiglPointTranslator, simple){
    CTiglPoint x1(0,2,0);
    CTiglPoint x2(2,2,0);
    CTiglPoint x3(0,0,0);
    CTiglPoint x4(2,0,0);

    CTiglPoint p(1,1,0);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.5, eta, abs_error);
    ASSERT_NEAR( 0.5, xsi, abs_error);

    p.x = 1; p.y = 0; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.5, eta, abs_error);
    ASSERT_NEAR( 1.0, xsi, abs_error);

    p.x = 0; p.y = 0; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.0, eta, abs_error);
    ASSERT_NEAR( 1.0, xsi, abs_error);

    p.x = 1.5; p.y = 1; p.z = 3;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.75, eta, abs_error);
    ASSERT_NEAR( 0.5 , xsi, abs_error);

}

TEST(TiglPointTranslator, simple2){
    CTiglPoint x1(-1,0,0);
    CTiglPoint x2(0,1,0);
    CTiglPoint x3(0,-1,0);
    CTiglPoint x4(1,0,0);

    CTiglPoint p(0,0,0);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.5, eta, abs_error);
    ASSERT_NEAR( 0.5, xsi, abs_error);
    
    p.x = -1.; p.y = 0.; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0., eta, abs_error);
    ASSERT_NEAR( 0., xsi, abs_error);
    
    p.x = 0.; p.y = 1.; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 1., eta, abs_error);
    ASSERT_NEAR( 0., xsi, abs_error);
    
    p.x = 0.; p.y = -1.; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0., eta, abs_error);
    ASSERT_NEAR( 1., xsi, abs_error);
    
    p.x = 1.; p.y = 0.; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 1., eta, abs_error);
    ASSERT_NEAR( 1., xsi, abs_error);            
}

// in this test, the hessian is not positive definite at the beginning, thus the algorithm would die if we dont do anything
TEST(TiglPointTranslator, indefinite_hessian){
    CTiglPoint x1(0,2,0);
    CTiglPoint x2(2,4,0);
    CTiglPoint x3(0,0,0);
    CTiglPoint x4(2,0,0);

    CTiglPoint p(1,1,0);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.5,   eta, abs_error);
    ASSERT_NEAR( 2./3., xsi, abs_error);
}

TEST(TiglPointTranslator, advanced){
    CTiglPoint x1(0,4,0);
    CTiglPoint x2(8,4,0);
    CTiglPoint x3(0,0,0);
    CTiglPoint x4(4,0,0);

    CTiglPoint p(3,2,0);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.5, eta, abs_error);
    ASSERT_NEAR( 0.5, xsi, abs_error);

    p.x = 1.5; p.y = 2; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.25, eta, abs_error);
    ASSERT_NEAR( 0.5 , xsi, abs_error);

    p.x = 1.5; p.y = 2; p.z = -3;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.25, eta, abs_error);
    ASSERT_NEAR( 0.5 , xsi, abs_error);
}

TEST(TiglPointTranslator, consistency){
    // create a bit more complex surface
    CTiglPoint x1(0,4,0);
    CTiglPoint x2(8,4,0);
    CTiglPoint x3(0,0,-1.);
    CTiglPoint x4(4,0,0.5);

    CTiglPointTranslator trans(x1, x2, x3, x4 );
    // now some random eta xsi pairs
    double eta_orig = 0.283;
    double xsi_orig = 0.8398;

    CTiglPoint p(0,0,0);
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(eta_orig,xsi_orig,&p) );

    //now translate back into eta xsi
    double eta, xsi;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );

    //we relax the error a bit, if we need more precision we have 
    //to adapt the algorithm
    double precision = 1e-5;
    ASSERT_NEAR( eta_orig, eta, precision);
    ASSERT_NEAR( xsi_orig, xsi, precision);

    //now another pair, interestingly, for this numbers we need the
    //backtracking. if we comment it out, the algorithm diverges!
    eta_orig = 0.84723;
    xsi_orig = 0.314159265;

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(eta_orig,xsi_orig,&p) );
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( eta_orig, eta, precision);
    ASSERT_NEAR( xsi_orig, xsi, precision);

    // lets test some points outside the quadriangle
    eta_orig = -1.735;
    xsi_orig = 2.3;

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(eta_orig,xsi_orig,&p) );
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( eta_orig, eta, precision);
    ASSERT_NEAR( xsi_orig, xsi, precision);
}

TEST(TiglPointTranslator, performance){
    int nruns = 100000;
    
    CTiglPoint x1(0,4,0);
    CTiglPoint x2(8,4,0);
    CTiglPoint x3(0,0,0);
    CTiglPoint x4(4,0,0);

    CTiglPoint p(3,2,1);

    double eta, xsi;
    double x = 0.;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    clock_t start = clock();

    for(int i = 0; i < nruns; ++i){
        trans.translate(p, &eta, &xsi) ;
        //just some dummy to prevent compiler optimization
        x = x + 1.0;
    }

    clock_t stop = clock();

    ASSERT_EQ((double)nruns, x);
    ASSERT_NEAR(0.5, eta, abs_error);
    ASSERT_NEAR(0.5, xsi, abs_error);
        
    double time_elapsed = (double)(stop - start)/(double)CLOCKS_PER_SEC/(double)nruns;
    time_elapsed *= 1000000.;
    printf("Elapsed average time: %f [us]\n", time_elapsed);
    
    ASSERT_TRUE(true);
}

TEST(TiglPointTranslator, Bug1){
    CTiglPoint x1(15.080271828981320681,6.3331944636655546077,-0.82588659024004418274);
    CTiglPoint x2(20.620103026537961455,16.956343945599122947,-0.089370094291781887463);
    CTiglPoint x3(18.836422557259382415,6.3331944636655546077,-0.95705426399850279662);
    CTiglPoint x4(22.115034014736966839,16.956343945599122947,-0.14157423473675034842);

    CTiglPoint p(21.344060867260836289,16.956343945599122947,-0.10327302824713803509);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
}

TEST(TiglPointTranslator, Bug2){
    CTiglPoint x1(49.949999820005999,-5.8928318504061994e-005,2.9999999998003934);
    CTiglPoint x2(56.939999932802237,-4.6876665182871935e-005,9.4999999998820623);
    CTiglPoint x3(55.949849998596576,0.00010609274864148424,3.000000000376426);
    CTiglPoint x4(59.179943999476052,1.4731199884798687e-005,9.5000000000971134);

    CTiglPoint p(58.609604046723632,0.044117587761595446,9.2291668207101427);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    
    ASSERT_LE(eta, 1.0);
    ASSERT_LE(xsi, 1.0);
    ASSERT_GE(eta, 0.0);
    ASSERT_GE(xsi, 0.0);
}

TEST(TiglPointTranslator, Bug3){
    // this bug provokes a line search error
    CTiglPoint x1(45.508156566358146,28.862645269916445,1.422736682335471);
    CTiglPoint x2(45.613850894119032,28.931475252736018,1.4325057334778124);
    CTiglPoint x3(47.97740962007417,28.862645269916445,1.5096965771722497);
    CTiglPoint x4(48.007314417208725,28.931475252736018,1.4325057334778124);

    CTiglPoint p(40.630419753446105,16.113766742873569,0.34096276352572791);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
}

TEST(TiglPointTranslator, Bug4){
    // this bug provokes a line search error
    CTiglPoint x1(45.508156566358146,28.862645269916445,1.422736682335471);
    CTiglPoint x2(45.613850894119032,28.931475252736018,1.4325057334778124);
    CTiglPoint x3(47.97740962007417,28.862645269916445,1.5096965771722497);
    CTiglPoint x4(48.007314417208725,28.931475252736018,1.4325057334778124);

    CTiglPoint p(39.866104718872023,16.193327665794129,0.34559303306764189);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
}


/**
 * This bug occured because of the different scale. In this case,
 * the tolerance of the gradient was chosen too small.
 * The gradient is quadratic in the scale, hence, the tolerance
 * must be adapted.
 */
TEST(TiglPointTranslator, Bug5){
    CTiglPoint x1(4000.0000000000000, 0.00000000000000000, 3000.0000000000000);
    CTiglPoint x2(6893.7199999999993, 4250.0000000000000, 3000.0000000000000);
    CTiglPoint x3(7554.9955629914994, 0.00000000000000000, 3017.7749461533222);
    CTiglPoint x4(8693.7789747175666, 4250.0000000000000, 3006.3001935476932);

    CTiglPoint p(7060.6260619422628, 1062.5000000000005, 3011.1796935014363);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4);

    ASSERT_EQ ( TIGL_SUCCESS, trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR(0.25, eta, 1e-5);
    ASSERT_NEAR(0.75, xsi, 1e-5);
}

