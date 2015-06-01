/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-06-01 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CPOINTSTOLINEARBSPLINE_H
#define CPOINTSTOLINEARBSPLINE_H

#include <vector>
#include <gp_Pnt.hxx>
#include <Handle_Geom_BSplineCurve.hxx>

namespace tigl
{

/**
 * @brief This class creates a linaer B-Spline interpolation
 * of the given points.
 */
class CPointsToLinearBSpline
{
public:
    CPointsToLinearBSpline(const std::vector<gp_Pnt>& points);

    Handle_Geom_BSplineCurve Curve() const;

    operator Handle_Geom_BSplineCurve() const;

    ~CPointsToLinearBSpline();

private:
    const std::vector<gp_Pnt>& _points;
};


} // namespace tigl

#endif // CPOINTSTOLINEARBSPLINE_H
