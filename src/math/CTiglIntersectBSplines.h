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

TIGL_EXPORT std::vector<CurveIntersectionResult> IntersectBSplines(const Handle(Geom_BSplineCurve) curve1, const Handle(Geom_BSplineCurve) curve2, double tolerance=1e-5);

} // namespace tigl

#endif // CTIGLINTERSECTBSPLINES_H
