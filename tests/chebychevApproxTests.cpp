/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-11-18 Martin Siggel <martin.siggel@dlr.de>
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
#include "tiglmathfunctions.h"
#include "CTiglError.h"

/******************************************************************************/

class ChebychevApproxTests : public ::testing::Test 
{
protected:
    static void SetUpTestCase() 
    {
    }

    static void TearDownTestCase() 
    {
    }

    virtual void SetUp() {}
    virtual void TearDown() {}
};

namespace
{
    // some functions to approximate
    double parabola(double x, void*)
    {
        return x*x;
    }
    
    double line(double x, void*)
    {
        return -5.*x;
    }
    
    double shifted_parabola(double x, void*)
    {
        return 2.*(x-4.)*(x-4.) - 1.;
    }
    
    double cubic_with_parameter(double x, void* p)
    {
        double par = *(double*)p;
        return par*x*x*x;
    }

}

TEST_F(ChebychevApproxTests, parabola)
{
    math_Vector v = tigl::cheb_approx(parabola, NULL, 4, -1., 1.);
    ASSERT_NEAR(0.5, v(0), 1e-12);
    ASSERT_NEAR(0.0, v(1), 1e-12);
    ASSERT_NEAR(0.5, v(2), 1e-12);
    ASSERT_NEAR(0.0, v(3), 1e-12);
}

TEST_F(ChebychevApproxTests, line)
{
    math_Vector v = tigl::cheb_approx(line, NULL, 4, -1., 1.);
    ASSERT_NEAR(0.0, v(0), 1e-12);
    ASSERT_NEAR(-5., v(1), 1e-12);
    ASSERT_NEAR(0.0, v(2), 1e-12);
    ASSERT_NEAR(0.0, v(3), 1e-12);
}

TEST_F(ChebychevApproxTests, line_invalid)
{
    ASSERT_THROW(tigl::cheb_approx(line, NULL, 0, -1., 1.), tigl::CTiglError);
   
}

TEST_F(ChebychevApproxTests, shifted_parabola)
{
    math_Vector v = tigl::cheb_approx(shifted_parabola, NULL, 4, 3., 5.);
    ASSERT_NEAR(0.0, v(0), 1e-12);
    ASSERT_NEAR(0.0, v(1), 1e-12);
    ASSERT_NEAR(1.0, v(2), 1e-12);
    ASSERT_NEAR(0.0, v(3), 1e-12);
}

TEST_F(ChebychevApproxTests, cubic_with_parameter)
{
    double parm = 7.;
    math_Vector v = tigl::cheb_approx(cubic_with_parameter, &parm, 10, -1., 1.);
    ASSERT_NEAR(0.0, v(0), 1e-12);
    ASSERT_NEAR(0.75*parm, v(1), 1e-12);
    ASSERT_NEAR(0.0    , v(2), 1e-12);
    ASSERT_NEAR(parm/4., v(3), 1e-12);
    ASSERT_NEAR(0.0, v(4), 1e-12);
    ASSERT_NEAR(0.0, v(5), 1e-12);
    ASSERT_NEAR(0.0, v(6), 1e-12);
    ASSERT_NEAR(0.0, v(7), 1e-12);
    ASSERT_NEAR(0.0, v(8), 1e-12);
    ASSERT_NEAR(0.0, v(9), 1e-12);
}
