/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-11-17 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCSTCurveBuilder.h"

#include "tiglmathfunctions.h"
#include "CFunctionToBspline.h"

#include <cassert>

namespace
{

    class CSTFunction : public tigl::MathFunc3d
    {
    public:
        CSTFunction(tigl::CCSTCurveBuilder* b)
            : builder(b)
        {
        }

        /// we reparameterize the cst curve to achieve correct leading and trailing edge
        /// interpolation
        double valueX(double t) OVERRIDE
        {

            /// determine x parameterization depending on N1 and N2
            double N1 = builder->N1();
            double N2 = builder->N2();

            if (N1 >= 1. && N2 >= 1.) {
                // hypersonic biconvex airfoil
                return t;
            }
            else if (N1 < 1 && N2 >= 1.) {
                // classical airfoil
                return t*t;
            }
            else if (N1 >= 1 && N2 < 1) {
                // inverted airfoil
                return -t*t + 2*t;
            }
            else {
                // N1 < 1 && N2 < 1
                // elliptic body
                return -2.*t*t*t + 3*t*t;
            }
        }

        double valueY(double t) OVERRIDE
        {
            double x = valueX(t);


            return tigl::cstcurve(builder->N1(), builder->N2(), builder->B(), builder->T(), x);
        }

        double valueZ(double /*t*/) OVERRIDE
        {
            return 0;
        }

    private:
        tigl::CCSTCurveBuilder* builder;
    };
}

namespace tigl
{

CCSTCurveBuilder::CCSTCurveBuilder(double N1, double N2, const std::vector<double>& B, double T)
{
    _n1 = N1; _n2 = N2; _b = B; _t = T;
}

double CCSTCurveBuilder::N1() const
{
    return _n1;
}

double CCSTCurveBuilder::N2() const
{
    return _n2;
}

double CCSTCurveBuilder::T() const
{
    return _t;
}

std::vector<double> CCSTCurveBuilder::B() const
{
    return _b;
}

Handle(Geom_BSplineCurve) CCSTCurveBuilder::Curve()
{
    CSTFunction function(this);
    CFunctionToBspline approximator(function, 0., 1., 4, 1e-5, 10);
    return approximator.Curve();
}

}
