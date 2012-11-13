/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-13 Martin Siggel <Martin.Siggel@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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

using namespace tigl;

TEST(TiglPointTranslator, simple){
    CTiglPoint x1(0,2,0);
    CTiglPoint x2(2,2,0);
    CTiglPoint x3(0,0,0);
    CTiglPoint x4(2,0,0);

    CTiglPoint p(1,1,0);

    double eta, xsi;

    CTiglPointTranslator trans(x1, x2, x3, x4 );

    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.5, eta, 1e-5);
    ASSERT_NEAR( 0.5, xsi, 1e-5);

    p.x = 1; p.y = 0; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.5, eta, 1e-5);
    ASSERT_NEAR( 1.0, xsi, 1e-5);

    p.x = 0; p.y = 0; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.0, eta, 1e-5);
    ASSERT_NEAR( 1.0, xsi, 1e-5);

    p.x = 1.5; p.y = 1; p.z = 3;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.75, eta, 1e-5);
    ASSERT_NEAR( 0.5 , xsi, 1e-5);

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
    ASSERT_NEAR( 0.5, eta, 1e-5);
    ASSERT_NEAR( 0.5, xsi, 1e-5);

    p.x = 1.5; p.y = 2; p.z = 0;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.25, eta, 1e-5);
    ASSERT_NEAR( 0.5 , xsi, 1e-5);

    p.x = 1.5; p.y = 2; p.z = -3;
    ASSERT_EQ ( TIGL_SUCCESS,  trans.translate(p, &eta, &xsi) );
    ASSERT_NEAR( 0.25, eta, 1e-5);
    ASSERT_NEAR( 0.5 , xsi, 1e-5);
}
