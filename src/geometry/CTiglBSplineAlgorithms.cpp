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

#include <Geom2d_BSplineCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
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

    std::vector<double> toVector(const TColStd_Array1OfReal& array)
    {
        std::vector<double> result(static_cast<size_t>(array.Length()));
        int low = array.Lower();
        for (int i = array.Lower(); i <= array.Upper(); ++i) {
            result[static_cast<size_t>(i-low)] = array.Value(i);
        }
        return result;
    }

    void assertRange(const Handle(Geom_Curve)& curve, double umin, double umax, double tol=1e-7)
    {
        if (std::abs(curve->FirstParameter() - umin) > tol || std::abs(curve->LastParameter() - umax) > tol) {
            throw tigl::CTiglError("Curve not in range [" + tigl::std_to_string(umin) + ", " + tigl::std_to_string(umax) + "].");
        }
    }
}


namespace tigl
{


Handle(TColStd_HArray1OfReal) CTiglBSplineAlgorithms::computeParamsBSplineCurve(const Handle(TColgp_HArray1OfPnt)& points, const double alpha)
{
    Handle(TColStd_HArray1OfReal) parameters(new TColStd_HArray1OfReal(points->Lower(), points->Upper()));

    parameters->SetValue(points->Lower(), 0.);

    for (int i = points->Lower() + 1; i <= points->Upper(); ++i) {
        double length = pow(points->Value(i).SquareDistance(points->Value(i - 1)), alpha / 2.);
        parameters->SetValue(i, length + parameters->Value(i-1));
    }


    double totalLength = parameters->Value(parameters->Upper());
    for (int i = parameters->Lower() + 1; i <= parameters->Upper(); ++i) {
         parameters->SetValue(i, parameters->Value(i) / totalLength);
    }

    return parameters;
}

std::pair<Handle(TColStd_HArray1OfReal), Handle(TColStd_HArray1OfReal) >
CTiglBSplineAlgorithms::computeParamsBSplineSurf(const TColgp_Array2OfPnt& points, double alpha)
{
    // first for parameters in u-direction:
    Handle(TColStd_HArray1OfReal) paramsU(new TColStd_HArray1OfReal(points.LowerRow(), points.UpperRow()));
    paramsU->Init(0.);
    for (int vIdx = points.LowerCol(); vIdx <= points.UpperCol(); ++vIdx) {
        Handle(TColStd_HArray1OfReal) parameters_u_line = computeParamsBSplineCurve(pntArray2GetColumn(points, vIdx), alpha);

        // average over columns
        for (int uIdx = parameters_u_line->Lower(); uIdx <= parameters_u_line->Upper(); ++uIdx) {
            double val = paramsU->Value(uIdx) + parameters_u_line->Value(uIdx)/(double)points.RowLength();
            paramsU->SetValue(uIdx, val);
        }
    }


    // now for parameters in v-direction:
    Handle(TColStd_HArray1OfReal) paramsV = new TColStd_HArray1OfReal (points.LowerCol(), points.UpperCol());
    paramsV->Init(0.);
    for (int uIdx = points.LowerRow(); uIdx <= points.UpperRow(); ++uIdx) {
        Handle(TColStd_HArray1OfReal) parameters_v_line = computeParamsBSplineCurve(pntArray2GetRow(points, uIdx), alpha);

        // average over rows
        for (int vIdx = points.LowerCol(); vIdx <= points.UpperCol(); ++vIdx) {
            double val = paramsV->Value(vIdx) + parameters_v_line->Value(vIdx)/(double)points.ColLength();
            paramsV->SetValue(vIdx, val);
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
                                                                    const Handle(TColStd_HArray1OfReal) vParameters)
{
    // check amount of given parameters
    if (vParameters->Length() != curves.size()) {
        throw CTiglError("The amount of given parameters has to be equal to the amount of given B-splines!", TIGL_MATH_ERROR);
    }

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

    // create matrix of new control points with size which is possibly DIFFERENT from the size of controlPoints
    Handle(TColgp_HArray2OfPnt) cpSurf;
    Handle(TColgp_HArray1OfPnt) interpPointsVDir = new TColgp_HArray1OfPnt(1, static_cast<Standard_Integer>(nCurves));

    // now continue to create new control points by interpolating the remaining columns of controlPoints in Skinning direction (here v-direction) by B-splines
    for (int cpUIdx = 1; cpUIdx <= numControlPointsU; ++cpUIdx) {
        for (int cpVIdx = 1; cpVIdx <= nCurves; ++cpVIdx) {
            interpPointsVDir->SetValue(cpVIdx, compatSplines[cpVIdx - 1]->Pole(cpUIdx));
        }
        GeomAPI_Interpolate interpolationObject(interpPointsVDir, vParameters, false, 1e-5);
        interpolationObject.Perform();

        // check that interpolation was successful
        if (!interpolationObject.IsDone()) {
            throw CTiglError("Error computing skinning surface", TIGL_MATH_ERROR);
        }

        Handle(Geom_BSplineCurve) interpSpline = interpolationObject.Curve();
        
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

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::curvesToSurface(const std::vector<Handle(Geom_BSplineCurve) >& bsplCurves)
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

    std::pair<Handle(TColStd_HArray1OfReal), Handle(TColStd_HArray1OfReal) > parameters
            = CTiglBSplineAlgorithms::computeParamsBSplineSurf(controlPoints);

    return CTiglBSplineAlgorithms::curvesToSurface(compatibleSplines, parameters.second);
}

Handle(Geom_BSplineCurve) CTiglBSplineAlgorithms::reparametrizeBSpline(const Handle(Geom_BSplineCurve) spline, const TColStd_Array1OfReal& old_parameters, const TColStd_Array1OfReal& new_parameters)
{
    // stop, if the sizes of old_parameters and new_parameters are not equal
    if (old_parameters.Length() != new_parameters.Length()) {
        throw tigl::CTiglError("The amounts of old and new parameters are not equal!");
    }

    // create a copy of the given B-spline curve to avoid shadow effects
    Handle(Geom_BSplineCurve) copied_spline = Handle(Geom_BSplineCurve)::DownCast(spline->Copy());

    // sort parameter arrays
    std::vector<double> old_parameters_vector(toVector(old_parameters));
    std::vector<double> new_parameters_vector(toVector(new_parameters));

    std::sort(old_parameters_vector.begin(), old_parameters_vector.end());
    std::sort(new_parameters_vector.begin(), new_parameters_vector.end());

    // insert knots at old parameter values, so that one can reparametrize intervals between these old parameter values afterwards
    for (unsigned int i = 0; i < old_parameters_vector.size(); ++i) {
        // if between parameter range of B-spline
        double parm = old_parameters_vector[i];
        if (parm - copied_spline->Knot(1) > 1e-15 && std::abs(copied_spline->Knot(copied_spline->NbKnots()) - parm) > 1e-15) {
            // insert knot degree-times
            CurveAdapterView(copied_spline).insertKnot(parm, copied_spline->Degree(), 1e-7);
        }
    }

    // move knots to reparametrize
    TColStd_Array1OfReal knots(1, copied_spline->NbKnots());
    copied_spline->Knots(knots);
    for (unsigned int interval_idx = 0; interval_idx < old_parameters_vector.size() - 1; ++interval_idx) {
        double interval_beginning_old = old_parameters_vector[interval_idx];
        double interval_end_old = old_parameters_vector[interval_idx + 1];

        double interval_beginning_new = new_parameters_vector[interval_idx];
        double interval_end_new = new_parameters_vector[interval_idx + 1];

        for (int knot_idx = 1; knot_idx <= copied_spline->NbKnots(); ++knot_idx) {
            double knot = copied_spline->Knot(knot_idx);
            if (interval_beginning_old <= knot && knot < interval_end_old) {
                knots(knot_idx) = (knot - interval_beginning_old) / (interval_end_old - interval_beginning_old) * (interval_end_new - interval_beginning_new) + interval_beginning_new;
            }
        }
    }

    // case: knot = upper bound of parameter range
    knots(copied_spline->NbKnots()) = new_parameters_vector[new_parameters_vector.size() - 1];

    // make sure that the new knots are unique
    for (int knot_idx = 2; knot_idx <= knots.Length(); ++knot_idx) {
        if (std::abs(knots(knot_idx - 1) - knots(knot_idx)) < 1e-15) {
            throw tigl::CTiglError("There are duplicate knots in the knot vector!");
        }
    }

    // eliminate small inaccuracy of first knot
    if (std::abs(knots(1)) < 1e-5) {
        knots(1) = 0.;
    }

    // the multiplicity of the knots is not modified by reparametrization itself, but by knot insertion above (degree times):
    copied_spline->SetKnots(knots);

    return copied_spline;
}

Handle(Geom_BSplineCurve) CTiglBSplineAlgorithms::reparametrizeBSplineContinuously(const Handle(Geom_BSplineCurve) spline,
                                                                                   const TColStd_Array1OfReal& old_parameters,
                                                                                   const TColStd_Array1OfReal& new_parameters)
{
    // stop, if the sizes of old_parameters and new_parameters are not equal
    if (old_parameters.Length() != new_parameters.Length()) {
        throw tigl::CTiglError("The amounts of old and new parameters are not equal!");
    }

    // create a copy of the given B-spline curve to avoid shadow effects
    Handle(Geom_BSplineCurve) copied_spline = Handle(Geom_BSplineCurve)::DownCast(spline->Copy());

    // sort parameter arrays
    std::vector<double> old_parameters_vector;
    for (int i = 1; i <= old_parameters.Length(); ++i) {
        old_parameters_vector.push_back(old_parameters(i));
    }

    std::vector<double> new_parameters_vector;
    for (int i = 1; i <= new_parameters.Length(); ++i) {
        new_parameters_vector.push_back(new_parameters(i));
    }

    std::sort(old_parameters_vector.begin(), old_parameters_vector.end());
    std::sort(new_parameters_vector.begin(), new_parameters_vector.end());

    ParametrizingFunction paramFunc(old_parameters, new_parameters);

    // get the flat knots
    TColStd_Array1OfReal knots(1, copied_spline->NbKnots());
    copied_spline->Knots(knots);

    TColStd_Array1OfInteger mults(1, copied_spline->NbKnots());
    copied_spline->Multiplicities(mults);

    int n_flat_knots = 0;
    for (int i = 1; i <= mults.Length(); ++i) {
        n_flat_knots += mults(i);
    }
    TColStd_Array1OfReal flat_knots(1, n_flat_knots);
    BSplCLib::KnotSequence(knots, mults, flat_knots);

    TColgp_Array1OfPnt cp(1, copied_spline->NbPoles());
    copied_spline->Poles(cp);

    TColStd_Array1OfReal new_flat_knots(1, n_flat_knots + 5);
    int u_degree = 3;
    TColgp_Array1OfPnt new_cp(1, copied_spline->NbPoles() + 5);
    int status = 0;

    BSplCLib::FunctionReparameterise(paramFunc, copied_spline->Degree(), flat_knots, cp, new_flat_knots, u_degree, new_cp, status);

    // find out the new knots and their multiplicities
    int n_knots = BSplCLib::KnotsLength(new_flat_knots);

    TColStd_Array1OfReal new_knots(1, n_knots);
    TColStd_Array1OfInteger new_mults(1, n_knots);

    BSplCLib::Knots(new_flat_knots, new_knots, new_mults);

    // create the reparametrized B-spline
    Handle(Geom_BSplineCurve) reparam_spline = new Geom_BSplineCurve(new_cp, new_knots, new_mults, u_degree);


    return reparam_spline;
}

ParametrizingFunction::ParametrizingFunction(const TColStd_Array1OfReal& old_params, const TColStd_Array1OfReal& new_params)
{
    old_parameters = new TColStd_HArray1OfReal(1, old_params.Length());
    new_parameters = new TColStd_HArray1OfReal(1, new_params.Length());
    for (int param_idx = 1; param_idx <= old_params.Length(); ++param_idx) {
        old_parameters->SetValue(param_idx, old_params(param_idx));
        new_parameters->SetValue(param_idx, new_params(param_idx));
    }
}

void ParametrizingFunction::Evaluate(const Standard_Integer theDerivativeRequest, const Standard_Real * theStartEnd, const Standard_Real theParameter,
                                     Standard_Real & theResult, Standard_Integer & theErrorCode) const
{
    Handle(TColgp_HArray1OfPnt2d) points = new TColgp_HArray1OfPnt2d(1, old_parameters->Length());
    for (int param_idx = 1; param_idx <= old_parameters->Length(); ++param_idx) {
        points->SetValue(param_idx, gp_Pnt2d(old_parameters->Value(param_idx), 0));
    }

    Geom2dAPI_Interpolate interpolationObject(points, new_parameters, false, 1e-15);
    interpolationObject.Perform();

    // check that interpolation was successful
    assert(interpolationObject.IsDone());

    Handle(Geom2d_BSplineCurve) curve = interpolationObject.Curve();

    gp_Pnt2d resulting_point = curve->Value(theParameter);
    theResult = resulting_point.X();

}

Handle(Geom_BSplineCurve) CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(const Handle(Geom_BSplineCurve) spline,
                                                                                         const TColStd_Array1OfReal& old_parameters,
                                                                                         const TColStd_Array1OfReal& new_parameters,
                                                                                         unsigned int n_control_pnts)
{
    if (old_parameters.Length() != new_parameters.Length()) {
        throw CTiglError("parameter sizes dont match");
    }

    // create a B-spline as a function for reparametrization
    Handle(TColgp_HArray1OfPnt2d) old_parameters_pnts = new TColgp_HArray1OfPnt2d(1, old_parameters.Length());
    for (int parameter_idx = 1; parameter_idx <= old_parameters.Length(); ++parameter_idx) {
        old_parameters_pnts->SetValue(parameter_idx, gp_Pnt2d(old_parameters(parameter_idx), 0));
    }

    // convert type of new_parameters
    Handle(TColStd_HArray1OfReal) new_parameters_modtype = new TColStd_HArray1OfReal(1, new_parameters.Length());
    for (int parameter_idx = 1; parameter_idx <= new_parameters.Length(); ++parameter_idx) {
        new_parameters_modtype->SetValue(parameter_idx, new_parameters(parameter_idx));
    }
    Geom2dAPI_Interpolate interpolationObject(old_parameters_pnts, new_parameters_modtype, false, 1e-15);
    interpolationObject.Perform();

    // check that interpolation was successful
    if (!interpolationObject.IsDone()) {
        throw CTiglError("Cannot reparametrize", TIGL_MATH_ERROR);
    }

    Handle(Geom2d_BSplineCurve) reparametrizing_spline = interpolationObject.Curve();

    // Create a vector of parameters including the intersection parameters
    std::vector<double> breaks;
    for (Standard_Integer i = new_parameters.Lower() + 1; i < new_parameters.Upper(); ++i) {
        breaks.push_back(new_parameters.Value(i));
    }
    std::vector<double> new_params_std = LinspaceWithBreaks(new_parameters(new_parameters.Lower()),
                                                            new_parameters(new_parameters.Upper()),
                                                            std::max(static_cast<unsigned int>(101), n_control_pnts*2),
                                                            breaks);

    // Compute points on spline at the new parameters
    // Those will be approximated later on
    TColgp_Array1OfPnt points(1, static_cast<Standard_Integer>(new_params_std.size()));
    for (int i = 1; i <= new_params_std.size(); ++i) {
        double oldParameter = reparametrizing_spline->Value(new_params_std[static_cast<Standard_Integer>(i-1)]).X();
        points(i) = spline->Value(oldParameter);
    }

    // Create the new spline as a interpolation of the old one
    CTiglBSplineApproxInterp approximationObj(points, n_control_pnts, 3);

    // Interpolate points at breaking parameters (required for gordon surface)
    for (Standard_Integer iparm = new_parameters.Lower(); iparm <= new_parameters.Upper(); ++iparm) {
        double thebreak = new_parameters.Value(iparm);
        size_t idx = std::find_if(new_params_std.begin(), new_params_std.end(), IsInsideTolerance(thebreak)) - new_params_std.begin();
        approximationObj.InterpolatePoint(idx);
    }

    CTiglApproxResult result = approximationObj.FitCurveOptimal(new_params_std);
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

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::interpolatingSurface(const TColgp_Array2OfPnt& points, const Handle(TColStd_HArray1OfReal) parameters_u, const Handle(TColStd_HArray1OfReal) parameters_v, bool is_closed_u, bool is_closed_v) {

    // first interpolate all points by B-splines in u-direction
    std::vector<Handle(Geom_BSplineCurve)> splines_u_vector;
    for (int cpVIdx = points.LowerCol(); cpVIdx <= points.UpperCol(); ++cpVIdx) {
        GeomAPI_Interpolate interpolationObject(pntArray2GetColumn(points, cpVIdx), parameters_u, false /*is_closed_u*/, 1e-15);
        interpolationObject.Perform();

        // check that interpolation was successful
        assert(interpolationObject.IsDone());

        Handle(Geom_BSplineCurve) curve = interpolationObject.Curve();

        // support for closed B-spline curves
        /*if (curve->IsClosed()) {
            curve->SetNotPeriodic();
        }*/
        splines_u_vector.push_back(curve);
    }

    // now create a skinned surface with these B-splines which represents the interpolating surface
    Handle(Geom_BSplineSurface) interpolatingSurf = CTiglBSplineAlgorithms::curvesToSurface(splines_u_vector, parameters_v);

    return interpolatingSurf;
}

void checkCurveNetworkCompatibility(const std::vector<Handle(Geom_BSplineCurve) >& profiles,
                                    const std::vector<Handle(Geom_BSplineCurve) >& guides,
                                    const std::vector<double>& intersection_params_spline_u,
                                    const std::vector<double>& intersection_params_spline_v,
                                    double tol = 3e-4)
{
    // find out the 'average' scale of the B-splines in order to being able to handle a more approximate dataset and find its intersections
    double splines_scale = 0.5 * (CTiglBSplineAlgorithms::scaleOfBSplines(profiles)+ CTiglBSplineAlgorithms::scaleOfBSplines(guides));

    if (std::abs(intersection_params_spline_u.front()) > splines_scale * tol || std::abs(intersection_params_spline_u.back() - 1.) > splines_scale * tol) {
        throw tigl::CTiglError("WARNING: B-splines in u-direction mustn't stick out, spline network must be 'closed'!");
    }

    if (std::abs(intersection_params_spline_v.front()) > splines_scale * tol || std::abs(intersection_params_spline_v.back() - 1.) > splines_scale * tol) {
        throw tigl::CTiglError("WARNING: B-splines in v-direction mustn't stick out, spline network must be 'closed'!");
    }

    // check compatibilty of network
    for (size_t u_param_idx = 0; u_param_idx < intersection_params_spline_u.size(); ++u_param_idx) {
        double spline_u_param = intersection_params_spline_u[u_param_idx];
        const Handle(Geom_BSplineCurve)& spline_v = guides[u_param_idx];
        for (size_t v_param_idx = 0; v_param_idx < intersection_params_spline_v.size(); ++v_param_idx) {
            const Handle(Geom_BSplineCurve)& spline_u = profiles[v_param_idx];
            double spline_v_param = intersection_params_spline_v[v_param_idx];

            gp_Pnt p_prof = spline_u->Value(spline_u_param);
            gp_Pnt p_guid = spline_v->Value(spline_v_param);
            double distance = p_prof.Distance(p_guid);

            if (distance > splines_scale * tol) {
                throw tigl::CTiglError("B-spline network is incompatible (e.g. wrong parametrization) or intersection parameters are in a wrong order!");
            }
        }
    }
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::createGordonSurface(const std::vector<Handle(Geom_BSplineCurve) >& profiles,
                                                                        const std::vector<Handle(Geom_BSplineCurve) >& guides,
                                                                        const Handle(TColStd_HArray1OfReal) intersection_params_spline_u,
                                                                        const Handle(TColStd_HArray1OfReal) intersection_params_spline_v)
{
    // check whether there are any u-directional and v-directional B-splines in the vectors
    if (profiles.size() < 2) {
        throw CTiglError("There must be at least two profiles for the gordon surface.", TIGL_MATH_ERROR);
    }

    if (guides.size()  < 2) {
        throw CTiglError("There must be at least two guides for the gordon surface.", TIGL_MATH_ERROR);
    }

    // check B-spline parametrization (should be from 0 to 1):
    for (unsigned int profileIdx = 0; profileIdx < profiles.size(); ++profileIdx) {
        assertRange(profiles[profileIdx], 0., 1., 1e-5);
    }

    for (unsigned int guideIdx = 0; guideIdx < guides.size(); ++guideIdx) {
        assertRange(guides[guideIdx], 0., 1., 1e-5);
    }

    // TODO: Do we really need to check compatibility?
    // We don't need to do this, if the curves were reparametrized before
    // In this case, they might be even incompatible, as the curves have been approximated
    checkCurveNetworkCompatibility(profiles, guides,
                                   toVector(intersection_params_spline_u->Array1()),
                                   toVector(intersection_params_spline_v->Array1()));

    // Skinning in v-direction with u directional B-Splines
    Handle(Geom_BSplineSurface) surface_v = curvesToSurface(profiles, intersection_params_spline_v);
    // therefore reparametrization before this method

    // Skinning in u-direction with v directional B-Splines
    Handle(Geom_BSplineSurface) surface_u_unflipped = curvesToSurface(guides, intersection_params_spline_u);

    // flipping of the surface in v-direction; flipping is redundant here, therefore the next line is a comment!
    Handle(Geom_BSplineSurface) surface_u = flipSurface(surface_u_unflipped);

    // setting everything up for creating Tensor Product Surface by interpolating intersection points of profiles and guides with B-Spline surface
    // find the intersection points:
    TColgp_Array2OfPnt intersection_pnts(1, intersection_params_spline_u->Upper(), 1, intersection_params_spline_v->Upper());

    // use splines in u-direction to get intersection points
    for (size_t spline_idx = 0; spline_idx < profiles.size(); ++spline_idx) {
        for (int intersection_idx = intersection_params_spline_u->Lower(); intersection_idx <= intersection_params_spline_u->Upper(); ++intersection_idx) {
            Handle(Geom_BSplineCurve) spline_u = profiles[spline_idx];
            double parameter = intersection_params_spline_u->Value(intersection_idx);
            intersection_pnts(intersection_idx, static_cast<Standard_Integer>(spline_idx + 1)) = spline_u->Value(parameter);
        }
    }

    // if there are too little points for degree in u-direction = 3 and degree in v-direction=3 creating an interpolation B-spline surface isn't possible in Open CASCADE

    // Open CASCADE doesn't have a B-spline surface interpolation method where one can give the u- and v-directional parameters as arguments
    Handle(Geom_BSplineSurface) tensorProdSurf = CTiglBSplineAlgorithms::interpolatingSurface(intersection_pnts, intersection_params_spline_u, intersection_params_spline_v, profiles[0]->IsClosed(), guides[0]->IsClosed());

    // match degree of all three surfaces
    Standard_Integer degreeU = std::max(std::max(surface_u->UDegree(),
                                                 surface_v->UDegree()),
                                                 tensorProdSurf->UDegree());

    Standard_Integer degreeV = std::max(std::max(surface_u->VDegree(),
                                                 surface_v->VDegree()),
                                                 tensorProdSurf->VDegree());

    // check whether degree elevation is necessary (does method elevate_degree_u()) and if yes, elevate degree
    surface_u->IncreaseDegree(degreeU, degreeV);
    surface_v->IncreaseDegree(degreeU, degreeV);
    tensorProdSurf->IncreaseDegree(degreeU, degreeV);

    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector_unmod;
    surfaces_vector_unmod.push_back(surface_u);
    surfaces_vector_unmod.push_back(surface_v);
    surfaces_vector_unmod.push_back(tensorProdSurf);

    // create common knot vector for all three surfaces
    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector = createCommonKnotsVectorSurface(surfaces_vector_unmod);

    assert(surfaces_vector.size() == 3);

    surface_u = surfaces_vector[0];
    surface_v = surfaces_vector[1];
    tensorProdSurf = surfaces_vector[2];

    // creating the Gordon Surface = s_u + s_v - tps
    // We reuse the u surface for storing the result

    for (int cp_u_idx = 1; cp_u_idx <= surface_u->NbUPoles(); ++cp_u_idx) {
        for (int cp_v_idx = 1; cp_v_idx <= surface_u->NbVPoles(); ++cp_v_idx) {
            gp_Pnt cp_surf_u = surface_u->Pole(cp_u_idx, cp_v_idx);
            gp_Pnt cp_surf_v = surface_v->Pole(cp_u_idx, cp_v_idx);
            gp_Pnt cp_tensor = tensorProdSurf->Pole(cp_u_idx, cp_v_idx);

            surface_u->SetPole(cp_u_idx, cp_v_idx, cp_surf_u.XYZ() + cp_surf_v.XYZ() - cp_tensor.XYZ());
        }
    }

    return surface_u;
}

std::vector<std::pair<double, double> > CTiglBSplineAlgorithms::intersections(const Handle(Geom_BSplineCurve) spline1, const Handle(Geom_BSplineCurve) spline2, double tolerance) {
    // light weight simple minimizer

    // check parametrization of B-splines beforehand

    // find out the average scale of the two B-splines in order to being able to handle a more approximate curves and find its intersections
    double splines_scale = (CTiglBSplineAlgorithms::scaleOfBSpline(spline1) + CTiglBSplineAlgorithms::scaleOfBSpline(spline2)) / 2.;

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

double CTiglBSplineAlgorithms::scaleOfBSplines(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector)
{
    double maxScale = 0.;
    for (std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = splines_vector.begin(); it != splines_vector.end(); ++it) {
        maxScale = std::max(scaleOfBSpline(*it), maxScale);
    }

    return maxScale;
}

double CTiglBSplineAlgorithms::scaleOfBSpline(const Handle(Geom_BSplineCurve)& spline)
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

void CTiglBSplineAlgorithms::eliminateInaccuraciesNetworkIntersections(const std::vector<Handle(Geom_BSplineCurve)> & sorted_splines_u, const std::vector<Handle(Geom_BSplineCurve)> & sorted_splines_v, math_Matrix & intersection_params_u, math_Matrix & intersection_params_v) {

    // eliminate small inaccuracies of the intersection parameters:

    // first intersection
    for (unsigned int spline_u_idx = 0; spline_u_idx < sorted_splines_u.size(); ++spline_u_idx) {
        if (std::abs(intersection_params_u(spline_u_idx, 0) - sorted_splines_u[0]->Knot(1)) < 0.001) {
            if (std::abs(sorted_splines_u[0]->Knot(1)) < 1e-10) {
                intersection_params_u(spline_u_idx, 0) = 0;
            }
            else {
                intersection_params_u(spline_u_idx, 0) = sorted_splines_u[0]->Knot(1);
            }
        }
    }

    for (unsigned int spline_v_idx = 0; spline_v_idx < sorted_splines_v.size(); ++spline_v_idx) {
        if (std::abs(intersection_params_v(0, spline_v_idx) - sorted_splines_v[0]->Knot(1)) < 0.001) {
            if (std::abs(sorted_splines_v[0]->Knot(1)) < 1e-10) {
                intersection_params_v(0, spline_v_idx) = 0;
            }
            else {
                intersection_params_v(0, spline_v_idx) = sorted_splines_v[0]->Knot(1);
            }
        }
    }

    // last intersection
    for (unsigned int spline_u_idx = 0; spline_u_idx < sorted_splines_u.size(); ++spline_u_idx) {
        if (std::abs(intersection_params_u(spline_u_idx, sorted_splines_v.size() - 1) - sorted_splines_u[0]->Knot(sorted_splines_u[0]->NbKnots())) < 0.001) {
            intersection_params_u(spline_u_idx, sorted_splines_v.size() - 1) = sorted_splines_u[0]->Knot(sorted_splines_u[0]->NbKnots());
        }
    }

    for (unsigned int spline_v_idx = 0; spline_v_idx < sorted_splines_v.size(); ++spline_v_idx) {
        if (std::abs(intersection_params_v(sorted_splines_u.size() - 1, spline_v_idx) - sorted_splines_v[0]->Knot(sorted_splines_v[0]->NbKnots())) < 0.001) {
            intersection_params_v(sorted_splines_u.size() - 1, spline_v_idx) = sorted_splines_v[0]->Knot(sorted_splines_v[0]->NbKnots());
        }
    }
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

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::createGordonSurfaceGeneral(const std::vector<Handle(Geom_BSplineCurve) >& splines_u_vector_in,
                                                                               const std::vector<Handle(Geom_BSplineCurve) >& splines_v_vector_in)
{

    std::vector<Handle(Geom_BSplineCurve) > profiles(splines_u_vector_in);
    std::vector<Handle(Geom_BSplineCurve) > guides(splines_v_vector_in);

    // check whether there are any u-directional and v-directional B-splines in the vectors
    if (profiles.size() == 0) {
        throw tigl::CTiglError("There are no profile curves!");
    }

    if (guides.size() == 0) {
        throw tigl::CTiglError("There are no guide curves!");
    }

    // reparametrize into [0,1]
    for (unsigned int spline_u_idx = 0; spline_u_idx < profiles.size(); ++spline_u_idx) {
        reparametrizeBSpline(*profiles[spline_u_idx], 0., 1., 1e-15);
    }

    for (unsigned int spline_v_idx = 0; spline_v_idx < guides.size(); ++spline_v_idx) {
        reparametrizeBSpline(*guides[spline_v_idx], 0., 1., 1e-15);
    }
    // now the parameter range of all  profiles and guides is [0, 1]

    // now find all intersections of all B-splines with each other
    math_Matrix intersection_params_u(0, profiles.size() - 1, 0, guides.size() - 1);
    math_Matrix intersection_params_v(0, profiles.size() - 1, 0, guides.size() - 1);

    for (unsigned int spline_u_idx = 0; spline_u_idx < profiles.size(); ++spline_u_idx) {
        for (unsigned int spline_v_idx = 0; spline_v_idx < guides.size(); ++spline_v_idx) {
            std::vector<std::pair<double, double> > intersection_params_vector = CTiglBSplineAlgorithms::intersections(profiles[spline_u_idx], guides[spline_v_idx]);

            if (intersection_params_vector.size() < 1) {
                throw tigl::CTiglError("U-directional B-spline and v-directional B-spline don't intersect each other!");
            }

            else if (intersection_params_vector.size() == 1) {
                intersection_params_u(spline_u_idx, spline_v_idx) = intersection_params_vector[0].first;
                intersection_params_v(spline_u_idx, spline_v_idx) = intersection_params_vector[0].second;
            }
                // for closed curves
            else if (intersection_params_vector.size() == 2) {

                // only the u-directional B-spline curves are closed
                if (profiles[0]->IsClosed()) {

                    if (spline_v_idx == 0) {
                        intersection_params_u(spline_u_idx, spline_v_idx) = std::min(intersection_params_vector[0].first, intersection_params_vector[1].first);
                    }
                    else if (spline_v_idx == guides.size() - 1) {
                        intersection_params_u(spline_u_idx, spline_v_idx) = std::max(intersection_params_vector[0].first, intersection_params_vector[1].first);
                    }

                    // intersection_params_vector[0].second == intersection_params_vector[1].second
                    intersection_params_v(spline_u_idx, spline_v_idx) = intersection_params_vector[0].second;
                }

                // only the v-directional B-spline curves are closed
                if (guides[0]->IsClosed()) {

                    if (spline_u_idx == 0) {
                        intersection_params_v(spline_u_idx, spline_v_idx) = std::min(intersection_params_vector[0].second, intersection_params_vector[1].second);
                    }
                    else if (spline_u_idx == profiles.size() - 1) {
                        intersection_params_v(spline_u_idx, spline_v_idx) = std::max(intersection_params_vector[0].second, intersection_params_vector[1].second);
                    }
                    // intersection_params_vector[0].first == intersection_params_vector[1].first
                    intersection_params_u(spline_u_idx, spline_v_idx) = intersection_params_vector[0].first;
                }

//                // TODO: both u-directional splines and v-directional splines are closed
//               else if (intersection_params_vector.size() == 4) {

//                }
            }

            else if (intersection_params_vector.size() > 2) {
                throw tigl::CTiglError("U-directional B-spline and v-directional B-spline have more than two intersections with each other!");
            }
        }
    }

    // sort intersection_params_u and intersection_params_v and u-directional and v-directional B-spline curves
    tigl::CTiglCurveNetworkSorter sorterObj(std::vector<Handle(Geom_Curve)>(profiles.begin(), profiles.end()),
                                            std::vector<Handle(Geom_Curve)>(guides.begin(), guides.end()),
                                            intersection_params_u,
                                            intersection_params_v);
    sorterObj.Perform();

    // get the sorted matrices and vectors
    intersection_params_u = sorterObj.ProfileIntersectionParms();
    intersection_params_v = sorterObj.GuideIntersectionParms();

    // copy sorted curves back into out curves array
    std::transform(sorterObj.Profiles().begin(), sorterObj.Profiles().end(), profiles.begin(), Handle(Geom_BSplineCurve)::DownCast);
    std::transform(sorterObj.Guides().begin(), sorterObj.Guides().end(), guides.begin(), Handle(Geom_BSplineCurve)::DownCast);

    // eliminate small inaccuracies of the intersection parameters:
    CTiglBSplineAlgorithms::eliminateInaccuraciesNetworkIntersections(profiles, guides, intersection_params_u, intersection_params_v);

    TColStd_Array1OfReal average_intersection_params_u(1, guides.size());
    for (unsigned int spline_v_idx = 1; spline_v_idx <= guides.size(); ++spline_v_idx) {
        double sum = 0;
        for (unsigned int spline_u_idx = 1; spline_u_idx <= profiles.size(); ++spline_u_idx) {
            sum += intersection_params_u(spline_u_idx - 1, spline_v_idx - 1);
        }
        average_intersection_params_u(spline_v_idx) = sum / profiles.size();
    }

    TColStd_Array1OfReal average_intersection_params_v(1, profiles.size());
    for (unsigned int spline_u_idx = 1; spline_u_idx <= profiles.size(); ++spline_u_idx) {
        double sum = 0;
        for (unsigned int spline_v_idx = 1; spline_v_idx <= guides.size(); ++spline_v_idx) {
            sum += intersection_params_v(spline_u_idx - 1, spline_v_idx - 1); // TODO: scientific approach for power
        }
        average_intersection_params_v(spline_u_idx) = sum / guides.size();
    }


    if (average_intersection_params_u(1) > 1e-5 || average_intersection_params_v(1) > 1e-5) {
        throw CTiglError("At least one B-splines has no intersection at the beginning.");
    }

    // Get maximum number of control points to figure out detail of spline
    int max_cp_u = 0, max_cp_v = 0;
    for(std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = profiles.begin(); it != profiles.end(); ++it) {
        max_cp_u = std::max(max_cp_u, (*it)->NbPoles());
    }
    for(std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = guides.begin(); it != guides.end(); ++it) {
        max_cp_v = std::max(max_cp_v, (*it)->NbPoles());
    }
    // we want to use at least 30 control points to be able to reparametrize the geometry properly
    max_cp_u = std::max(30, max_cp_u + 10);
    max_cp_v = std::max(30, max_cp_v + 10);


    // reparametrize u-directional B-splines
    std::vector<Handle(Geom_BSplineCurve)> reparam_splines_u;
    for (unsigned int spline_u_idx = 0; spline_u_idx < profiles.size(); ++spline_u_idx) {

        TColStd_Array1OfReal old_parameters(1, guides.size());
        for (unsigned int spline_v_idx = 1; spline_v_idx <= guides.size(); ++spline_v_idx) {
            old_parameters(spline_v_idx) = intersection_params_u(spline_u_idx, spline_v_idx - 1);
        }

        // eliminate small inaccuracies at the first knot
        if (std::abs(old_parameters(1)) < 1e-5) {
            old_parameters(1) = 0;
        }

        if (std::abs(average_intersection_params_u(1)) < 1e-5) {
            average_intersection_params_u(1) = 0;
        }

        // eliminate small inaccuracies at the last knot
        if (std::abs(old_parameters(old_parameters.Length()) - 1) < 1e-5) {
            old_parameters(old_parameters.Length()) = 1;
        }

        if (std::abs(average_intersection_params_u(average_intersection_params_u.Length()) - 1) < 1e-5) {
            average_intersection_params_u(average_intersection_params_u.Length()) = 1;
        }


        Handle(Geom_BSplineCurve) reparam_spline_u = CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(profiles[spline_u_idx], old_parameters, average_intersection_params_u, max_cp_u);
        reparam_splines_u.push_back(reparam_spline_u);
    }

    // reparametrize v-directional B-splines
    std::vector<Handle(Geom_BSplineCurve)> reparam_splines_v;
    for (unsigned int spline_v_idx = 0; spline_v_idx < guides.size(); ++spline_v_idx) {

        TColStd_Array1OfReal old_parameters(1, profiles.size());
        for (unsigned int spline_u_idx = 1; spline_u_idx <= profiles.size(); ++spline_u_idx) {
            old_parameters(spline_u_idx) = intersection_params_v(spline_u_idx - 1, spline_v_idx);
        }

        // eliminate small inaccuracies at the first knot
        if (std::abs(old_parameters(1)) < 1e-5) {
            old_parameters(1) = 0;
        }

        if (std::abs(average_intersection_params_v(1)) < 1e-5) {
            average_intersection_params_v(1) = 0;
        }

        // eliminate small inaccuracies at the last knot
        if (std::abs(old_parameters(old_parameters.Length()) - 1) < 1e-5) {
            old_parameters(old_parameters.Length()) = 1;
        }

        if (std::abs(average_intersection_params_v(average_intersection_params_v.Length()) - 1) < 1e-5) {
            average_intersection_params_v(average_intersection_params_v.Length()) = 1;
        }

        Handle(Geom_BSplineCurve) reparam_spline_v = CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(guides[spline_v_idx], old_parameters, average_intersection_params_v, max_cp_v);
        reparam_splines_v.push_back(reparam_spline_v);
    }

    // splines are reparametrized, but don't have common knot vector yet, in this sense not compatible

    // convert types of average_intersection_params
    Handle(TColStd_HArray1OfReal) av_intersection_params_u = new TColStd_HArray1OfReal(1, average_intersection_params_u.Length());
    for (int i = 1; i <= average_intersection_params_u.Length(); ++i) {
        av_intersection_params_u->SetValue(i, average_intersection_params_u(i));
    }

    Handle(TColStd_HArray1OfReal) av_intersection_params_v = new TColStd_HArray1OfReal(1, average_intersection_params_v.Length());
    for (int i = 1; i <= average_intersection_params_v.Length(); ++i) {
        av_intersection_params_v->SetValue(i, average_intersection_params_v(i));
    }

    Handle(Geom_BSplineSurface) gordonSurface = CTiglBSplineAlgorithms::createGordonSurface(reparam_splines_u, reparam_splines_v, av_intersection_params_u, av_intersection_params_v);
    return gordonSurface;
}

math_Matrix CTiglBSplineAlgorithms::bsplineBasisMat(int degree, const TColStd_Array1OfReal& knots, const TColStd_Array1OfReal& params)
{
    Standard_Integer ncp = knots.Length() - degree - 1;
    math_Matrix mx(1, params.Length(), 1, ncp);
    mx.Init(0.);
    math_Matrix bspl_basis(1, 1, 1, degree+1);
    bspl_basis.Init(0.);
    for (Standard_Integer iparm = 1; iparm <= params.Length(); ++iparm) {
        Standard_Integer basis_start_index = 0;
        BSplCLib::EvalBsplineBasis(1, 0, degree+1, knots, params.Value(iparm), basis_start_index, bspl_basis);
        mx.Set(iparm, iparm, basis_start_index, basis_start_index + degree, bspl_basis);
    }
    return mx;
}

} // namespace tigl
