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

#ifndef CCSTCURVEBUILDER_H
#define CCSTCURVEBUILDER_H

#include "tigl_internal.h"


#include <vector>

#include <Geom_BSplineCurve.hxx>

namespace tigl
{

/**
 * @brief The CCSTCurveBuilder class generates a B-Spline Curve from a CST parameterization
 */
class CCSTCurveBuilder
{
public:

    /**
     * @brief The Algorithm enum gives a choice of the method.
     *
     * Piecewise_Chebychev_Approximation subdivides the curve into
     * segments, where each segment is approximated using a Chebychev polynomials.
     * The final result is the C1 concatenation of these polynomials to a B-Spline
     *
     * GeomAPI_PointsToBSpline uses OCCT's internal approximation algorithm to create
     * a B-Spline that approximates a CST Curve.
     */
    enum class Algorithm {
        Piecewise_Chebychev_Approximation = 0,
        GeomAPI_PointsToBSpline
    };

    TIGL_EXPORT CCSTCurveBuilder(double N1, double N2, const std::vector<double>& B, double T, Algorithm method=Algorithm::Piecewise_Chebychev_Approximation);

    // returns parameters of cst curve
    TIGL_EXPORT double N1() const;
    TIGL_EXPORT double N2() const;
    TIGL_EXPORT std::vector<double> B() const;
    TIGL_EXPORT double T() const;

    TIGL_EXPORT Handle(Geom_BSplineCurve) Curve();

private:
    double _n1, _n2, _t;
    std::vector<double> _b;
    int _degree;
    double _tol;
    Algorithm _algo;
};

} // namespace tigl

#endif // CCSTCURVEBUILDER_H
