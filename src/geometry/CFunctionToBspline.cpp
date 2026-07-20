/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-11-17 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CFunctionToBspline.h"

#include "CTiglLogging.h"
#include "CTiglError.h"

#include <math_Matrix.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <gp_Vec.hxx>

#include <vector>

#include <cmath>

namespace
{
    /// just a helper class to store chebychev approximation data
    class ChebSegment
    {
    public:
        ChebSegment(int degree)
            : cx(0,degree), cy(0,degree), cz(0,degree)
        {
        }

        math_Vector cx, cy, cz;
        double umin, umax;
        double error;
    };
    
    /// returns a slice of v, i.e. v[low:high]
    math_Vector subVec(const math_Vector& v, int low, int high)
    {
        math_Vector v2(low, high);
        for (int i = low; i <= high; ++i) {
            v2(i) = v(i);
        }
        return v2;
    }

    /// Angle (radians) below which a knot left at full multiplicity by the join step is
    /// treated as numerical noise rather than a real kink. This is comfortably above the
    /// floating-point-level residuals seen on short, already-converged segments (observed
    /// up to ~0.02 degrees), and comfortably below any genuine kink (this codebase's own
    /// convention elsewhere, CTiglBSplineAlgorithms::getKinkParameters, treats >= 6 degrees
    /// as a real kink for curves).
    const double negligibleKinkAngle = 0.5 * M_PI / 180.0;
}

namespace tigl
{

class FuncAdaptor : public MathFunc1d
{
public:
    enum Direction
    {
        X,
        Y,
        Z
    };

    FuncAdaptor(MathFunc3d& f, Direction dir)
        : _func(f), _dir(dir)
    {
    }

    ~FuncAdaptor() override
    {
    }

    double value(double t) override
    {
        switch (_dir) {
        case X:
            return _func.valueX(t);
        case Y:
            return _func.valueY(t);
        case Z:
            return _func.valueZ(t);
        default:
            return 0.;
        }
    }

private:
    MathFunc3d& _func;
    Direction _dir;
};

/// actual implementation of approximation algorithm
class CFunctionToBspline::CFunctionToBsplineImpl
{
public:
    CFunctionToBsplineImpl(MathFunc3d& func,
                           double umin, double umax,
                           int degree,
                           double tolerance,
                           int maxDepth)
        : _xfunc(func, FuncAdaptor::X),
          _yfunc(func, FuncAdaptor::Y),
          _zfunc(func, FuncAdaptor::Z)
    {
        _umin = umin;
        _umax = umax;
        _degree = degree;
        _maxDepth = maxDepth;
        _tol = tolerance;
        _err = 0.;
    }
    
    Handle(Geom_BSplineCurve) Curve();
    
    /// approximates the curve in the subrange [a, b]
    std::vector<ChebSegment> approxSegment(double a, double b, int depth);
    
    /// smooth concatenation the bspline curves to one curve
    Handle(Geom_BSplineCurve) concatC1(const std::vector<Handle(Geom_BSplineCurve)>& curves);

    /// Relaxes interior knots that were left at full multiplicity (a hard C0 corner) by
    /// concatC1's continuity check purely due to numerical noise, not a real kink - see
    /// negligibleKinkAngle above.
    void relaxNegligibleKinks(Handle(Geom_BSplineCurve)& curve);

    /// members
    FuncAdaptor _xfunc, _yfunc, _zfunc;
    double _umin, _umax, _tol, _err;
    int _maxDepth, _degree;
};

// ---------------- Interfacing class -------------------------- // 

CFunctionToBspline::CFunctionToBspline(MathFunc3d& func,
                                       double umin, double umax,
                                       int degree,
                                       double tolerance,
                                       int maxDepth)
{
    pimpl = new CFunctionToBsplineImpl(func, umin, umax, degree, tolerance, maxDepth);
}

CFunctionToBspline::~CFunctionToBspline()
{
    delete pimpl;
}

Handle(Geom_BSplineCurve) CFunctionToBspline::Curve()
{
    return pimpl->Curve();
}

// ---------------- Implementation class -------------------------- // 

std::vector<ChebSegment> CFunctionToBspline::CFunctionToBsplineImpl::approxSegment(double umin, double umax, int depth)
{
    // to estimate the error, we do a chebycheff approximation at higher
    // degree and evaluate the coefficients
    const int K = _degree + 4;
    const math_Vector cx = cheb_approx(_xfunc, K+1, umin, umax);
    const math_Vector cy = cheb_approx(_yfunc, K+1, umin, umax);
    const math_Vector cz = cheb_approx(_zfunc, K+1, umin, umax);
    
    // estimate error
    double errx=0., erry = 0., errz = 0.;
    for (int i = _degree+1; i < K+1; ++i) {
        errx += fabs(cx(i));
        erry += fabs(cy(i));
        errz += fabs(cz(i));
    }
    const double error = std::sqrt(errx*errx + erry*erry + errz*errz);
    
    if (depth >= _maxDepth && error >= _tol) {
        // maxDepth was hit before the requested tolerance was reached - the segment is
        // accepted anyway, but this silently breaks the tolerance promise. This typically
        // means the function has a locally very high derivative/curvature (e.g. near a
        // sqrt(x)-like singularity) that keeps failing to converge no matter how far the
        // interval is bisected.
        LOG(WARNING) << "CFunctionToBspline: maxDepth (" << _maxDepth << ") reached on ["
                     << umin << ", " << umax << "] with approximation error " << error
                     << " exceeding requested tolerance " << _tol;
    }

    if (error < _tol || depth >= _maxDepth) {
        // we can use this approximation, store to structure
        ChebSegment seg(_degree);
        seg.cx = subVec(cx, 0, _degree);
        seg.cy = subVec(cy, 0, _degree);
        seg.cz = subVec(cz, 0, _degree);
        seg.error = error;
        seg.umin = umin;
        seg.umax = umax;
        std::vector<ChebSegment> list;
        list.push_back(seg);
        return list;
    }
    else {
        // we have to split the range in two parts and do the approximation for each of them
        const double alpha = 0.5;
        std::vector<ChebSegment> list1 = approxSegment(umin, umin + (umax-umin)*alpha, depth + 1);
        std::vector<ChebSegment> list2 = approxSegment(umin + (umax-umin)*alpha, umax, depth + 1);
        // combine lists
        list1.insert(list1.end(), list2.begin(), list2.end());
        return list1;
    }
    
}

Handle(Geom_BSplineCurve) CFunctionToBspline::CFunctionToBsplineImpl::Curve()
{
    bool interpolate = true;
    
    std::vector<ChebSegment> segments = approxSegment(_umin, _umax, 1);
    
    int N = _degree + 1;
    math_Matrix Mt = monimial_to_bezier(N)*cheb_to_monomial(N);
    
    // get estimated error and create bspline segments
    std::vector<Handle(Geom_BSplineCurve)> curves;
    double errTotal = 0.;
    std::vector<ChebSegment>::iterator it = segments.begin();
    for (; it != segments.end(); ++it) {
        // get control points
        ChebSegment& seg = *it;
        
        math_Vector cpx = Mt*seg.cx;
        math_Vector cpy = Mt*seg.cy;
        math_Vector cpz = Mt*seg.cz;
        
        TColgp_Array1OfPnt cp(1,cpx.Length());
        for (int i = 1; i <= cpx.Length(); ++i) {
            gp_Pnt p(cpx(i-1), cpy(i-1), cpz(i-1));
            cp.SetValue(i, p);
        }
        
        if (interpolate) {
            gp_Pnt pstart(_xfunc.value(seg.umin), _yfunc.value(seg.umin), _zfunc.value(seg.umin));
            gp_Pnt pstop (_xfunc.value(seg.umax), _yfunc.value(seg.umax), _zfunc.value(seg.umax));
            cp.SetValue(1, pstart);
            cp.SetValue(cpx.Length(), pstop);
        }
        
        // create knots and multiplicity vector
        TColStd_Array1OfReal knots(1,2);
        knots.SetValue(1, seg.umin);
        knots.SetValue(2, seg.umax);
        TColStd_Array1OfInteger mults(1,2);
        mults.SetValue(1, _degree+1);
        mults.SetValue(2, _degree+1);
        
        Handle(Geom_BSplineCurve) curve = new Geom_BSplineCurve(cp, knots, mults, _degree);
        curves.push_back(curve);
        
        if (seg.error > errTotal) {
            errTotal = seg.error;
        }
    }
    _err = errTotal;

    // concatenate c1 the bspline curves
    Handle(Geom_BSplineCurve) result = concatC1(curves);
    relaxNegligibleKinks(result);

#ifdef DEBUG
    LOG(INFO) << "Result of BSpline approximation of function:";
    LOG(INFO) << "    approximation error = " << errTotal;
    LOG(INFO) << "    number of control points = " << result->NbPoles();
    LOG(INFO) << "    number of segments = " << curves.size();
#endif
    return result;
}

double CFunctionToBspline::ApproxError()
{
    return pimpl->_err;
}

Handle(Geom_BSplineCurve) CFunctionToBspline::CFunctionToBsplineImpl::concatC1(const std::vector<Handle(Geom_BSplineCurve)>& curves)
{
    if (curves.size() == 0) {
        return NULL;
    }
    else if (curves.size() == 1) {
        return curves[0];
    }

    // Join the independently fitted segments into a single B-spline, letting OCCT reduce
    // the knot multiplicity at each join as far as the actual continuity of the control
    // points allows (checked against _tol via Geom_BSplineCurve::RemoveKnot), instead of
    // blindly declaring a multiplicity that isn't backed by the data. Where two segments'
    // tangents genuinely disagree beyond _tol (e.g. near a function with unbounded
    // curvature), this correctly leaves a real C0 join there rather than hiding it.
    //
    // WithRatio=False keeps each segment's original parameter range instead of
    // reparametrizing to a uniform parametrization - callers rely on the resulting curve's
    // parameter matching the original function's parameter.
    GeomConvert_CompCurveToBSplineCurve joiner(curves[0]);
    for (size_t i = 1; i < curves.size(); ++i) {
        if (!joiner.Add(curves[i], _tol, Standard_True, Standard_False)) {
            throw CTiglError("CFunctionToBspline: failed to join curve segments");
        }
    }
    return joiner.BSplineCurve();
}

void CFunctionToBspline::CFunctionToBsplineImpl::relaxNegligibleKinks(Handle(Geom_BSplineCurve)& curve)
{
    if (curve.IsNull()) {
        return;
    }

    const double eps = 1e-7;
    for (int i = 2; i < curve->NbKnots(); ++i) {
        if (curve->Multiplicity(i) != curve->Degree()) {
            // already smoother than a hard corner - nothing to do
            continue;
        }

        double knot = curve->Knot(i);
        gp_Pnt pLeft, pRight;
        gp_Vec dLeft, dRight;
        curve->D1(knot - eps, pLeft, dLeft);
        curve->D1(knot + eps, pRight, dRight);

        if (dLeft.Angle(dRight) < negligibleKinkAngle) {
            // the tangent mismatch here is negligible - concatC1's join tolerance just
            // wasn't loose enough to reduce the multiplicity itself (typically because
            // the reconstruction is numerically sensitive for a short segment). We've
            // already verified the angle is tiny, so relax to C1 with a generous
            // tolerance rather than leaving a knot that downstream code (e.g. surface
            // kink detection, which - unlike the curve version - does not check the
            // angle) would otherwise treat as a real, hard corner.
            int nKnotsBefore = curve->NbKnots();
            curve->RemoveKnot(i, curve->Degree() - 1, 1.0);
            if (curve->NbKnots() < nKnotsBefore) {
                // the knot was fully removed (only possible if Degree()-1 == 0) - account
                // for the index shift
                --i;
            }
        }
    }
}

} // namespace tigl
