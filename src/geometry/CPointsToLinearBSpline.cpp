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

#include "CPointsToLinearBSpline.h"

#include <Geom_BSplineCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

namespace tigl
{

CPointsToLinearBSpline::CPointsToLinearBSpline(const std::vector<gp_Pnt>& points)
    : _points(points)
{
}

Handle(Geom_BSplineCurve) CPointsToLinearBSpline::Curve() const
{
    int ncp = (int) _points.size();
    std::vector<gp_Pnt>::const_iterator iter;

    // compute total length and copy control points
    int icp = 1;
    gp_Pnt lastP = _points[0];
    double totalLen = 0.;
    TColgp_Array1OfPnt cp(1, ncp);
    for (iter = _points.begin(); iter != _points.end(); ++iter) {
        gp_Pnt p = *iter;
        double segLen = p.Distance(lastP);
        totalLen += segLen;
        cp.SetValue(icp++, p);
        lastP = p;
    }

    // compute knots
    TColStd_Array1OfReal knots(1, ncp);
    TColStd_Array1OfInteger mults(1, ncp);

    lastP = _points[0];
    double lastKnot = 0;
    for (int i = 1; i <= ncp; ++i) {
        if (i == 1 || i == ncp) {
            mults.SetValue(i,2);
        }
        else {
            mults.SetValue(i,1);
        }

        double knot = cp.Value(i).Distance(lastP)/totalLen + lastKnot;
        knots.SetValue(i, knot);

        lastKnot = knot;
        lastP = cp.Value(i);
    }

    return new Geom_BSplineCurve(cp, knots, mults, 1, false);
}

CPointsToLinearBSpline::operator Handle(Geom_BSplineCurve)() const
{
    return Curve();
}

CPointsToLinearBSpline::~CPointsToLinearBSpline()
{
}


} // namespace tigl
