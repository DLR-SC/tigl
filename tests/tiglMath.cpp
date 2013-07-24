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
