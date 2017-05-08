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

#include <TColgp_HArray1OfPnt.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>
#include <Precision.hxx>

#include <algorithm>

#include <cassert>

namespace
{
    // Symmetrizes two control points along the x-z plane
    void symPole(Handle(Geom_BSplineCurve) c, int i1, int i2)
    {
        gp_Pnt p1 = c->Pole(i1);
        gp_Pnt p2 = c->Pole(i2);

        assert(fabs(p1.Y() + p2.Y()) < Precision::Confusion());
        // z will be symmetrized
        double pmeanz = (p1.Z() + p2.Z())/2.;
        double pmeany = (p1.Y() - p2.Y())/2.;

        p1.SetY(pmeany);
        p2.SetY(-pmeany);
        p1.SetZ(pmeanz);
        p2.SetZ(pmeanz);

        c->SetPole(i1, p1);
        c->SetPole(i2, p2);
    }
}

namespace tigl
{

CTiglSymetricSplineBuilder::CTiglSymetricSplineBuilder(const CPointContainer& points)
    : _points(points)
{
}

Handle(Geom_BSplineCurve) CTiglSymetricSplineBuilder::GetBSpline() const
{
    checkInputData();

    // spline through the points in both directions
    // then take the average of both curves to achieve
    // symmetry wrt x-y plane

    // forward spline
    Handle(Geom_BSplineCurve) c1 =  GetBSplineInternal(_points);

    CPointContainer pointsRev = _points;
    std::reverse(pointsRev.begin(), pointsRev.end());

    // backward spline
    Handle(Geom_BSplineCurve) c2 =  GetBSplineInternal(pointsRev);
    // reverse it, to achieve same direction as c1
    c2->Reverse();

#ifdef DEBUG
    // check assumptions (knots must be same in both curves)
    assert(c1->NbKnots() == c2->NbKnots());
    assert(c1->NbPoles() == c2->NbPoles());

    for (int ik = 1; ik <= c1->NbKnots(); ++ik) {
        assert(fabs(c1->Knot(ik) - c2->Knot(ik)) < 1e-8);
    }
#endif

    // symmetrize control points values
    for (int icp = 1; icp <= c1->NbPoles(); ++icp) {
        gp_Pnt pMean = 0.5*(c1->Pole(icp).XYZ() + c2->Pole(icp).XYZ());
        c1->SetPole(icp, pMean);
    }

    return c1;
}

Handle(Geom_BSplineCurve) CTiglSymetricSplineBuilder::GetBSplineInternal(const CPointContainer& inputPoints) const
{
    CPointContainer points = inputPoints;

    // mirror each point at x-z plane i.e. mirror y coordinate to close the profile
    // and skip first point
    for (int i = static_cast<int>(inputPoints.size()) - 1; i > 0; i--) {
        gp_Pnt curP = inputPoints[i];
        if (i == inputPoints.size() - 1 && std::abs(curP.Y()) < 1e-6) {
            // do not add the same points twice
            continue;
        }
        curP.SetY(-1. * curP.Y());
        points.push_back(curP);
    }

    // build interpolation curve
    Handle(TColgp_HArray1OfPnt) pnts = new TColgp_HArray1OfPnt(1, static_cast<Standard_Integer>(points.size()));
    for (unsigned int i = 0; i < points.size(); ++i) {
        pnts->SetValue(i+1, points[i]);
    }

    // The interpolation gives a closed but unsymmetric result
    GeomAPI_Interpolate interpolator(pnts, true, 1e-7);
    interpolator.Perform();

    Handle(Geom_BSplineCurve) c = interpolator.Curve();
    // This is required in order to get a clamped bspline
    // with symmetric knots
    c->SetNotPeriodic();

    // symmetrize control points
    for (int icp = 1; 2*icp <= c->NbPoles(); ++icp) {
        int i1 = icp;
        int i2 = c->NbPoles() - icp + 1;
        symPole(c, i1, i2);
    }

    // trim the curve to the first half
    double umin, umax;
    umin = c->FirstParameter();
    umax = c->LastParameter();
    assert(fabs(c->Value((umin+umax)/2.).Y()) < Precision::Confusion());

    c = GeomConvert::CurveToBSplineCurve(new Geom_TrimmedCurve(c, umin, (umin+umax)/2.));

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

