/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
#include "CTiglPointTranslator.h"
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

TEST(TiglPointTranslator, performance){
    int nruns = 1000000;
    
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
	
	ASSERT_EQ((double)nruns, x);
	ASSERT_NEAR(0.5, eta, abs_error);
	ASSERT_NEAR(0.5, xsi, abs_error);
        
    clock_t stop = clock();
    double time_elapsed = (double)(stop - start)/(double)CLOCKS_PER_SEC/(double)nruns;
    time_elapsed *= 1000000.;
    printf("Elapsed average time: %f [us]\n", time_elapsed);
    
    ASSERT_TRUE(true);
   
}
