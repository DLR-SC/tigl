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

#include <math_Matrix.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <GeomConvert.hxx>

#include <vector>

#include <cassert>
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
}

namespace tigl
{

/// actual implementation of approximation algorithm
class CFunctionToBspline::CFunctionToBsplineImpl
{
public:
    CFunctionToBsplineImpl(MathFunc x, MathFunc y, MathFunc z, void* obj,
                           double umin, double umax,
                           int degree,
                           double tolerance,
                           int maxDepth)
        : _xfunc(x), _yfunc(y), _zfunc(z), _obj(obj)
    {
        _umin = umin;
        _umax = umax;
        _degree = degree;
        _maxDepth = maxDepth;
        _tol = tolerance;
        _err = 0.;
    }
    
    Handle_Geom_BSplineCurve Curve();
    
    /// approximates the curve in the subrange [a, b]
    std::vector<ChebSegment> approxSegment(double a, double b, int depth);
    
    /// smooth concatenation the bspline curves to one curve
    Handle_Geom_BSplineCurve concatC1(const std::vector<Handle_Geom_BSplineCurve>& curves);
    
    /// members
    MathFunc _xfunc, _yfunc, _zfunc;
    void* _obj;
    double _umin, _umax, _tol, _err;
    int _maxDepth, _degree;
};

// ---------------- Interfacing class -------------------------- // 

CFunctionToBspline::CFunctionToBspline(MathFunc x, MathFunc y, MathFunc z, void* obj,
                                       double umin, double umax,
                                       int degree,
                                       double tolerance,
                                       int maxDepth)
{
    pimpl = new CFunctionToBsplineImpl(x, y, z, obj, umin, umax, degree, tolerance, maxDepth);
}

CFunctionToBspline::~CFunctionToBspline()
{
    delete pimpl;
}

Handle_Geom_BSplineCurve CFunctionToBspline::Curve()
{
    return pimpl->Curve();
}

// ---------------- Implementation class -------------------------- // 

std::vector<ChebSegment> CFunctionToBspline::CFunctionToBsplineImpl::approxSegment(double umin, double umax, int depth)
{
    // to estimate the error, we do a chebycheff approximation at higher
    // degree and evaluate the coefficients
    const int K = _degree + 4;
    const math_Vector cx = cheb_approx(_xfunc, _obj, K+1, umin, umax);
    const math_Vector cy = cheb_approx(_yfunc, _obj, K+1, umin, umax);
    const math_Vector cz = cheb_approx(_zfunc, _obj, K+1, umin, umax);
    
    // estimate error
    double errx=0., erry = 0., errz = 0.;
    for (int i = _degree+1; i < K+1; ++i) {
        errx += fabs(cx(i));
        erry += fabs(cy(i));
        errz += fabs(cz(i));
    }
    const double error = std::sqrt(errx*errx + erry*erry + errz*errz);
    
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

Handle_Geom_BSplineCurve CFunctionToBspline::CFunctionToBsplineImpl::Curve()
{
    bool interpolate = true;
    
    std::vector<ChebSegment> segments = approxSegment(_umin, _umax, 1);
    
    int N = _degree + 1;
    math_Matrix Mt = monimial_to_bezier(N)*cheb_to_monomial(N);
    
    // get estimated error and create bspline segments
    std::vector<Handle_Geom_BSplineCurve> curves;
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
            gp_Pnt pstart(_xfunc(seg.umin, _obj), _yfunc(seg.umin, _obj), _zfunc(seg.umin, _obj));
            gp_Pnt pstop (_xfunc(seg.umax, _obj), _yfunc(seg.umax, _obj), _zfunc(seg.umax, _obj));
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
        
        Handle_Geom_BSplineCurve curve = new Geom_BSplineCurve(cp, knots, mults, _degree);
        curves.push_back(curve);
        
        if (seg.error > errTotal) {
            errTotal = seg.error;
        }
    }
    _err = errTotal;
     
    // concatenate c1 the bspline curves
    Handle_Geom_BSplineCurve result = concatC1(curves);

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

Handle_Geom_BSplineCurve CFunctionToBspline::CFunctionToBsplineImpl::concatC1(const std::vector<Handle_Geom_BSplineCurve>& curves)
{
    if (curves.size() == 0) {
        return NULL;
    }
    else if (curves.size() == 1) {
        return curves[0];
    }

#ifdef DEBUG
    // check range connectivities
    for (size_t i = 1; i < curves.size(); ++i) {
        Handle_Geom_BSplineCurve lastCurve = curves[i-1];
        Handle_Geom_BSplineCurve thisCurve = curves[i];
        assert(lastCurve->LastParameter() == thisCurve->FirstParameter());
    }
#endif

    // count control points
    int ncp = 2;
    int nkn = 1;
    std::vector<Handle_Geom_BSplineCurve>::const_iterator curveIt;
    for (curveIt = curves.begin(); curveIt != curves.end(); ++curveIt) {
        Handle_Geom_BSplineCurve curve = *curveIt;
        ncp += curve->NbPoles() - 2;
        nkn += curve->NbKnots() - 1;
    }

    // allocate arrays
    TColgp_Array1OfPnt      cpoints(1, ncp);
    TColStd_Array1OfReal    knots(1, nkn);
    TColStd_Array1OfInteger mults(1, nkn);

    int iknotT = 1, imultT = 1, icpT = 1;
    int icurve = 0;
    for (curveIt = curves.begin(); curveIt != curves.end(); ++curveIt, ++icurve) {
        Handle_Geom_BSplineCurve curve = *curveIt;

        // special handling of the first knot, control point
        knots.SetValue(iknotT++, curve->Knot(1));
        if (icurve == 0) {
            // we just copy the data of the very first point/knot
            mults.SetValue(imultT++, curve->Multiplicity(1));
            cpoints.SetValue(icpT++, curve->Pole(1));
        }
        else {
            // set multiplicity to maxDegree to allow c0 concatenation
            mults.SetValue(imultT++, _degree-1);
            // omit the first control points of the current curve
        }

        // just copy control points, weights, knots and multiplicites
        for (int iknot = 2; iknot < curve->NbKnots(); ++iknot) {
            knots.SetValue(iknotT++, curve->Knot(iknot));
            mults.SetValue(imultT++, curve->Multiplicity(iknot));
        }
        for (int icp = 2; icp < curve->NbPoles(); ++icp) {
            cpoints.SetValue(icpT++, curve->Pole(icp));
        }
    }

    // special handling of the last point and knot
    Handle_Geom_BSplineCurve lastCurve = curves[curves.size()-1];
    knots.SetValue(iknotT, lastCurve->Knot(lastCurve->NbKnots()));
    mults.SetValue(imultT,  lastCurve->Multiplicity(lastCurve->NbKnots()));
    cpoints.SetValue(icpT, lastCurve->Pole(lastCurve->NbPoles()));

    Handle_Geom_BSplineCurve result = new Geom_BSplineCurve(cpoints, knots, mults, _degree);
    return result;
}

} // namespace tigl
