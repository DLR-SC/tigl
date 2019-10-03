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

#include "CTiglLineSegment.h"

#include "tiglcommonfunctions.h"

namespace tigl
{

CTiglLineSegment::CTiglLineSegment(const tigl::CTiglPoint &p1, const tigl::CTiglPoint &p2)
    : p(p1), d(p2 - p1)
{
}

CTiglPoint CTiglLineSegment::value(double u) const
{
    return p + d*u;
}

double CTiglLineSegment::distance(const CTiglLineSegment& other) const
{
    // Christer Ericson, Real Time Collision Detection
    
    const CTiglPoint r = p - other.p;
    const double a = d.norm2Sqr();
    const double e = other.d.norm2Sqr();
    const double f = CTiglPoint::inner_prod(other.d, r);
    
    const double EPS = 1e-8;

    // The parameters of the points of minimal distance
    double s = 0., t = 0.;
    
    if (a <= EPS && e <= EPS) {
        s = t = 0.;
    }
    else if(a <= EPS) {
        s = 0.;
        t = Clamp(f/e, 0., 1.);
    }
    else {
        double c = CTiglPoint::inner_prod(d, r);
        if (e <= EPS) {
            t = 0.;
            s = Clamp(-c/a, 0., 1.);
        }
        else {
            double b = CTiglPoint::inner_prod(d, other.d);
            double denom = a*e - b*b;
            
            if (denom != 0.) {
                s = Clamp((b*f - c*e)/ denom, 0., 1.);
            }
            else {
                s = 0.;
            }
            t = (b*s + f);
            
            if (t < 0.) {
                t = 0.;
                s = Clamp( -c/a, 0., 1.);
            }
            else if (t > e) {
                t = 1.;
                s = Clamp((b-c) / a, 0., 1.);
            }
            else {
                t = t / e;
            }
        }
    }
    
    const CTiglPoint c1 = value(s);
    const CTiglPoint c2 = other.value(t);
    return sqrt(c1.distance2(c2));
}

} // namespace tigl
