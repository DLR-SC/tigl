/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2012-12-18 Martin Siggel <Martin.Siggel@dlr.de>
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
#include "tigl.h"

#include "math/ITiglObjectiveFunction.h"
#include "math/CTiglOptimizer.h"

#include <cmath>

using namespace tigl;

#define SQR(x) ((x)*(x))

namespace
{
    inline double MAX(double a, double b){ return a>b? a : b;}
}

class Rosenbrock2d : public tigl::ITiglObjectiveFunction
{
public:
    int getParameterCount() const OVERRIDE { return 2; }

    bool hasAnalyticGradient() const OVERRIDE{ return true; };
    bool hasAnalyticHessian () const OVERRIDE{ return true; };

    double getFunctionValue(const double * x) const OVERRIDE;
    void  getGradient(const double * x, double * dx) const OVERRIDE;
    void  getHessian (const double * x, double * H) const OVERRIDE;
};


double Rosenbrock2d::getFunctionValue(const double * x) const
{
    const double& x1 = x[0];
    const double& x2 = x[1];

    return SQR(1-x1) + 100. * SQR(x2-SQR(x1));
}

void Rosenbrock2d::getGradient(const double * x, double * dx) const
{
    const double& x1 = x[0];
    const double& x2 = x[1];

    dx[0] = -2.*(1-x1) -400. * x1 * (x2-SQR(x1));
    dx[1] = 200. * (x2-SQR(x1));
}

void Rosenbrock2d::getHessian(const double * x, double * H) const
{
    const double& x1 = x[0];
    const double& x2 = x[1];

    int n = getParameterCount();

    TIGL_MATRIX2D(H,n,1,1) =  200.;
    TIGL_MATRIX2D(H,n,0,1) = -400.*x1;
    TIGL_MATRIX2D(H,n,1,0) = -400.*x1;
    TIGL_MATRIX2D(H,n,0,0) = -400.*(-3.*SQR(x1) + x2) + 2.;
}

TEST(TiglOptimizer, numericalObjectiveFunction){

    Rosenbrock2d r;

    double x[2] = {1,1};

    ASSERT_NEAR(0., r.getFunctionValue(x), 1.e-10);

    double dx[2], dxnum[2];
    double Hnum[4], Hana[4];

    r.getGradient(x,dx);
    ASSERT_NEAR(0., dx[0], 1.e-10);
    ASSERT_NEAR(0., dx[1], 1.e-10);

    // check consistency of numerical hessian and gradient
    double prec = 1e-2;

    x[0] = 2.71828183; x[1] = 3.14159265;
    r.getGradient(x,dx);
    r.getNumericalGradient(x,dxnum);
    r.getHessian(x,Hana);
    r.getNumericalHessian(x,Hnum);

    //check gradient
    ASSERT_NEAR(dx[0], dxnum[0], MAX(prec, prec*fabs(dx[0])));
    ASSERT_NEAR(dx[1], dxnum[1], MAX(prec, prec*fabs(dx[1])));

    //check hessian
    ASSERT_NEAR(Hana[0], Hnum[0], MAX(prec, prec*fabs(Hana[0])));
    ASSERT_NEAR(Hana[1], Hnum[1], MAX(prec, prec*fabs(Hana[1])));
    ASSERT_NEAR(Hana[2], Hnum[2], MAX(prec, prec*fabs(Hana[2])));
    ASSERT_NEAR(Hana[3], Hnum[3], MAX(prec, prec*fabs(Hana[3])));

    x[0] = 100.; x[1] = -2.;
    r.getGradient(x,dx);
    r.getNumericalGradient(x,dxnum);
    r.getHessian(x,Hana);
    r.getNumericalHessian(x,Hnum);

    //check gradient
    ASSERT_NEAR(dx[0], dxnum[0], MAX(prec, prec*fabs(dx[0])));
    ASSERT_NEAR(dx[1], dxnum[1], MAX(prec, prec*fabs(dx[1])));

    //check hessian
    ASSERT_NEAR(Hana[0], Hnum[0], MAX(prec, prec*fabs(Hana[0])));
    ASSERT_NEAR(Hana[1], Hnum[1], MAX(prec, prec*fabs(Hana[1])));
    ASSERT_NEAR(Hana[2], Hnum[2], MAX(prec, prec*fabs(Hana[2])));
    ASSERT_NEAR(Hana[3], Hnum[3], MAX(prec, prec*fabs(Hana[3])));
}

TEST(TiglOptimizer, optimizeRosenbrock){
    Rosenbrock2d r;

    double x[2];
    x[0] = -1.2; x[1] = 1;

    //solution is at (1,1)
    ASSERT_TRUE(CTiglOptimizer::optNewton2d(r, x,1e-15,1e-8) == TIGL_SUCCESS);

    ASSERT_NEAR(1., x[0], 1e-6);
    ASSERT_NEAR(1., x[1], 1e-6);
}


