/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-01-18 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglArcLengthReparameterization.h"

#include "CFunctionToBspline.h"
#include "tiglmathfunctions.h"
#include "CTiglError.h"

#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>

namespace tigl
{

class BSplineArcLength : public MathFunc3d
{
public:
    BSplineArcLength(const Handle(Geom_BSplineCurve) curve)
        : _curve(curve)
    {
        // intentionally empty
    }

    double getParameter(double arcLength);

    // returns the total length of the spline
    double getTotalLength();

    double valueX(double t) OVERRIDE {
        return getParameter(t);
    }

    double valueY(double t) OVERRIDE
    {
        return 0.;
    }
    double valueZ(double t) OVERRIDE
    {
        return 0.;
    }

private:
    GeomAdaptor_Curve _curve;
};

double BSplineArcLength::getParameter(double arcLength)
{
    GCPnts_AbscissaPoint algo(_curve, arcLength, _curve.FirstParameter());
    if (algo.IsDone()) {
        return algo.Parameter();
    }
    else {
        throw tigl::CTiglError("BSplineArcLength: can't compute paramter for arc given length.", TIGL_MATH_ERROR);
    }
}

double BSplineArcLength::getTotalLength()
{
    return GCPnts_AbscissaPoint::Length(_curve);
}


CTiglArcLengthReparameterization::CTiglArcLengthReparameterization()
    : _isInitialized(false)
{
}

bool CTiglArcLengthReparameterization::isInitialized() const
{
    return _isInitialized;
}

double CTiglArcLengthReparameterization::parameter(double arcLength) const
{
    if (!isInitialized()) {
        throw CTiglError("CTiglArcLengthReparameterization is not initialized", TIGL_MATH_ERROR);
    }

    return _reparCurve->Value(arcLength).X();
}

double CTiglArcLengthReparameterization::totalLength() const
{
    if (!isInitialized()) {
        throw CTiglError("CTiglArcLengthReparameterization is not initialized", TIGL_MATH_ERROR);
    }

    return _totalLength;
}

void CTiglArcLengthReparameterization::init(const Handle(Geom_BSplineCurve) curve, double accuracy)
{
    _isInitialized = false;

    BSplineArcLength arcLenFun(curve);

    // Compute total arc length
    _totalLength = arcLenFun.getTotalLength();

    CFunctionToBspline approximator(arcLenFun, 0., _totalLength, 3, accuracy/10.);
    _reparCurve = approximator.Curve();

    _isInitialized = true;
}

} // namespace tigl
