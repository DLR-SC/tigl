/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-09-30 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLINTERSECTBSPLINES_H
#define CTIGLINTERSECTBSPLINES_H

#include <Geom_BSplineCurve.hxx>
#include "CTiglPoint.h"

#include <vector>

namespace tigl
{

struct CurveIntersectionResult
{
    double parmOnCurve1;
    double parmOnCurve2;
    CTiglPoint point;
};

/**
 * @brief Computes all intersections of 2 B-Splines curves
 *
 * An intersection is counted, whenever the two curves come closer than tolerance.
 * If a whole interval is closes than tolerance, the nearest point in the interval is searched.
 *
 * The function divides the input curves and checks, if their bounding boxes (convex hulls)
 * intersect each other. If so, this process is repeated until the curve segment is almost a line segment.
 * This result is used to locally optimize into a true minimum.
 */
TIGL_EXPORT std::vector<CurveIntersectionResult> IntersectBSplines(const Handle(Geom_BSplineCurve) curve1, const Handle(Geom_BSplineCurve) curve2, double tolerance=1e-5);

} // namespace tigl

#endif // CTIGLINTERSECTBSPLINES_H
