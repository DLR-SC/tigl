/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-06-10 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglSymetricSplineBuilder.h"

#include "CTiglError.h"

#include <Geom_BSplineCurve.hxx>
#include <Precision.hxx>
#include "CTiglPointsToBSplineInterpolation.h"
#include "CTiglBSplineAlgorithms.h"
#include "tiglcommonfunctions.h"

#include <algorithm>

#include <cassert>
#include <cmath>

namespace tigl
{

CTiglSymetricSplineBuilder::CTiglSymetricSplineBuilder(const CPointContainer& points)
    : _points(points)
{
}

Handle(Geom_BSplineCurve) CTiglSymetricSplineBuilder::GetBSpline() const
{
    checkInputData();


    CPointContainer points = _points;

    // y is already ~ 0, make it really zero!
    points[0].SetY(0.);

    // mirror each point at x-z plane i.e. mirror y coordinate to close the profile
    // and skip first point
    for (size_t i = _points.size() - 1; i > 0; i--) {
        gp_Pnt curP = _points[i];
        if (i == _points.size() - 1 && std::abs(curP.Y()) < 1e-6) {
            // do not add the same points twice
            continue;
        }
        curP.SetY(-1. * curP.Y());
        points.push_back(curP);
    }

    points.push_back(points[0]);

    auto pnts = OccArray(points);
    CTiglPointsToBSplineInterpolation interp(pnts, 3, true);
    auto c = interp.Curve();

    double umin, umax;
    umin = c->FirstParameter();
    umax = c->LastParameter();
    assert(fabs(c->Value((umin+umax)/2.).Y()) < Precision::Confusion());

    // Return the first half of the curve only
    c = CTiglBSplineAlgorithms::trimCurve(c, umin, (umin + umax) / 2.);

    return c;
}

void CTiglSymetricSplineBuilder::checkInputData() const
{
    gp_Pnt pstart = _points[0];
    gp_Pnt plast = _points[_points.size()-1];

    // the first point must start at the symmetry plane
    if (fabs(pstart.Y()) > Precision::Confusion()) {
        throw CTiglError("First point does not lie on x-z Plane (CTiglSymetricSplineBuilder::GetBSpline)!");
    }

    // the last point must start at the symmetry plane
    if (fabs(plast.Y()) > Precision::Confusion()) {
        throw CTiglError("Last point does not lie on x-z Plane (CTiglSymetricSplineBuilder::GetBSpline)!");
    }
}

} // namespace tigl
