/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-05-24 Merlin Pelz <Merlin.Pelz@dlr.de>
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

#include "CTiglBSplineAlgorithms.h"
#include "CTiglCurveNetworkSorter.h"
#include "CTiglError.h"
#include "CSharedPtr.h"
#include "CTiglBSplineApproxInterp.h"
#include "to_string.h"
#include "tiglcommonfunctions.h"

#include <Standard_Version.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>
#include <Geom2dAPI_Interpolate.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColgp_HArray1OfPnt2d.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColgp_HArray2OfPnt.hxx>
#include <BSplCLib.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Precision.hxx>

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <cassert>


namespace
{

    class helper_function_unique
    {
    public:
        helper_function_unique(double tolerance = 1e-15)
            : _tol(tolerance)
        {}

        // helper function for std::unique
        bool operator()(double a, double b)
        {
            return (fabs(a - b) < _tol);
        }
    private:
        double _tol;
    };
    
    enum SurfAdapterDir
    {
        udir = 0,
        vdir = 1
    };
    
    class SurfAdapterView
    {
    public:
        SurfAdapterView(Handle(Geom_BSplineSurface) surf, SurfAdapterDir dir)
            : _surf(surf), _dir(dir)
        {
        }

        void insertKnot(double knot, int mult, double tolerance=1e-15)
        {
            if (_dir == udir) {
                _surf->InsertUKnot(knot, mult, tolerance, false);
            }
            else {
                _surf->InsertVKnot(knot, mult, tolerance, false);
            }
        }

        double getKnot(int idx) const
        {
            if (_dir == udir) {
                return _surf->UKnot(idx);
            }
            else {
                return _surf->VKnot(idx);
            }
        }

        int getMult(int idx) const
        {
            if (_dir == udir) {
                return _surf->UMultiplicity(idx);
            }
            else {
                return _surf->VMultiplicity(idx);
            }
        }

        int getNKnots() const
        {
            if (_dir == udir) {
                return _surf->NbUKnots();
            }
            else {
                return _surf->NbVKnots();
            }
        }

        int getDegree() const
        {
            if (_dir == udir) {
                return _surf->UDegree();
            }
            else {
                return _surf->VDegree();
            }
        }
        
        void setDir(SurfAdapterDir dir)
        {
            _dir = dir;
        }

        operator const Handle(Geom_BSplineSurface)&() const
        {
            return _surf;
        }
    private:
        Handle(Geom_BSplineSurface) _surf;
        SurfAdapterDir _dir;
    };
    
    class CurveAdapterView
    {
    public:
        CurveAdapterView(Handle(Geom_BSplineCurve) curve)
            : _curve(curve)
        {
        }

        void insertKnot(double knot, int mult, double tolerance=1e-15)
        {
            _curve->InsertKnot(knot, mult, tolerance, false);
        }

        double getKnot(int idx) const
        {
            return _curve->Knot(idx);
        }

        int getMult(int idx) const
        {
            return _curve->Multiplicity(idx);
        }

        int getNKnots() const
        {
            return _curve->NbKnots();
        }

        int getDegree() const
        {
            return _curve->Degree();
        }

        operator const Handle(Geom_BSplineCurve)&() const
        {
            return _curve;
        }
 
    private:
        Handle(Geom_BSplineCurve) _curve;
    };

    template <class SplineAdapter>
    bool haveSameRange(const std::vector<SplineAdapter>& splines_vector, double par_tolerance)
    {
        double begin_param_dir = splines_vector[0].getKnot(1);
        double end_param_dir = splines_vector[0].getKnot(splines_vector[0].getNKnots());
        for (unsigned int spline_idx = 1; spline_idx < splines_vector.size(); ++spline_idx) {
            const SplineAdapter& curSpline = splines_vector[spline_idx];
            double begin_param_dir_surface = curSpline.getKnot(1);
            double end_param_dir_surface = curSpline.getKnot(curSpline.getNKnots());
            if (std::abs(begin_param_dir_surface - begin_param_dir) > par_tolerance || std::abs(end_param_dir_surface - end_param_dir) > par_tolerance) {
                return false;
            }
        }
        return true;
    }

    template <class SplineAdapter>
    bool haveSameDegree(const std::vector<SplineAdapter>& splines)
    {
        int degree = splines[0].getDegree();
        for (unsigned int splineIdx = 1; splineIdx < splines.size(); ++splineIdx) {
            if (splines[splineIdx].getDegree() != degree) {
                return false;
            }
        }
        return true;
    }
    
    template <class SplineAdapter>
    int findKnot(const SplineAdapter& spline, double knot, double tolerance=1e-15)
    {
        for (int curSplineKnotIdx = 1; curSplineKnotIdx <= spline.getNKnots(); ++curSplineKnotIdx) {
            if (std::abs(spline.getKnot(curSplineKnotIdx) - knot) < tolerance) {
                return curSplineKnotIdx;
            }
        }
        return -1;
    }

    /**
     * @brief createCommonKnotsVectorImpl:
     *          Creates a common knot vector in u- or v-direction of the given vector of B-splines
     *          The common knot vector contains all knots in u- or v-direction of all splines with the highest multiplicity of all splines.
     * @param old_splines_vector:
     *          the given vector of B-spline splines that could have a different knot vector in u- or v-direction
     */
    template <class SplineAdapter>
    void makeGeometryCompatibleImpl(std::vector<SplineAdapter>& splines_vector, double par_tolerance)
    {
        // all B-spline splines must have the same parameter range in the chosen direction
        if (!haveSameRange(splines_vector, par_tolerance)) {
            throw tigl::CTiglError("B-splines don't have the same parameter range at least in one direction (u / v) in method createCommonKnotsVectorImpl!", TIGL_MATH_ERROR);
        }

        // all B-spline splines must have the same degree in the chosen direction
        if (!haveSameDegree(splines_vector)) {
            throw tigl::CTiglError("B-splines don't have the same degree at least in one direction (u / v) in method createCommonKnotsVectorImpl!", TIGL_MATH_ERROR);
        }

        // create a vector of all knots in chosen direction (u or v) of all splines
        std::vector<double> resultKnots;
        for (typename std::vector<SplineAdapter>::const_iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            const SplineAdapter& spline = *splineIt;
            for (int knot_idx = 1; knot_idx <= spline.getNKnots(); ++knot_idx) {
                resultKnots.push_back(spline.getKnot(knot_idx));
            }
        }

        // sort vector of all knots in given direction of all splines
        std::sort(resultKnots.begin(), resultKnots.end());

        // delete duplicate knots, so that in all_knots are all unique knots
        resultKnots.erase(std::unique(resultKnots.begin(), resultKnots.end(), helper_function_unique(par_tolerance)), resultKnots.end());

        // find highest multiplicities
        std::vector<int> resultMults(resultKnots.size(), 0);
        for (typename std::vector<SplineAdapter>::const_iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            const SplineAdapter& spline = *splineIt;
            for (unsigned int knotIdx = 0; knotIdx < resultKnots.size(); ++knotIdx) {
                // get multiplicity of current knot in surface
                int splKnotIdx = findKnot(spline, resultKnots[knotIdx], par_tolerance);
                if (splKnotIdx > 0) {
                    resultMults[knotIdx] = std::max(resultMults[knotIdx], spline.getMult(splKnotIdx));
                }
            }
        }

        // now insert missing knots in all splines
        for (typename std::vector<SplineAdapter>::iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            SplineAdapter& spline = *splineIt;
            for (unsigned int knotIdx = 0; knotIdx < resultKnots.size(); ++knotIdx) {
                spline.insertKnot(resultKnots[knotIdx], resultMults[knotIdx], par_tolerance);
            }
        }
    } // makeGeometryCompatibleImpl
    
    template <class OccMatrix, class OccVector, class OccHandleVector>
    OccHandleVector array2GetColumn(const OccMatrix& matrix, int colIndex)
    {
        OccHandleVector colVector =  new OccVector(matrix.LowerRow(), matrix.UpperRow());

        for (int rowIdx = matrix.LowerRow(); rowIdx <= matrix.UpperRow(); ++rowIdx) {
            colVector->SetValue(rowIdx, matrix(rowIdx, colIndex));
        }

        return colVector;
    }
    
    template <class OccMatrix, class OccVector, class OccHandleVector>
    OccHandleVector array2GetRow(const OccMatrix& matrix, int rowIndex)
    {
        OccHandleVector rowVector = new OccVector(matrix.LowerCol(), matrix.UpperCol());
        
        for (int colIdx = matrix.LowerCol(); colIdx <= matrix.UpperCol(); ++colIdx) {
            rowVector->SetValue(colIdx, matrix(rowIndex, colIdx));
        }
        
        return rowVector;
    }

    Handle_TColgp_HArray1OfPnt pntArray2GetColumn(const TColgp_Array2OfPnt& matrix, int colIndex)
    {
        return array2GetColumn<TColgp_Array2OfPnt, TColgp_HArray1OfPnt, Handle_TColgp_HArray1OfPnt>(matrix, colIndex);
    }

    Handle_TColgp_HArray1OfPnt pntArray2GetRow(const TColgp_Array2OfPnt& matrix, int rowIndex)
    {
        return array2GetRow<TColgp_Array2OfPnt, TColgp_HArray1OfPnt, Handle_TColgp_HArray1OfPnt>(matrix, rowIndex);
    }

    void clampBSpline(Handle(Geom_BSplineCurve)& curve)
    {
        if (!curve->IsPeriodic()) {
            return;
        }
        curve->SetNotPeriodic();

        Handle(Geom_Curve) c = new Geom_TrimmedCurve(curve, curve->FirstParameter(), curve->LastParameter());
        curve = GeomConvert::CurveToBSplineCurve(c);
    }
    
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

const double CTiglBSplineAlgorithms::REL_TOL_CLOSED = 1e-8;

bool CTiglBSplineAlgorithms::isUDirClosed(const TColgp_Array2OfPnt& points, double tolerance)
{
    bool uDirClosed = true;
    int ulo = points.LowerRow();
    int uhi = points.UpperRow();
    // check that first row and last row are the same
    for (int v_idx = points.LowerCol(); v_idx <= points.UpperCol(); ++v_idx) {
        gp_Pnt pfirst = points.Value(ulo, v_idx);
        gp_Pnt pLast = points.Value(uhi, v_idx);
        uDirClosed = uDirClosed & pfirst.IsEqual(pLast, tolerance);
    }
    return uDirClosed;
}

bool CTiglBSplineAlgorithms::isVDirClosed(const TColgp_Array2OfPnt& points, double tolerance)
{
    bool vDirClosed = true;
    int vlo = points.LowerCol();
    int vhi = points.UpperCol();
    for (int u_idx = points.LowerRow(); u_idx <= points.UpperRow(); ++u_idx) {
        vDirClosed = vDirClosed & points.Value(u_idx, vlo).IsEqual(points.Value(u_idx, vhi), tolerance);
    }
    return vDirClosed;
}

double CTiglBSplineAlgorithms::scale(const TColgp_Array2OfPnt& points)
{
    double theScale = 0.;
    for (int uidx = points.LowerRow(); uidx <= points.UpperRow(); ++uidx) {
        gp_Pnt pFirst = points.Value(uidx, points.LowerCol());
        for (int vidx = points.LowerCol() + 1; vidx <= points.UpperCol(); ++vidx) {
            double dist = pFirst.Distance(points.Value(uidx, vidx));
            theScale = std::max(theScale, dist);
        }
    }
    return theScale;
}

std::vector<double> CTiglBSplineAlgorithms::computeParamsBSplineCurve(const Handle(TColgp_HArray1OfPnt)& points, const double alpha)
{
    std::vector<double> parameters(points->Length());

    parameters[0] = 0.;

    for (size_t i = 1; i < parameters.size(); ++i) {
        int iArray = static_cast<int>(i) + points->Lower();
        double length = pow(points->Value(iArray).SquareDistance(points->Value(iArray - 1)), alpha / 2.);
        parameters[i] = parameters[i - 1] + length;
    }

    double totalLength = parameters.back();
    for (size_t i = 0; i < parameters.size(); ++i) {
        parameters[i] /= totalLength;
    }

    return parameters;
}

std::pair<std::vector<double>, std::vector<double> >
CTiglBSplineAlgorithms::computeParamsBSplineSurf(const TColgp_Array2OfPnt& points, double alpha)
{
    // first for parameters in u-direction:
    std::vector<double> paramsU(static_cast<size_t>(points.ColLength()), 0.);
    for (int vIdx = points.LowerCol(); vIdx <= points.UpperCol(); ++vIdx) {
        std::vector<double> parameters_u_line = computeParamsBSplineCurve(pntArray2GetColumn(points, vIdx), alpha);

        // average over columns
        for (size_t uIdx = 0; uIdx < parameters_u_line.size(); ++uIdx) {
            paramsU[uIdx] += parameters_u_line[uIdx]/(double)points.RowLength();
        }
    }


    // now for parameters in v-direction:
    std::vector<double> paramsV(static_cast<size_t>(points.RowLength()), 0.);
    for (int uIdx = points.LowerRow(); uIdx <= points.UpperRow(); ++uIdx) {
        std::vector<double> parameters_v_line = computeParamsBSplineCurve(pntArray2GetRow(points, uIdx), alpha);

        // average over rows
        for (size_t vIdx = 0; vIdx < parameters_v_line.size(); ++vIdx) {
            paramsV[vIdx] += parameters_v_line[vIdx]/(double)points.ColLength();
        }
    }

    // put computed parameters for both u- and v-direction in output tuple
    return std::make_pair(paramsU, paramsV);

}


std::vector<Handle(Geom_BSplineCurve)> CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector, double tol)
{
    // TODO: Match parameter range

    // Create a copy that we can modify
    std::vector<CurveAdapterView> splines_adapter;
    for (size_t i = 0; i < splines_vector.size(); ++i) {
        splines_adapter.push_back(Handle(Geom_BSplineCurve)::DownCast(splines_vector[i]->Copy()));
    }

    makeGeometryCompatibleImpl(splines_adapter, tol);

    return std::vector<Handle(Geom_BSplineCurve)>(splines_adapter.begin(), splines_adapter.end());
}

std::vector<Handle(Geom_BSplineSurface) > CTiglBSplineAlgorithms::createCommonKnotsVectorSurface(const std::vector<Handle(Geom_BSplineSurface) >& old_surfaces_vector)
{
    // all B-spline surfaces must have the same parameter range in u- and v-direction
    // TODO: Match parameter range

    // Create a copy that we can modify
    std::vector<SurfAdapterView> adapterSplines;
    for (size_t i = 0; i < old_surfaces_vector.size(); ++i) {
        adapterSplines.push_back(SurfAdapterView(Handle(Geom_BSplineSurface)::DownCast(old_surfaces_vector[i]->Copy()), udir));
    }

    // first in u direction
    makeGeometryCompatibleImpl(adapterSplines, 1e-15);

    for (size_t i = 0; i < old_surfaces_vector.size(); ++i) adapterSplines[i].setDir(vdir);

    // now in v direction
    makeGeometryCompatibleImpl(adapterSplines, 1e-15);

    return std::vector<Handle(Geom_BSplineSurface)>(adapterSplines.begin(), adapterSplines.end());
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::curvesToSurface(const std::vector<Handle(Geom_BSplineCurve) >& curves,
                                                                    const std::vector<double>& vParameters, bool continuousIfClosed)
{
    // check amount of given parameters
    if (vParameters.size() != curves.size()) {
        throw CTiglError("The amount of given parameters has to be equal to the amount of given B-splines!", TIGL_MATH_ERROR);
    }

    // check if all curves are closed
    double tolerance = scale(curves) * REL_TOL_CLOSED;
    bool makeClosed = continuousIfClosed & curves.front()->IsEqual(curves.back(), tolerance);

    matchDegree(curves);
    size_t nCurves = curves.size();

    // create a common knot vector for all splines
    std::vector<Handle(Geom_BSplineCurve) > compatSplines = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(curves, 1e-15);

    const Handle(Geom_BSplineCurve)& firstCurve = compatSplines[0];
    size_t numControlPointsU = firstCurve->NbPoles();

    int degreeV = 0;
    int degreeU = firstCurve->Degree();
    Handle(TColStd_HArray1OfReal) knotsV;
    Handle(TColStd_HArray1OfInteger) multsV;

    size_t nPointsAdapt = makeClosed ? nCurves - 1 : nCurves;

    // create matrix of new control points with size which is possibly DIFFERENT from the size of controlPoints
    Handle(TColgp_HArray2OfPnt) cpSurf;
    Handle(TColgp_HArray1OfPnt) interpPointsVDir = new TColgp_HArray1OfPnt(1, static_cast<Standard_Integer>(nPointsAdapt));

    // now continue to create new control points by interpolating the remaining columns of controlPoints in Skinning direction (here v-direction) by B-splines
    for (int cpUIdx = 1; cpUIdx <= numControlPointsU; ++cpUIdx) {
        for (int cpVIdx = 1; cpVIdx <= nPointsAdapt; ++cpVIdx) {
            interpPointsVDir->SetValue(cpVIdx, compatSplines[cpVIdx - 1]->Pole(cpUIdx));
        }
        GeomAPI_Interpolate interpolationObject(interpPointsVDir, toArray(vParameters), makeClosed, 1e-5);
        interpolationObject.Perform();

        // check that interpolation was successful
        if (!interpolationObject.IsDone()) {
            throw CTiglError("Error computing skinning surface", TIGL_MATH_ERROR);
        }

        Handle(Geom_BSplineCurve) interpSpline = interpolationObject.Curve();

        if (makeClosed) {
            clampBSpline(interpSpline);
        }

        if (cpUIdx == 1) {
            degreeV = interpSpline->Degree();
            knotsV = new TColStd_HArray1OfReal(1, interpSpline->NbKnots());
            interpSpline->Knots(knotsV->ChangeArray1());
            multsV = new TColStd_HArray1OfInteger(1, interpSpline->NbKnots());
            interpSpline->Multiplicities(multsV->ChangeArray1());
            cpSurf = new TColgp_HArray2OfPnt(1, static_cast<Standard_Integer>(numControlPointsU), 1, interpSpline->NbPoles());
        }

        // the final surface control points are the control points resulting from
        // the interpolation
        for (int i = cpSurf->LowerCol(); i <= cpSurf->UpperCol(); ++i) {
            cpSurf->SetValue(cpUIdx, i, interpSpline->Pole(i));
        }

        // check degree always the same
        assert(degreeV == interpSpline->Degree());
    }

    TColStd_Array1OfReal knotsU(1, firstCurve->NbKnots());
    firstCurve->Knots(knotsU);
    TColStd_Array1OfInteger multsU(1, firstCurve->NbKnots());
    firstCurve->Multiplicities(multsU);

    Handle(Geom_BSplineSurface) skinnedSurface = new Geom_BSplineSurface(cpSurf->Array2(),
                                                                         knotsU, knotsV->Array1(),
                                                                         multsU, multsV->Array1(),
                                                                         degreeU, degreeV);

    return skinnedSurface;
}

void CTiglBSplineAlgorithms::matchDegree(const std::vector<Handle(Geom_BSplineCurve) >& bsplines)
{
    int maxDegree = 0;
    for (std::vector<Handle(Geom_BSplineCurve) >::const_iterator it = bsplines.begin(); it != bsplines.end(); ++it) {
        int curDegree = (*it)->Degree();
        if (curDegree > maxDegree) {
            maxDegree = curDegree;
        }
    }

    for (std::vector<Handle(Geom_BSplineCurve) >::const_iterator it = bsplines.begin(); it != bsplines.end(); ++it) {
        int curDegree = (*it)->Degree();
        if (curDegree < maxDegree) {
            (*it)->IncreaseDegree(maxDegree);
        }
    }
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::curvesToSurface(const std::vector<Handle(Geom_BSplineCurve) >& bsplCurves, bool continuousIfClosed)
{
    matchDegree(bsplCurves);

    // create a common knot vector for all splines
    std::vector<Handle(Geom_BSplineCurve) > compatibleSplines = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(bsplCurves, 1e-15);

    // create a matrix of control points of all B-splines (splines do have the same amount of control points now)
    TColgp_Array2OfPnt controlPoints(1, compatibleSplines[0]->NbPoles(),
                                     1, static_cast<Standard_Integer>(compatibleSplines.size()));

    for (unsigned int spline_idx = 1; spline_idx <= compatibleSplines.size(); ++spline_idx) {
        for (int point_idx = 1; point_idx <= compatibleSplines[0]->NbPoles(); ++point_idx) {
            controlPoints(point_idx, spline_idx) = compatibleSplines[spline_idx - 1]->Pole(point_idx);
        }
    }

    std::pair<std::vector<double>, std::vector<double> > parameters
            = CTiglBSplineAlgorithms::computeParamsBSplineSurf(controlPoints);

    return CTiglBSplineAlgorithms::curvesToSurface(compatibleSplines, parameters.second, continuousIfClosed);
}

Handle(Geom_BSplineCurve) CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(const Handle(Geom_BSplineCurve) spline,
                                                                                         const std::vector<double>& old_parameters,
                                                                                         const std::vector<double>& new_parameters,
                                                                                         size_t n_control_pnts)
{
    if (old_parameters.size() != new_parameters.size()) {
        throw CTiglError("parameter sizes dont match");
    }

    // create a B-spline as a function for reparametrization
    Handle(TColgp_HArray1OfPnt2d) old_parameters_pnts = new TColgp_HArray1OfPnt2d(1, old_parameters.size());
    for (size_t parameter_idx = 0; parameter_idx < old_parameters.size(); ++parameter_idx) {
        int occIdx = static_cast<int>(parameter_idx + 1);
        old_parameters_pnts->SetValue(occIdx, gp_Pnt2d(old_parameters[parameter_idx], 0));
    }

    Geom2dAPI_Interpolate interpolationObject(old_parameters_pnts, toArray(new_parameters), false, 1e-15);
    interpolationObject.Perform();

    // check that interpolation was successful
    if (!interpolationObject.IsDone()) {
        throw CTiglError("Cannot reparametrize", TIGL_MATH_ERROR);
    }

    Handle(Geom2d_BSplineCurve) reparametrizing_spline = interpolationObject.Curve();

    // Create a vector of parameters including the intersection parameters
    std::vector<double> breaks;
    for (size_t ipar = 1; ipar < new_parameters.size() - 1; ++ipar) {
        breaks.push_back(new_parameters[ipar]);
    }

    double par_tol = 1e-10;

#define MODEL_KINKS
#ifdef MODEL_KINKS
    // remove kinks from breaks
    std::vector<double> kinks = CTiglBSplineAlgorithms::getKinkParameters(spline);
    for (size_t ikink = 0; ikink < kinks.size(); ++ikink) {
        double kink = kinks[ikink];
        std::vector<double>::iterator it = std::find_if(breaks.begin(), breaks.end(), IsInsideTolerance(kink, par_tol));
        if (it != breaks.end()) {
            breaks.erase(it);
        }
    }
#endif

    // create equidistance array of parameters, including the breaks
    std::vector<double> parameters = LinspaceWithBreaks(new_parameters.front(),
                                                        new_parameters.back(),
                                                        std::max(static_cast<size_t>(101), n_control_pnts*2),
                                                        breaks);
#ifdef MODEL_KINKS
    // insert kinks into parameters array at the correct position
    for (size_t ikink = 0; ikink < kinks.size(); ++ikink) {
        double kink = kinks[ikink];
        parameters.insert( 
            std::upper_bound( parameters.begin(), parameters.end(), kink),
            kink);
    }
#endif

    // Compute points on spline at the new parameters
    // Those will be approximated later on
    TColgp_Array1OfPnt points(1, static_cast<Standard_Integer>(parameters.size()));
    for (size_t i = 1; i <= parameters.size(); ++i) {
        double oldParameter = reparametrizing_spline->Value(parameters[i-1]).X();
        points(static_cast<Standard_Integer>(i)) = spline->Value(oldParameter);
    }

    bool makeContinous = spline->IsClosed() &&
            spline->DN(spline->FirstParameter(), 1).Angle(spline->DN(spline->LastParameter(), 1)) < 6. / 180. * M_PI;

    // Create the new spline as a interpolation of the old one
    CTiglBSplineApproxInterp approximationObj(points, static_cast<int>(n_control_pnts), 3, makeContinous);

    breaks.insert(breaks.begin(), new_parameters.front());
    breaks.push_back(new_parameters.back());
    // Interpolate points at breaking parameters (required for gordon surface)
    for (size_t ibreak = 0; ibreak < breaks.size(); ++ibreak) {
        double thebreak = breaks[ibreak];
        size_t idx = static_cast<size_t>(
            std::find_if(parameters.begin(), parameters.end(), IsInsideTolerance(thebreak)) -
            parameters.begin());
        approximationObj.InterpolatePoint(idx);
    }

#ifdef MODEL_KINKS
    for (size_t ikink = 0; ikink < kinks.size(); ++ikink) {
        double kink = kinks[ikink];
        size_t idx = static_cast<size_t>(
            std::find_if(parameters.begin(), parameters.end(), IsInsideTolerance(kink, par_tol)) -
            parameters.begin());
        approximationObj.InterpolatePoint(idx, true);
    }
#endif

    CTiglApproxResult result = approximationObj.FitCurveOptimal(parameters);
    Handle(Geom_BSplineCurve) reparametrized_spline = result.curve;

    assert(!reparametrized_spline.IsNull());

    return reparametrized_spline;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::flipSurface(const Handle(Geom_BSplineSurface) surface)
{
    Handle(Geom_BSplineSurface) result = Handle(Geom_BSplineSurface)::DownCast(surface->Copy());
    result->ExchangeUV();
    return result;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::pointsToSurface(const TColgp_Array2OfPnt& points,
                                                                    const std::vector<double>& uParams,
                                                                    const std::vector<double>& vParams,
                                                                    bool uContinousIfClosed, bool vContinousIfClosed)
{

    double tolerance = REL_TOL_CLOSED * scale(points);
    bool makeVDirClosed = vContinousIfClosed & isVDirClosed(points, tolerance);
    bool makeUDirClosed = uContinousIfClosed & isUDirClosed(points, tolerance);

    // GeomAPI_Interpolate does not want to have the last point,
    // if the curve should be closed. It internally uses the first point
    // as the last point
    int nPointsUpper = makeUDirClosed ? points.UpperRow() -1 : points.UpperRow();

    // first interpolate all points by B-splines in u-direction
    std::vector<Handle(Geom_BSplineCurve)> uSplines;
    for (int cpVIdx = points.LowerCol(); cpVIdx <= points.UpperCol(); ++cpVIdx) {
        Handle_TColgp_HArray1OfPnt points_u = new TColgp_HArray1OfPnt(points.LowerRow(), nPointsUpper);
        for (int iPointU = points_u->Lower(); iPointU <= points_u->Upper(); ++iPointU) {
            points_u->SetValue(iPointU, points.Value(iPointU, cpVIdx));
        }

        GeomAPI_Interpolate interpolationObject(points_u, toArray(uParams), makeUDirClosed, 1e-15);
        interpolationObject.Perform();

        // check that interpolation was successful
        assert(interpolationObject.IsDone());

        Handle(Geom_BSplineCurve) curve = interpolationObject.Curve();

        // for further processing, we have to unperiodise / clamp the curve
        if (makeUDirClosed) {
            clampBSpline(curve);
        }
        uSplines.push_back(curve);
    }

    // now create a skinned surface with these B-splines which represents the interpolating surface
    Handle(Geom_BSplineSurface) interpolatingSurf = CTiglBSplineAlgorithms::curvesToSurface(uSplines, vParams, makeVDirClosed );

    return interpolatingSurf;
}


std::vector<std::pair<double, double> > CTiglBSplineAlgorithms::intersections(const Handle(Geom_BSplineCurve) spline1, const Handle(Geom_BSplineCurve) spline2, double tolerance) {
    // light weight simple minimizer

    // check parametrization of B-splines beforehand

    // find out the average scale of the two B-splines in order to being able to handle a more approximate curves and find its intersections
    double splines_scale = (CTiglBSplineAlgorithms::scale(spline1) + CTiglBSplineAlgorithms::scale(spline2)) / 2.;

    std::vector<std::pair<double, double> > intersection_params_vector;
    GeomAPI_ExtremaCurveCurve intersectionObj(spline1, spline2);
    for (int intersect_idx = 1; intersect_idx <= intersectionObj.NbExtrema(); ++intersect_idx) {
        double param1 = 0.;
        double param2 = 0.;
        intersectionObj.Parameters(intersect_idx, param1, param2);

        // filter out real intersections
        gp_Pnt point1 = spline1->Value(param1);
        gp_Pnt point2 = spline2->Value(param2);

        if (point1.Distance(point2) < tolerance * splines_scale) {
            intersection_params_vector.push_back(std::make_pair(param1, param2));
        }
        else {
            throw CTiglError("Curves do not intersect each other", TIGL_MATH_ERROR);
        }

        // for closed B-splines:
        if (intersectionObj.NbExtrema() == 1 && spline1->IsClosed() && std::abs(param1 - spline1->Knot(1)) < 1e-6) {
            // GeomAPI_ExtremaCurveCurve doesn't find second intersection point at the end of the closed curve, so add it by hand
            intersection_params_vector.push_back(std::make_pair(spline1->Knot(spline1->NbKnots()), param2));
        }

        if (intersectionObj.NbExtrema() == 1 && spline1->IsClosed() && std::abs(param1 - spline1->Knot(spline1->NbKnots())) < 1e-6) {
            // GeomAPI_ExtremaCurveCurve doesn't find second intersection point at the beginning of the closed curve, so add it by hand
            intersection_params_vector.push_back(std::make_pair(spline1->Knot(1), param2));
        }

        if (intersectionObj.NbExtrema() == 1 && spline2->IsClosed() && std::abs(param2 - spline2->Knot(1)) < 1e-6) {
            // GeomAPI_ExtremaCurveCurve doesn't find second intersection point at the end of the closed curve, so add it by hand
            intersection_params_vector.push_back(std::make_pair(param1, spline2->Knot(spline2->NbKnots())));
        }

        if (intersectionObj.NbExtrema() == 1 && spline2->IsClosed() && std::abs(param2 - spline2->Knot(spline2->NbKnots())) < 1e-6) {
            // GeomAPI_ExtremaCurveCurve doesn't find second intersection point at the beginning of the closed curve, so add it by hand
            intersection_params_vector.push_back(std::make_pair(param1, spline2->Knot(1)));
        }
    }


    return intersection_params_vector;
}

double CTiglBSplineAlgorithms::scale(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector)
{
    double maxScale = 0.;
    for (std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = splines_vector.begin(); it != splines_vector.end(); ++it) {
        maxScale = std::max(scale(*it), maxScale);
    }

    return maxScale;
}

double CTiglBSplineAlgorithms::scale(const Handle(Geom_BSplineCurve)& spline)
{
    double scale = 0.;
    gp_Pnt first_ctrl_pnt = spline->Pole(1);
    for (int ctrl_pnt_idx = 2; ctrl_pnt_idx <= spline->NbPoles(); ++ctrl_pnt_idx) {
        // compute distance of the first control point to the others and save biggest distance
        double distance = first_ctrl_pnt.Distance(spline->Pole(ctrl_pnt_idx));

        scale = std::max(scale, distance);
    }
    return scale;
}

void CTiglBSplineAlgorithms::reparametrizeBSpline(Geom_BSplineCurve& spline, double umin, double umax, double tol)
{
    if (std::abs(spline.Knot(1) - umin) > tol || std::abs(spline.Knot(spline.NbKnots()) - umax) > tol) {
        TColStd_Array1OfReal aKnots (1, spline.NbKnots());
        spline.Knots (aKnots);
        BSplCLib::Reparametrize (0., 1., aKnots);
        spline.SetKnots (aKnots);
    }
}

math_Matrix CTiglBSplineAlgorithms::bsplineBasisMat(int degree, const TColStd_Array1OfReal& knots, const TColStd_Array1OfReal& params, unsigned int derivOrder)
{
    Standard_Integer ncp = knots.Length() - degree - 1;
    math_Matrix mx(1, params.Length(), 1, ncp);
    mx.Init(0.);
    math_Matrix bspl_basis(1, derivOrder + 1, 1, degree + 1);
    bspl_basis.Init(0.);
    for (Standard_Integer iparm = 1; iparm <= params.Length(); ++iparm) {
        Standard_Integer basis_start_index = 0;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,1,0)
        BSplCLib::EvalBsplineBasis(derivOrder, degree + 1, knots, params.Value(iparm), basis_start_index, bspl_basis);
#else
        BSplCLib::EvalBsplineBasis(1, derivOrder, degree + 1, knots, params.Value(iparm), basis_start_index, bspl_basis);
#endif
        if(derivOrder > 0) {
            math_Vector help_vector(1, ncp);
            help_vector.Init(0.);
            help_vector.Set(basis_start_index, basis_start_index + degree, bspl_basis.Row(derivOrder + 1));
            mx.SetRow(iparm, help_vector);
        }
        else {
            mx.Set(iparm, iparm, basis_start_index, basis_start_index + degree, bspl_basis);
        }
    }
    return mx;
}

std::vector<double> CTiglBSplineAlgorithms::getKinkParameters(const Handle(Geom_BSplineCurve)& curve)
{
    if (curve.IsNull()) {
        throw CTiglError("Null Pointer curve", TIGL_NULL_POINTER);
    }

    double eps = 1e-8;

    std::vector<double> kinks;
    for (int knotIndex = 2; knotIndex < curve->NbKnots(); ++knotIndex) {
        if (curve->Multiplicity(knotIndex) == curve->Degree()) {
            double knot = curve->Knot(knotIndex);
            // check if really a kink
            double angle = curve->DN(knot + eps, 1).Angle(curve->DN(knot - eps, 1));
            if (angle > 6./180. * M_PI) {
                kinks.push_back(knot);
            }
        }
    }

    return kinks;
}

} // namespace tigl
