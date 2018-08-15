/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2019-04-24 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglBSplineApproxInterp.h"

#include <CTiglError.h>
#include <CTiglBSplineAlgorithms.h>

#include <TColgp_Array1OfPnt.hxx>
#include <Geom_BSplineCurve.hxx>

#include <algorithm>
#include <BSplCLib.hxx>
#include <math_Matrix.hxx>
#include <math_Gauss.hxx>

namespace
{

class helper_function_find
{
public:
    helper_function_find(double val, double tolerance = 1e-15)
        : _val(val), _tol(tolerance)
    {}

    // helper function for std::unique
    bool operator()(double a)
    {
        return (fabs(_val - a) < _tol);
    }
private:
    double _val;
    double _tol;
};

void insertKnot(double knot, int count, int degree, std::vector<double>& knots, std::vector<int>& mults, double tol = 1e-5)
{
    if (knot < knots.front() || knot > knots.back()) {
        throw tigl::CTiglError("knot out of range");
    }

    size_t pos = std::find_if(knots.begin(), knots.end(), helper_function_find(knot, tol)) - knots.begin();
    if (pos >= knots.size()) {
        // knot not found, insert new one
        pos = 0;
        while (knots[pos] < knot) {
            pos++;
        }
        knots.insert(knots.begin() + pos, knot);
        mults.insert(mults.begin() + pos, std::min(count, degree));
    }
    else {
        // knot found, increase multiplicity
        mults[pos] = std::min(mults[pos] + count, degree);
    }
}

}

namespace tigl
{

/**
 * @brief Copies the curve points
 */
CTiglBSplineApproxInterp::CTiglBSplineApproxInterp(const TColgp_Array1OfPnt& points, int nControlPoints, int degree, bool continuous_if_closed)
    : m_pnts(1, points.Length())
    , m_indexOfApproximated(static_cast<size_t>(points.Length()))
    , m_degree(degree)
    , m_ncp(nControlPoints)
    , m_C2Continuous(continuous_if_closed)
{
    for (Standard_Integer i = 0; i < points.Length(); ++i) {
        size_t idx = static_cast<size_t>(i);
        m_indexOfApproximated[idx] = idx;
        m_pnts.SetValue(i + 1, points.Value(points.Lower() + i));
    }
}

void CTiglBSplineApproxInterp::InterpolatePoint(size_t pointIndex, bool withKink)
{
    std::vector<size_t>::iterator it = std::find(m_indexOfApproximated.begin(), m_indexOfApproximated.end(), pointIndex);
    if (it == m_indexOfApproximated.end()) {
        throw CTiglError("Invalid index in CTiglBSplineApproxInterp::InterpolatePoint", TIGL_INDEX_ERROR);
    }
    m_indexOfApproximated.erase(it);

    m_indexOfInterpolated.push_back(pointIndex);
    if (withKink) {
        m_indexOfKinks.push_back(pointIndex);
    }
}

double CTiglBSplineApproxInterp::maxDistanceOfBoundingBox(const TColgp_Array1OfPnt& points) const
{
    gp_Pnt max(-DBL_MAX, -DBL_MAX, -DBL_MAX);
    gp_Pnt min(DBL_MAX, DBL_MAX, DBL_MAX);
    for (int i = points.Lower(); i <= points.Upper(); ++i) {
        const gp_Pnt& p = points.Value(i);
        max.SetX(std::max(max.X(), p.X()));
        max.SetY(std::max(max.Y(), p.Y()));
        max.SetZ(std::max(max.Z(), p.Z()));

        min.SetX(std::min(min.X(), p.X()));
        min.SetY(std::min(min.Y(), p.Y()));
        min.SetZ(std::min(min.Z(), p.Z()));
    }
    return max.Distance(min);
}

bool CTiglBSplineApproxInterp::isClosed() const
{
    if (!m_C2Continuous) {
        return false;
    }
    double maxDistance = maxDistanceOfBoundingBox(m_pnts);
    double error = 1e-12*maxDistance;
    return m_pnts.Value(m_pnts.Lower()).IsEqual(m_pnts.Value(m_pnts.Upper()), error);
}

bool CTiglBSplineApproxInterp::firstAndLastInterpolated() const
{
    bool first = std::find(m_indexOfInterpolated.begin(), m_indexOfInterpolated.end(), 0) != m_indexOfInterpolated.end();
    bool last = std::find(m_indexOfInterpolated.begin(), m_indexOfInterpolated.end(), m_pnts.Length() - 1) != m_indexOfInterpolated.end();
    return first && last;
}

std::vector<double> CTiglBSplineApproxInterp::computeParameters(double alpha) const
{
    double sum = 0.0;
    std::vector<double> t;

    size_t nPoints = static_cast<size_t>(m_pnts.Length());
    t.resize(nPoints);

    t[0] = 0.0;
    // calc total arc length: dt^2 = dx^2 + dy^2
    for (size_t i = 1; i < nPoints; i++) {
        Standard_Integer idx = static_cast<Standard_Integer>(i);
        double len2 = m_pnts.Value(idx).SquareDistance(m_pnts.Value(idx + 1));
        sum += pow(len2, alpha / 2.);
        t[i] = sum;
    }

    // normalize parameter with maximum
    double tmax = t[nPoints - 1];
    for (size_t i = 1; i < nPoints; i++) {
        t[i] /= tmax;
    }

    // reset end value to achieve a better accuracy
    t[nPoints - 1] = 1.0;
    return t;
}

void CTiglBSplineApproxInterp::computeKnots(int ncp, const std::vector<double>& parms, std::vector<double>& knots, std::vector<int>& mults) const
{
    int order = m_degree + 1;
    if (ncp < order) {
        throw CTiglError("Number of control points to small!", TIGL_MATH_ERROR);
    }

    double umin = *std::min_element(parms.begin(), parms.end());
    double umax = *std::max_element(parms.begin(), parms.end());

    knots.resize(static_cast<size_t>(ncp - m_degree + 1));
    mults.resize(static_cast<size_t>(ncp - m_degree + 1));

    // fill multiplicity at start
    knots[0] = umin;
    mults[0] = order;

    // number of knots between the multiplicities
    size_t N = (static_cast<size_t>(ncp - order));
    // set uniform knot distribution
    for (size_t i = 1; i <= N; ++i ) {
        knots[i] = umin + (umax - umin) * double(i) / double(N + 1);
        mults[i] = 1;
    }

    // fill multiplicity at end
    knots[N + 1] = umax;
    mults[N + 1] = order;

    for (std::vector<size_t>::const_iterator it = m_indexOfKinks.begin(); it != m_indexOfKinks.end(); ++it) {
        size_t idx = *it;
        insertKnot(parms[idx], m_degree, m_degree, knots, mults, 1e-4);
    }
}

CTiglApproxResult CTiglBSplineApproxInterp::FitCurve(const std::vector<double>& initialParms) const
{
    std::vector<double> parms;
    // compute initial parameters, if initialParms emtpy
    if (initialParms.empty()) {
        parms = computeParameters(0.5);
    }
    else {
        parms = initialParms;
    }

    if (parms.size() != static_cast<size_t>(m_pnts.Length())) {
        throw CTiglError("Number of parameters don't match number of points");
    }

    // Compute knots from parameters
    std::vector<double> knots;
    std::vector<int> mults;
    computeKnots(m_ncp, parms,
                 knots, mults);

    TColStd_Array1OfInteger occMults(1, static_cast<Standard_Integer>(mults.size()));
    TColStd_Array1OfReal occKnots(1, static_cast<Standard_Integer>(knots.size()));
    for (size_t i = 0; i < knots.size(); ++i) {
        Standard_Integer idx = static_cast<Standard_Integer>(i + 1);
        occKnots.SetValue(idx, knots[i]);
        occMults.SetValue(idx, mults[i]);
    }

    // solve system
    return solve(parms, occKnots, occMults);
}

CTiglApproxResult CTiglBSplineApproxInterp::FitCurveOptimal(const std::vector<double>& initialParms, int maxIter) const
{
    std::vector<double> parms;
    // compute initial parameters, if initialParms emtpy
    if (initialParms.empty()) {
        parms = computeParameters(0.5);
    }
    else {
        parms = initialParms;
    }

    if (parms.size() != static_cast<size_t>(m_pnts.Length())) {
        throw CTiglError("Number of parameters don't match number of points");
    }

    // Compute knots from parameters
    std::vector<double> knots;
    std::vector<int> mults;
    computeKnots(m_ncp, parms,
                 knots, mults);

    TColStd_Array1OfInteger occMults(1, static_cast<Standard_Integer>(mults.size()));
    TColStd_Array1OfReal occKnots(1, static_cast<Standard_Integer>(knots.size()));
    for (size_t i = 0; i < knots.size(); ++i) {
        Standard_Integer idx = static_cast<Standard_Integer>(i + 1);
        occKnots.SetValue(idx, knots[i]);
        occMults.SetValue(idx, mults[i]);
    }

    int iteration = 0;

    // solve system
    CTiglApproxResult result = solve(parms, occKnots, occMults);
    double old_error = result.error * 2.;

    while(result.error > 0 && (old_error - result.error) / std::max(result.error, 1e-6) > 1e-3 && iteration < maxIter) {
        old_error = result.error;

        optimizeParameters(result.curve, parms);
        result = solve(parms, occKnots, occMults);

        iteration++;
    }

    return result;
}

ProjectResult CTiglBSplineApproxInterp::projectOnCurve(const gp_Pnt& pnt, const Handle(Geom_Curve) &curve, double inital_Parm) const
{
    const int maxIter = 10; // maximum No of iterations
    const double eps  = 1.0E-6; // accuracy of arc length parameter

    double t = inital_Parm;

    // newton step
    double dt = 0;

    double f = 0;

    int iter = 0; // iteration counter
    do { // Newton iteration to get a better t parameter

        // Get the derivatives of the spline wrt parameter t
        gp_Vec p   = curve->DN(t, 0);
        gp_Vec dp  = curve->DN(t, 1);
        gp_Vec d2p = curve->DN(t, 2);


        // compute objective function and their derivative
        f = pnt.SquareDistance(p.XYZ());

        double df = (p.XYZ() - pnt.XYZ()).Dot(dp.XYZ());
        double d2f = (p.XYZ() - pnt.XYZ()).Dot(d2p.XYZ()) + dp.SquareMagnitude();

        // newton iterate
        dt = -df / d2f;
        double t_new = t + dt;

        // if parameter out of range reset it to the start value
        if (t_new < curve->FirstParameter() || t_new > curve->LastParameter()) {
            t_new = inital_Parm;
            dt = 0.;
        }
        t = t_new;

        iter++;
    }
    while (fabs(dt) > eps && iter < maxIter);

    return ProjectResult(t, sqrt(f));
}

math_Matrix CTiglBSplineApproxInterp::getContinuityMatrix(int nCtrPnts, int contin_cons, const std::vector<double>& params, const TColStd_Array1OfReal& flatKnots) const
{
    math_Matrix continuity_entries(1, contin_cons, 1, nCtrPnts);
    continuity_entries.Init(0.);
    TColStd_Array1OfReal continuity_params1(params[0], 1, 1);
    TColStd_Array1OfReal continuity_params2(params[params.size() - 1], 1, 1);

    math_Matrix diff1_1 = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, continuity_params1, 1);
    math_Matrix diff1_2 = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, continuity_params2, 1);

    math_Matrix diff2_1 = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, continuity_params1, 2);
    math_Matrix diff2_2 = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, continuity_params2, 2);

    // Set C1 condition
    continuity_entries.Set(1, 1, 1, nCtrPnts, diff1_1 - diff1_2);
    
    // Set C2 consition
    continuity_entries.Set(2, 2, 1, nCtrPnts, diff2_1 - diff2_2);
    if (!firstAndLastInterpolated()) {
        math_Matrix diff0_1 = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, continuity_params1, 0);
        math_Matrix diff0_2 = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, continuity_params2, 0);
        continuity_entries.Set(3, 3, 1, nCtrPnts, diff0_1 - diff0_2);
    }
    return continuity_entries;
}

CTiglApproxResult CTiglBSplineApproxInterp::solve(const std::vector<double>& params, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& mults) const
{

    // compute flat knots to solve system
    Standard_Integer nFlatKnots = BSplCLib::KnotSequenceLength(mults, m_degree, false);
    TColStd_Array1OfReal flatKnots(1, nFlatKnots);
    BSplCLib::KnotSequence(knots, mults, flatKnots);

    Standard_Integer n_apprxmated = static_cast<Standard_Integer>(m_indexOfApproximated.size());
    Standard_Integer n_intpolated = static_cast<Standard_Integer>(m_indexOfInterpolated.size());
    Standard_Integer n_continuityConditions = 0;
    
    bool makeClosed = isClosed();
    
    if (makeClosed) {
        // C0, C1, C2
        n_continuityConditions = 3;
        if (firstAndLastInterpolated()) {
            // Remove C0 as they are already equal by design
            n_continuityConditions--;
        }
    }
    
    // Number of control points required
    Standard_Integer nCtrPnts = flatKnots.Length() - m_degree - 1;

    if (nCtrPnts < n_intpolated + n_continuityConditions || nCtrPnts < m_degree + 1 + n_continuityConditions) {
        throw tigl::CTiglError("Too few control points for curve interpolation!");
    }

    if (n_apprxmated == 0 && nCtrPnts != n_intpolated + n_continuityConditions) {
        throw tigl::CTiglError("Wrong number of control points for curve interpolation!");
    }

    // Build left hand side of the equation
    Standard_Integer n_vars = nCtrPnts + n_intpolated + n_continuityConditions;
    math_Matrix lhs(1, n_vars, 1, n_vars);
    lhs.Init(0.);
    
    // Allocate right hand side
    math_Vector rhsx(1, n_vars);
    math_Vector rhsy(1, n_vars);
    math_Vector rhsz(1, n_vars);

    if (n_apprxmated > 0) {
        // Write b vector. These are the points to be approximated
        TColStd_Array1OfReal appParams(1, n_apprxmated);
        math_Vector bx(1, n_apprxmated);
        math_Vector by(1, n_apprxmated);
        math_Vector bz(1, n_apprxmated);
    
        Standard_Integer appIndex = 1;
        for (std::vector<size_t>::const_iterator it_idx = m_indexOfApproximated.begin(); it_idx != m_indexOfApproximated.end(); ++it_idx) {
            Standard_Integer ipnt = static_cast<Standard_Integer>(*it_idx + 1);
            const gp_Pnt& p = m_pnts.Value(ipnt);
            bx(appIndex) = p.X();
            by(appIndex) = p.Y();
            bz(appIndex) = p.Z();
            appParams(appIndex) = params[*it_idx];
            appIndex++;
        }

        // Solve constrained linear least squares
        // min(Ax - b) s.t. Cx = d
        // Create left hand side block matrix
        // A.T*A  C.T
        // C      0
        math_Matrix A = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, appParams);
        math_Matrix At = A.Transposed();

        lhs.Set(1, nCtrPnts, 1, nCtrPnts, At.Multiplied(A));

        rhsx.Set(1, nCtrPnts, At.Multiplied(bx));
        rhsy.Set(1, nCtrPnts, At.Multiplied(by));
        rhsz.Set(1, nCtrPnts, At.Multiplied(bz));
    }

    if (n_intpolated + n_continuityConditions > 0) {
        // Write d vector. These are the points that should be interpolated as well as the continuity constraints for closed curve
        math_Vector dx(1, n_intpolated + n_continuityConditions, 0.);
        math_Vector dy(1, n_intpolated + n_continuityConditions, 0.);
        math_Vector dz(1, n_intpolated + n_continuityConditions, 0.);
        if(n_intpolated > 0) {
            TColStd_Array1OfReal interpParams(1, n_intpolated);
            Standard_Integer intpIndex = 1;
            for (std::vector<size_t>::const_iterator it_idx = m_indexOfInterpolated.begin(); it_idx != m_indexOfInterpolated.end(); ++it_idx) {
                Standard_Integer ipnt = static_cast<Standard_Integer>(*it_idx + 1);
                const gp_Pnt& p = m_pnts.Value(ipnt);
                dx(intpIndex) = p.X();
                dy(intpIndex) = p.Y();
                dz(intpIndex) = p.Z();
                interpParams(intpIndex) = params[*it_idx];
                intpIndex++;
            }
            math_Matrix C = CTiglBSplineAlgorithms::bsplineBasisMat(m_degree, flatKnots, interpParams);
            math_Matrix Ct = C.Transposed();
            lhs.Set(1, nCtrPnts, nCtrPnts + 1, nCtrPnts + n_intpolated, Ct);
            lhs.Set(nCtrPnts + 1,  nCtrPnts + n_intpolated, 1, nCtrPnts, C);
        }

        // sets the C2 continuity constraints for closed curves on the left hand side if requested
        if (makeClosed) {
            math_Matrix continuity_entries = getContinuityMatrix(nCtrPnts, n_continuityConditions, params, flatKnots);
            lhs.Set(nCtrPnts + n_intpolated + 1, nCtrPnts + n_intpolated + n_continuityConditions, 1, nCtrPnts, continuity_entries);
            lhs.Set(1, nCtrPnts, nCtrPnts + n_intpolated + 1, nCtrPnts + n_intpolated + n_continuityConditions, continuity_entries.Transposed());
        }
        rhsx.Set(nCtrPnts + 1, n_vars, dx);
        rhsy.Set(nCtrPnts + 1, n_vars, dy);
        rhsz.Set(nCtrPnts + 1, n_vars, dz);
    }

    math_Gauss solver(lhs);

    math_Vector cp_x(1, n_vars);
    math_Vector cp_y(1, n_vars);
    math_Vector cp_z(1, n_vars);

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

    CTiglApproxResult result;
    result.curve = new Geom_BSplineCurve(poles, knots, mults, m_degree, false);

    // compute error
    double max_error = 0.;
    for (std::vector<size_t>::const_iterator it_idx = m_indexOfApproximated.begin(); it_idx != m_indexOfApproximated.end(); ++it_idx) {
        Standard_Integer ipnt = static_cast<Standard_Integer>(*it_idx + 1);
        const gp_Pnt& p = m_pnts.Value(ipnt);
        double par = params[*it_idx];

        double error = result.curve->Value(par).Distance(p);
        max_error = std::max(max_error, error);
    }
    result.error = max_error;

    return result;
}

/**
 * @brief Recalculates the curve parameters t_k after the
 * control points are fitted to achieve an even better fit.
 */
void CTiglBSplineApproxInterp::optimizeParameters(const Handle(Geom_Curve)& curve, std::vector<double>& m_t) const
{
    // optimize each parameter by finding it's position on the curve
    for (std::vector<size_t>::const_iterator it_idx = m_indexOfApproximated.begin(); it_idx != m_indexOfApproximated.end(); ++it_idx) {
        size_t idx = *it_idx;

        ProjectResult res = projectOnCurve(m_pnts.Value(static_cast<Standard_Integer>(idx + 1)), curve, m_t[idx]);

        // store optimised parameter
        m_t[idx] = res.parameter;
    }
}

} // namespace tigl
