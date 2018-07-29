#ifndef CTIGLPOINTSTOBSPLINEINTERPOLATION_H
#define CTIGLPOINTSTOBSPLINEINTERPOLATION_H

#include "tigl_internal.h"

#include <Geom_BSplineCurve.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <math_Matrix.hxx>
#include <vector>

namespace tigl
{

class CTiglPointsToBSplineInterpolation
{
public:
    TIGL_EXPORT CTiglPointsToBSplineInterpolation(const Handle(TColgp_HArray1OfPnt)& points, unsigned int maxDegree = 3, bool continuousIfClosed = false);

    TIGL_EXPORT CTiglPointsToBSplineInterpolation(const Handle(TColgp_HArray1OfPnt)& points, const std::vector<double>& parameters, unsigned int maxDegree = 3, bool continuousIfClosed = false);

    TIGL_EXPORT Handle(Geom_BSplineCurve) Curve() const;

    TIGL_EXPORT operator Handle(Geom_BSplineCurve)() const;

private:
    /// computes the maximum distance of the given points
    /// TODO: move to bsplinealgorithms::scale
    double maxDistanceOfBoundingBox(const TColgp_Array1OfPnt& points) const;

    bool isClosed() const;

    math_Matrix getContinuityMatrix(int nCtrPnts, const std::vector<double>& params, const TColStd_Array1OfReal& flatKnots) const;
    size_t nContiuityConditions() const;

    /// curve coordinates to be fitted by the B-spline
    const Handle(TColgp_HArray1OfPnt)& m_pnts;

    std::vector<double> m_params;


    /// degree of the B-spline
    int m_degree;

    /// Number of control points of the B-spline
    int m_ncp;

    /// determines the continuous closing of curve
    bool  m_C2Continuous;
};

} // namespace tigl

#endif // CTIGLPOINTSTOBSPLINEINTERPOLATION_H
