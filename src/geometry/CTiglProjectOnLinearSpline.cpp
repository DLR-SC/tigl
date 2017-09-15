/* 
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-15-01 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglProjectOnLinearSpline.h"

#include "CTiglError.h"

namespace tigl
{

CTiglProjectOnLinearSpline::CTiglProjectOnLinearSpline(Handle(Geom_BSplineCurve) curve, gp_Pnt p)
    : _curve(curve), _point(p), _isDone(false)
{
}

bool CTiglProjectOnLinearSpline::IsDone()
{
    compute();

    return _isDone;
}

gp_Pnt CTiglProjectOnLinearSpline::Point()
{
    if (!IsDone()) {
        throw CTiglError("Can not compute proejection point in CTiglProjectOnLinearSpline::Point", TIGL_MATH_ERROR);
    }

    return _pProj;
}

double CTiglProjectOnLinearSpline::Parameter()
{
    if (!IsDone()) {
        throw CTiglError("Can not compute proejection parameter in CTiglProjectOnLinearSpline::Parameter", TIGL_MATH_ERROR);
    }
    
    return _param;
}

CTiglProjectOnLinearSpline::~CTiglProjectOnLinearSpline()
{
}

bool CTiglProjectOnLinearSpline::compute()
{
    if (_isDone) {
        return true;
    }

    // check if curve is linear
    if (_curve->Degree() != 1) {
        return false;
    }

    double minDist = FLT_MAX;

    for (int i = 1; i < _curve->NbPoles(); ++i) {
        gp_Pnt p1 = _curve->Pole(i);
        gp_Pnt p2 = _curve->Pole(i+1);
        
        // project point onto line p1-p2
        gp_XYZ p1p2 = p1.XYZ() - p2.XYZ();
        gp_XYZ p1p  = p1.XYZ() - _point.XYZ();
        double alpha = p1p2.Dot(p1p) / p1p2.Dot(p1p2);

        if (alpha >= 0. && alpha <= 1.) {
            gp_Pnt pproj = p1.XYZ() - alpha * p1p2;

            double parm = _curve->Knot(i) + alpha*(_curve->Knot(i+1) - _curve->Knot(i));
            double dist = pproj.Distance(_point);

            if (dist < minDist) {
                _pProj = pproj;
                _param = parm;
                minDist = dist;
            }
        }
    }

    if (minDist < FLT_MAX) {
        _isDone = true;
    }
    else {
        _isDone = false;
    }

    return _isDone;
}

}
