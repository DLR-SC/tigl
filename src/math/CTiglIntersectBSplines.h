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
 * The function devides the imput curves and check, if their bounding boxes (convex hulls)
 * intersect each other. If so, this process is repeated until the curve segment is almost a line segment.
 * This result is used to locally optimize into a true minimum.
 */
TIGL_EXPORT std::vector<CurveIntersectionResult> IntersectBSplines(const Handle(Geom_BSplineCurve) curve1, const Handle(Geom_BSplineCurve) curve2, double tolerance=1e-5);

} // namespace tigl

#endif // CTIGLINTERSECTBSPLINES_H
