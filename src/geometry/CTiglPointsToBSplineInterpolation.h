/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-08-06 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLPOINTSTOBSPLINEINTERPOLATION_H
#define CTIGLPOINTSTOBSPLINEINTERPOLATION_H

#include "tigl_internal.h"

#include <Geom_BSplineCurve.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <math_Matrix.hxx>
#include <vector>

namespace tigl
{

/**
 * @brief Implements the b-spline interpolation algorithm as described by
 * Park (2000): Choosing nodes and knots in closed B-spline curve interpolation to point data
 */
class CTiglPointsToBSplineInterpolation
{
public:
    TIGL_EXPORT CTiglPointsToBSplineInterpolation(const Handle(TColgp_HArray1OfPnt) & points,
                                                  unsigned int maxDegree = 3, bool continuousIfClosed = false);

    TIGL_EXPORT CTiglPointsToBSplineInterpolation(const Handle(TColgp_HArray1OfPnt) & points,
                                                  const std::vector<double>& parameters, unsigned int maxDegree = 3,
                                                  bool continuousIfClosed = false);

    /// Returns the interpolation curve
    TIGL_EXPORT Handle(Geom_BSplineCurve) Curve() const;

    TIGL_EXPORT operator Handle(Geom_BSplineCurve)() const;

    /// Returns the parameters of the interpolated points
    TIGL_EXPORT const std::vector<double>& Parameters() const;

    /// Returns the degree of the b-spline interpolation
    TIGL_EXPORT unsigned int Degree() const;

private:
    /// computes the maximum distance of the given points
    /// TODO: move to bsplinealgorithms::scale
    double maxDistanceOfBoundingBox(const TColgp_Array1OfPnt& points) const;

    bool isClosed() const;

    bool needsShifting() const;

    /// curve coordinates to be fitted by the B-spline
    const Handle(TColgp_HArray1OfPnt) & m_pnts;

    std::vector<double> m_params;

    /// degree of the B-spline
    int m_degree;

    /// determines the continuous closing of curve
    bool m_C2Continuous;
};

} // namespace tigl

#endif // CTIGLPOINTSTOBSPLINEINTERPOLATION_H
