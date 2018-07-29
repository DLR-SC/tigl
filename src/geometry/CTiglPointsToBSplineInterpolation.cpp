#include "CTiglPointsToBSplineInterpolation.h"

#include "CTiglError.h"
#include "CTiglBSplineAlgorithms.h"

#include <BSplCLib.hxx>
#include <math_Gauss.hxx>

#include <algorithm>

namespace
{

Handle(TColStd_HArray1OfReal) toArray(const std::vector<double>& vector)
{
    Handle(TColStd_HArray1OfReal) array = new TColStd_HArray1OfReal(1, static_cast<int>(vector.size()));
    int ipos = 1;
    for (std::vector<double>::const_iterator it = vector.begin(); it != vector.end(); ++it, ipos++) {
        array->SetValue(ipos, *it);
    }

    return array;
}

} // namespace

namespace tigl
{

CTiglPointsToBSplineInterpolation::CTiglPointsToBSplineInterpolation(const Handle(TColgp_HArray1OfPnt)& points, unsigned int maxDegree, bool continuousIfClosed)
    : m_pnts(points)
    , m_degree(maxDegree)
    , m_C2Continuous(continuousIfClosed)
{

    m_params = CTiglBSplineAlgorithms::computeParamsBSplineCurve(points);
}

CTiglPointsToBSplineInterpolation::CTiglPointsToBSplineInterpolation(const Handle(TColgp_HArray1OfPnt)& points, const std::vector<double> &parameters, unsigned int maxDegree, bool continuousIfClosed)
    : m_pnts(points)
    , m_degree(maxDegree)
    , m_params(parameters)
    , m_C2Continuous(continuousIfClosed)
{
    if (m_params.size() != m_pnts->Length()) {
        throw CTiglError("Number of parameters and points don't match in CTiglPointsToBSplineInterpolation");
    }
}

Handle(Geom_BSplineCurve) CTiglPointsToBSplineInterpolation::Curve() const
{
    unsigned int max_degree = std::min(m_pnts->Length(), m_degree);

    int n_continuityConditions = nContiuityConditions();
    std::vector<double> knots = CTiglBSplineAlgorithms::knotsFromCurveParameters(m_params, max_degree, n_continuityConditions);

    Handle(TColStd_HArray1OfReal) occFlatKnots = toArray(knots);
    int knotsLen = BSplCLib::KnotsLength(occFlatKnots->Array1());

    TColStd_Array1OfReal occKnots(1, knotsLen);
    TColStd_Array1OfInteger occMults(1, knotsLen);
    BSplCLib::Knots(occFlatKnots->Array1(), occKnots, occMults);


    int nCtrPnts = m_params.size() + n_continuityConditions;
    int nParams = m_params.size();
    math_Matrix lhs(1, nCtrPnts, 1, nCtrPnts);
    lhs.Init(0.);

    math_Matrix A = CTiglBSplineAlgorithms::bsplineBasisMat(max_degree, occFlatKnots->Array1(), toArray(m_params)->Array1());
    lhs.Set(1, nParams, 1, nCtrPnts, A);

    // sets the C2 continuity constraints for closed curves on the left hand side if requested
    if (n_continuityConditions > 0) {
        math_Matrix continuity_entries = getContinuityMatrix(nCtrPnts,
                                                             m_params, occFlatKnots->Array1());
        lhs.Set(nParams + 1, nCtrPnts, 1, nCtrPnts, continuity_entries);
    }

    // right hand side
    math_Vector rhsx(1, nCtrPnts, 0.);
    math_Vector rhsy(1, nCtrPnts, 0.);
    math_Vector rhsz(1, nCtrPnts, 0.);
    for (int i = 1; i<= m_pnts->Length(); ++i) {
        const gp_Pnt& p = m_pnts->Value(i);
        rhsx(i) = p.X();
        rhsy(i) = p.Y();
        rhsz(i) = p.Z();
    }

    math_Gauss solver(lhs);

    math_Vector cp_x(1, nCtrPnts);
    math_Vector cp_y(1, nCtrPnts);
    math_Vector cp_z(1, nCtrPnts);

    solver.Solve(rhsx, cp_x);
    if (!solver.IsDone()) {
        throw CTiglError("Singular Matrix", TIGL_MATH_ERROR);
    }

    solver.Solve(rhsy, cp_y);
    if (!solver.IsDone()) {
        throw CTiglError("Singular Matrix", TIGL_MATH_ERROR);
    }

    solver.Solve(rhsz, cp_z);
    if (!solver.IsDone()) {
        throw CTiglError("Singular Matrix", TIGL_MATH_ERROR);
    }

    TColgp_Array1OfPnt poles(1, nCtrPnts);
    for (Standard_Integer icp = 1; icp <= nCtrPnts; ++icp) {
        gp_Pnt pnt(cp_x.Value(icp), cp_y.Value(icp), cp_z.Value(icp));
        poles.SetValue(icp, pnt);
    }

    Handle(Geom_BSplineCurve) result = new Geom_BSplineCurve(poles, occKnots, occMults, max_degree, false);
    return result;
}

size_t CTiglPointsToBSplineInterpolation::nContiuityConditions() const
{
    if (!m_C2Continuous) {
        return 0;
    }

    unsigned int max_degree = std::min(m_pnts->Length(), m_degree);

    // we allow at maximum c2 continuity
    unsigned int contin_cons = std::min(max_degree - 1, 2u);

    return contin_cons;
}

math_Matrix CTiglPointsToBSplineInterpolation::getContinuityMatrix(int nCtrPnts, const std::vector<double>& params, const TColStd_Array1OfReal& flatKnots) const
{
    unsigned int max_degree = std::min(m_pnts->Length(), m_degree);

    unsigned int contin_cons = nContiuityConditions();

    math_Matrix continuity_entries(1, contin_cons, 1, nCtrPnts);
    continuity_entries.Init(0.);
    TColStd_Array1OfReal continuity_params1(params[0], 1, 1);
    TColStd_Array1OfReal continuity_params2(params[params.size() - 1], 1, 1);

    if (contin_cons >= 1) {
        // Set C1 condition
        math_Matrix diff1_1 = CTiglBSplineAlgorithms::bsplineBasisMat(max_degree, flatKnots, continuity_params1, 1);
        math_Matrix diff1_2 = CTiglBSplineAlgorithms::bsplineBasisMat(max_degree, flatKnots, continuity_params2, 1);
        continuity_entries.Set(1, 1, 1, nCtrPnts, diff1_1 - diff1_2);
    }

    if (contin_cons >= 2) {
        // Set C2 consition
        math_Matrix diff2_1 = CTiglBSplineAlgorithms::bsplineBasisMat(max_degree, flatKnots, continuity_params1, 2);
        math_Matrix diff2_2 = CTiglBSplineAlgorithms::bsplineBasisMat(max_degree, flatKnots, continuity_params2, 2);
        continuity_entries.Set(2, 2, 1, nCtrPnts, diff2_1 - diff2_2);
    }
    return continuity_entries;
}

double CTiglPointsToBSplineInterpolation::maxDistanceOfBoundingBox(const TColgp_Array1OfPnt& points) const
{
    double distance;
    double maxDistance = 0.;
    for (int i = points.Lower(); i <= points.Upper(); ++i) {
        for (int j = points.Lower(); j <= points.Upper(); ++j) {
            distance = points.Value(i).Distance(points.Value(j));
            if (maxDistance < distance) {
                maxDistance = distance;
            }
        }
    }
    return maxDistance;
}

bool CTiglPointsToBSplineInterpolation::isClosed() const
{
    double maxDistance = maxDistanceOfBoundingBox(m_pnts->Array1());
    double error = 1e-12*maxDistance;
    return m_pnts->Value(m_pnts->Lower()).IsEqual(m_pnts->Value(m_pnts->Upper()), error);
}

CTiglPointsToBSplineInterpolation::operator Handle(Geom_BSplineCurve)() const
{
    return Curve();
}

} // namespace tigl
