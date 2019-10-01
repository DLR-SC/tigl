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
    
    CTiglPoint r = p - other.p;
    double a = d.norm2Sqr();
    double e = other.d.norm2Sqr();
    double f = CTiglPoint::inner_prod(other.d, r);
    
    const double EPS = 1e-8;
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
            float denom = a*e - b*b;
            
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
    
    CTiglPoint c1 = value(s);
    CTiglPoint c2 = other.value(t);
    return sqrt(c1.distance2(c2));
}

} // namespace tigl
