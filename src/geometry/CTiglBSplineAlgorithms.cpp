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
#include "CTiglBSplineFit.h"

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

    // helper function for std::unique
    bool helper_function_unique(double a, double b)
    {
        return (fabs(a - b) < 1e-15);
    }
    
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

        void insertKnot(double knot, int mult)
        {
            double tolerance = 1e-15;
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

        void insertKnot(double knot, int mult)
        {
            double tolerance = 1e-15;
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
    bool haveSameRange(const std::vector<SplineAdapter>& splines_vector)
    {
        double begin_param_dir = splines_vector[0].getKnot(1);
        double end_param_dir = splines_vector[0].getKnot(splines_vector[0].getNKnots());
        for (unsigned int spline_idx = 1; spline_idx < splines_vector.size(); ++spline_idx) {
            const SplineAdapter& curSpline = splines_vector[spline_idx];
            double begin_param_dir_surface = curSpline.getKnot(1);
            double end_param_dir_surface = curSpline.getKnot(curSpline.getNKnots());
            if (std::abs(begin_param_dir_surface - begin_param_dir) > 1e-5 || std::abs(end_param_dir_surface - end_param_dir) > 1e-5) {
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
    int findKnot(const SplineAdapter& spline, double knot)
    {
        for (int curSplineKnotIdx = 1; curSplineKnotIdx <= spline.getNKnots(); ++curSplineKnotIdx) {
            if (std::abs(spline.getKnot(curSplineKnotIdx) - knot) < 1e-15) {
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
    void makeGeometryCompatibleImpl(std::vector<SplineAdapter>& splines_vector)
    {
        // all B-spline splines must have the same parameter range in the chosen direction
        if (!haveSameRange(splines_vector)) {
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
        resultKnots.erase(std::unique(resultKnots.begin(), resultKnots.end(), helper_function_unique), resultKnots.end());

        // find highest multiplicities
        std::vector<int> resultMults(resultKnots.size(), 0);
        for (typename std::vector<SplineAdapter>::const_iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            const SplineAdapter& spline = *splineIt;
            for (unsigned int knotIdx = 0; knotIdx < resultKnots.size(); ++knotIdx) {
                // get multiplicity of current knot in surface
                int splKnotIdx = findKnot(spline, resultKnots[knotIdx]);
                if (splKnotIdx > 0) {
                    resultMults[knotIdx] = std::max(resultMults[knotIdx], spline.getMult(splKnotIdx));
                }
            }
        }

        // now insert missing knots in all splines
        for (typename std::vector<SplineAdapter>::iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            SplineAdapter& spline = *splineIt;
            for (unsigned int knotIdx = 0; knotIdx < resultKnots.size(); ++knotIdx) {
                spline.insertKnot(resultKnots[knotIdx], resultMults[knotIdx]);
            }
        }
    }
    
    Handle(TColgp_HArray1OfPnt) array2GetColumn(const TColgp_Array2OfPnt& matrix, int colIndex)
    {
        Handle(TColgp_HArray1OfPnt) colVector =  new TColgp_HArray1OfPnt(matrix.LowerRow(), matrix.UpperRow());

        for (int rowIdx = matrix.LowerRow(); rowIdx <= matrix.UpperRow(); ++rowIdx) {
            colVector->SetValue(rowIdx, matrix(rowIdx, colIndex));
        }

        return colVector;
    }
    
    Handle(TColgp_HArray1OfPnt) array2GetRow(const TColgp_Array2OfPnt& matrix, int rowIndex)
    {
        Handle(TColgp_HArray1OfPnt) rowVector = new TColgp_HArray1OfPnt(matrix.LowerCol(), matrix.UpperCol());
        
        for (int colIdx = matrix.LowerCol(); colIdx <= matrix.UpperCol(); ++colIdx) {
            rowVector->SetValue(colIdx, matrix(rowIndex, colIdx));
        }
        
        return rowVector;
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
        Handle(TColStd_HArray1OfReal) parameters_u_line = computeParamsBSplineCurve(array2GetColumn(points, vIdx), alpha);

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
        Handle(TColStd_HArray1OfReal) parameters_v_line = computeParamsBSplineCurve(array2GetRow(points, uIdx), alpha);

        // average over rows
        for (int vIdx = points.LowerCol(); vIdx <= points.UpperCol(); ++vIdx) {
            double val = paramsV->Value(vIdx) + parameters_v_line->Value(vIdx)/(double)points.ColLength();
            paramsV->SetValue(vIdx, val);
        }
    }

    // put computed parameters for both u- and v-direction in output tuple
    return std::make_pair(paramsU, paramsV);

}


std::vector<Handle(Geom_BSplineCurve)> CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector)
{
    // TODO: Match parameter range

    // Create a copy that we can modify
    std::vector<CurveAdapterView> splines_adapter;
    for (size_t i = 0; i < splines_vector.size(); ++i) {
        splines_adapter.push_back(Handle(Geom_BSplineCurve)::DownCast(splines_vector[i]->Copy()));
    }

    makeGeometryCompatibleImpl(splines_adapter);

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
    makeGeometryCompatibleImpl(adapterSplines);

    for (size_t i = 0; i < old_surfaces_vector.size(); ++i) adapterSplines[i].setDir(vdir);

    // now in v direction
    makeGeometryCompatibleImpl(adapterSplines);

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
    std::vector<Handle(Geom_BSplineCurve) > compatSplines = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(curves);

    const Handle(Geom_BSplineCurve)& firstCurve = compatSplines[0];
    size_t numControlPointsU = firstCurve->NbPoles();

    unsigned int degreeV = 0;
    unsigned int degreeU = firstCurve->Degree();
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
        assert(degreeV == spline->Degree());
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
    std::vector<Handle(Geom_BSplineCurve) > compatibleSplines = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(bsplCurves);

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

    // insert knots at old parameter values, so that one can reparametrize intervals between these old parameter values afterwards
    for (unsigned int i = 0; i < old_parameters_vector.size(); ++i) {
        // if between parameter range of B-spline
        if (old_parameters_vector[i] - copied_spline->Knot(1) > 1e-15 && std::abs(copied_spline->Knot(copied_spline->NbKnots()) - old_parameters_vector[i]) > 1e-15) {
            // insert knot degree-times

            bool is_there = false;
            unsigned int is_there_idx = 0;
            for (int knot_idx = 1; knot_idx <= copied_spline->NbKnots(); ++knot_idx) {
                if (std::abs(old_parameters_vector[i] - copied_spline->Knot(knot_idx)) < 1e-7) {
                    is_there = true;
                    is_there_idx = knot_idx;
                }
            }

            if (is_there) {
                copied_spline->IncreaseMultiplicity(is_there_idx, copied_spline->Degree());
            }
            else {
                copied_spline->InsertKnot(old_parameters_vector[i]);
                for (int knot_idx = 1; knot_idx <= copied_spline->NbKnots(); ++knot_idx) {
                    if (std::abs(old_parameters_vector[i] - copied_spline->Knot(knot_idx)) < 1e-15) {  // found inserted knot
                        copied_spline->IncreaseMultiplicity(knot_idx, copied_spline->Degree());  // already inserted above (M=1 already)
                    }
                }
            }
            //copied_spline->InsertKnot(old_parameters_vector[i], copied_spline->Degree(), 0, Standard_False);
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

//    // decrease the multiplicities of the knots at the (now new) parameters except at the beginning and end parameter which are at the
//    // beginning and end of the parameter range of the B-spline
//    std::vector<Handle(Geom_BSplineCurve)> spline_vector;
//    spline_vector.push_back(copied_spline);
//    double spline_geom_scale = CTiglBSplineAlgorithms::scaleOfBSplines(spline_vector);
//    double spline_param_scale = copied_spline->Knot(copied_spline->NbKnots()) - copied_spline->Knot(1);
//    double smoothing_interval_length = 5e-2;

//    for (int parameter_idx = 2; parameter_idx <= new_parameters.Length() - 1; ++parameter_idx) {
//        copied_spline->InsertKnot(new_parameters(parameter_idx) - smoothing_interval_length / 2. * spline_param_scale);
//        copied_spline->InsertKnot(new_parameters(parameter_idx) + smoothing_interval_length / 2. * spline_param_scale);

//        // search for knot index
//        unsigned int knot_idx = 0;
//        for (int i = 1; i <= copied_spline->NbKnots(); ++i) {
//            if (std::abs(copied_spline->Knot(i) - new_parameters(parameter_idx)) < 1e-10) {
//                knot_idx = i;
//            }
//        }

//        copied_spline->RemoveKnot(knot_idx, std::max(1, copied_spline->Degree() - 2), 0.5 * spline_geom_scale);
//    }

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

    unsigned int n_flat_knots = 0;
    for (int i = 1; i <= mults.Length(); ++i) {
        n_flat_knots += mults(i);
    }
    TColStd_Array1OfReal flat_knots(1, n_flat_knots);
    BSplCLib::KnotSequence(knots, mults, flat_knots);

    TColgp_Array1OfPnt cp(1, copied_spline->NbPoles());
    copied_spline->Poles(cp);

    TColStd_Array1OfReal new_flat_knots(1, n_flat_knots + 5);
    unsigned int u_degree = 3;
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
    assert(interpolationObject.IsDone());

    Handle(Geom2d_BSplineCurve) reparametrizing_spline = interpolationObject.Curve();

    TColStd_Array1OfReal t(1, 101);
    for (int i = 1; i <= 101; ++i) {
        t(i) = (i - 1)/100. * (new_parameters(new_parameters.Length()) - new_parameters(1)) + new_parameters(1);
    }

    TColStd_Array1OfReal old_t(1, t.Length());
    for (int i = 1; i <= t.Length(); ++i) {
        gp_Pnt2d old_t_pnt = reparametrizing_spline->Value(t(i));
        old_t(i) = old_t_pnt.X();
    }

    TColgp_Array1OfPnt points(1, old_t.Length());
    for (int i = 1; i <= t.Length(); ++i) {
        points(i) = spline->Value(old_t(i));
    }

    // type conversion for new parameters t
    std::vector<double> t_vector;
    for (int parameter_idx = 1; parameter_idx <= t.Length(); ++parameter_idx) {
        t_vector.push_back(t(parameter_idx));
    }

    BSplineFit approximationObj(3, n_control_pnts);
    approximationObj.Fit(points, t_vector);

    Handle(Geom_BSplineCurve) reparametrized_spline = approximationObj.Curve();

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
        Handle(TColgp_HArray1OfPnt) points_u = new TColgp_HArray1OfPnt(points.LowerRow(), points.UpperRow());
        for (int cpUIdx = points.LowerRow(); cpUIdx <= points.UpperRow(); ++cpUIdx) {
            points_u->SetValue(cpUIdx, points(cpUIdx, cpVIdx));
        }
        GeomAPI_Interpolate interpolationObject(points_u, parameters_u, false /*is_closed_u*/, 1e-15);
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
        const Handle(Geom_Curve)& profile = profiles[profileIdx];
        assert(std::abs(profile->FirstParameter()) < 1e-7);
        assert(std::abs(profile->LastParameter() - 1.) < 1e-7);
    }

    for (unsigned int guideIdx = 0; guideIdx < guides.size(); ++guideIdx) {
        const Handle(Geom_Curve)& guide = guides[guideIdx];
        assert(std::abs(guide->FirstParameter()) < 1e-7);
        assert(std::abs(guide->LastParameter() - 1.) < 1e-7);
    }

    // find out the 'average' scale of the B-splines in order to being able to handle a more approximate dataset and find its intersections
    double splines_scale = 0.5 * (CTiglBSplineAlgorithms::scaleOfBSplines(profiles)+ CTiglBSplineAlgorithms::scaleOfBSplines(guides));

    if (std::abs(intersection_params_spline_u->Value(1)) > splines_scale * 1e-5 || std::abs(intersection_params_spline_u->Value(intersection_params_spline_u->Upper()) - 1.) > splines_scale * 1e-5) {
        throw tigl::CTiglError("WARNING: B-splines in u-direction mustn't stick out, spline network must be 'closed'!");
    }

    if (std::abs(intersection_params_spline_v->Value(1)) > splines_scale * 1e-5 || std::abs(intersection_params_spline_v->Value(intersection_params_spline_v->Upper()) - 1.) > splines_scale * 1e-5) {
        throw tigl::CTiglError("WARNING: B-splines in v-direction mustn't stick out, spline network must be 'closed'!");
    }

    // check compatibilty of network
    for (int u_param_idx = intersection_params_spline_u->Lower(); u_param_idx <= intersection_params_spline_u->Upper(); ++u_param_idx) {

        double spline_u_param = intersection_params_spline_u->Value(u_param_idx);
        Handle(Geom_BSplineCurve) spline_v = guides[u_param_idx - 1];
        for (int v_param_idx = intersection_params_spline_v->Lower(); v_param_idx <= intersection_params_spline_v->Upper(); ++v_param_idx) {
            Handle(Geom_BSplineCurve) spline_u = profiles[v_param_idx - 1];
            double spline_v_param = intersection_params_spline_v->Value(v_param_idx);

            gp_Pnt point_spline_u = spline_u->Value(spline_u_param);
            gp_Pnt point_spline_v = spline_v->Value(spline_v_param);

            if (std::abs(point_spline_u.X() - point_spline_v.X()) > splines_scale * 1e-5 || std::abs(point_spline_u.Y() - point_spline_v.Y()) > splines_scale * 1e-5 || std::abs(point_spline_u.Z() - point_spline_v.Z()) > splines_scale * 1e-5) {
                //throw tigl::CTiglError("B-spline network is incompatible (e.g. wrong parametrization) or intersection parameters are in a wrong order!");
            }
        }
    }

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
    for (unsigned int spline_idx = 0; spline_idx < profiles.size(); ++spline_idx) {
        for (int intersection_idx = intersection_params_spline_u->Lower(); intersection_idx <= intersection_params_spline_u->Upper(); ++intersection_idx) {
            Handle(Geom_BSplineCurve) spline_u = profiles[spline_idx];
            double parameter = intersection_params_spline_u->Value(intersection_idx);
            intersection_pnts(intersection_idx, spline_idx + 1) = spline_u->Value(parameter);
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

    // change small inaccuracies of the first knot:
    if (std::abs(surface_u->UKnot(1)) > 1e-15 && std::abs(surface_u->UKnot(1)) < 1e-5) {
        surface_u->SetUKnot(1, 0);
    }
    if (std::abs(surface_v->UKnot(1)) > 1e-15 && std::abs(surface_v->UKnot(1)) < 1e-5) {
        surface_v->SetUKnot(1, 0);
    }
    if (std::abs(tensorProdSurf->UKnot(1)) > 1e-15 && std::abs(tensorProdSurf->UKnot(1)) < 1e-5) {
        tensorProdSurf->SetUKnot(1, 0);
    }

    // set first not differently than above because of Open CASCADE bug in SetVKnot()
    TColStd_Array1OfReal vknots_surf_u(1, surface_u->NbVKnots());
    surface_u->VKnots(vknots_surf_u);
    if (std::abs(surface_u->VKnot(1)) > 1e-15 && std::abs(surface_u->VKnot(1)) < 1e-5) {
        vknots_surf_u(1) = 0;
        surface_u->SetVKnots(vknots_surf_u);
    }
    TColStd_Array1OfReal vknots_surf_v(1, surface_v->NbVKnots());
    surface_v->VKnots(vknots_surf_v);
    if (std::abs(surface_v->VKnot(1)) > 1e-15 && std::abs(surface_v->VKnot(1)) < 1e-5) {
        vknots_surf_v(1) = 0;
        surface_v->SetVKnots(vknots_surf_v);
    }
    TColStd_Array1OfReal vknots_surf_t(1, tensorProdSurf->NbVKnots());
    tensorProdSurf->VKnots(vknots_surf_t);
    if (std::abs(tensorProdSurf->VKnot(1)) > 1e-15 && std::abs(tensorProdSurf->VKnot(1)) < 1e-5) {
        vknots_surf_t(1) = 0;
        tensorProdSurf->SetVKnots(vknots_surf_t);
    }

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

std::vector<std::pair<double, double> > CTiglBSplineAlgorithms::intersectionFinder(const Handle(Geom_BSplineCurve) spline1, const Handle(Geom_BSplineCurve) spline2) {
    // light weight simple minimizer

    // check parametrization of B-splines beforehand

    // find out the average scale of the two B-splines in order to being able to handle a more approximate curves and find its intersections
    std::vector<Handle(Geom_BSplineCurve)> spline1_vector;
    spline1_vector.push_back(spline1);
    double splines_scale = CTiglBSplineAlgorithms::scaleOfBSplines(spline1_vector);

    std::vector<Handle(Geom_BSplineCurve)> spline2_vector;
    spline2_vector.push_back(spline2);
    double spline2_scale = CTiglBSplineAlgorithms::scaleOfBSplines(spline2_vector);

    splines_scale = (splines_scale + spline2_scale) / 2.;

    std::vector<std::pair<double, double> > intersection_params_vector;
    GeomAPI_ExtremaCurveCurve intersectionObj(spline1, spline2);
    for (int intersect_idx = 1; intersect_idx <= intersectionObj.NbExtrema(); ++intersect_idx) {
        double param1 = 0.;
        double param2 = 0.;
        intersectionObj.Parameters(intersect_idx, param1, param2);

        // filter out real intersections
        gp_Pnt point1 = spline1->Value(param1);
        gp_Pnt point2 = spline2->Value(param2);
        if (std::pow(point1.X() - point2.X(), 2) + std::pow(point1.Y() - point2.Y(), 2) + std::pow(point1.Z() - point2.Z(), 2) < 1e-7 * splines_scale) {
            intersection_params_vector.push_back(std::make_pair(param1, param2));
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

double CTiglBSplineAlgorithms::scaleOfBSplines(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector) {
    double scale = 0.;
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {

        gp_Pnt first_ctrl_pnt = splines_vector[spline_idx]->Pole(1);
        for (int ctrl_pnt_idx = 2; ctrl_pnt_idx <= splines_vector[spline_idx]->NbPoles(); ++ctrl_pnt_idx) {
            // compute distance of the first control point to the others and save biggest distance
            gp_Pnt ctrl_pnt = splines_vector[spline_idx]->Pole(ctrl_pnt_idx);
            double distance = std::sqrt(std::pow(first_ctrl_pnt.X() - ctrl_pnt.X(), 2) + std::pow(first_ctrl_pnt.Y() - ctrl_pnt.Y(), 2) + std::pow(first_ctrl_pnt.Z() - ctrl_pnt.Z(), 2));

            if (scale < distance) {
                scale = distance;
            }
        }
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

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::createGordonSurfaceGeneral(const std::vector<Handle(Geom_BSplineCurve) >& splines_u_vector,
                                                                               const std::vector<Handle(Geom_BSplineCurve) >& splines_v_vector) {

    // check whether there are any u-directional and v-directional B-splines in the vectors
    if (splines_u_vector.size() == 0) {
        throw tigl::CTiglError("There are no u-directional B-splines!");
    }

    if (splines_v_vector.size() == 0) {
        throw tigl::CTiglError("There are no v-directional B-splines!");
    }

    // check parameter ranges of u-directional B-splines and change it in order to support vectors of B-splines with different parameter ranges:
    for (unsigned int spline_u_idx = 0; spline_u_idx < splines_u_vector.size(); ++spline_u_idx) {
        // check parametrization of B-splines:
        TColStd_Array1OfReal knots(1, splines_u_vector[spline_u_idx]->NbKnots());
        splines_u_vector[spline_u_idx]->Knots(knots);

        if (std::abs(splines_u_vector[spline_u_idx]->Knot(1)) > 1e-15 || std::abs(splines_u_vector[spline_u_idx]->Knot(splines_u_vector[spline_u_idx]->NbKnots()) - 1) > 1e-15) {
            if (std::abs(splines_u_vector[spline_u_idx]->Knot(1)) > 1e-15) {  // parameter range doesn't start with 0

                for (int knot_idx = 1; knot_idx <= splines_u_vector[spline_u_idx]->NbKnots(); ++knot_idx) {
                    knots(knot_idx) -= splines_u_vector[spline_u_idx]->Knot(1);
                }
            }

            if (std::abs(splines_u_vector[spline_u_idx]->Knot(splines_u_vector[spline_u_idx]->NbKnots()) - 1) > 1e-15) {  // parameter range doesn't end with 1

                for (int knot_idx = 1; knot_idx <= splines_u_vector[spline_u_idx]->NbKnots(); ++knot_idx) {
                    knots(knot_idx) /= splines_u_vector[spline_u_idx]->Knot(splines_u_vector[spline_u_idx]->NbKnots());
                }
            }

            // edit spline
            splines_u_vector[spline_u_idx]->SetKnots(knots);
        }
    }
    // now the parameter range of all u-directional B-splines is [0, 1]

    // check parameter ranges of v-directional B-splines:
    for (unsigned int spline_v_idx = 0; spline_v_idx < splines_v_vector.size(); ++spline_v_idx) {
        // check parametrization of B-splines:
        TColStd_Array1OfReal knots(1, splines_v_vector[spline_v_idx]->NbKnots());
        splines_v_vector[spline_v_idx]->Knots(knots);

        if (std::abs(splines_v_vector[spline_v_idx]->Knot(1)) > 1e-15 || std::abs(splines_v_vector[spline_v_idx]->Knot(splines_v_vector[spline_v_idx]->NbKnots()) - 1) > 1e-15) {
            if (std::abs(splines_v_vector[spline_v_idx]->Knot(1)) > 1e-15) {  // parameter range doesn't start with 0

                for (int knot_idx = 1; knot_idx <= splines_v_vector[spline_v_idx]->NbKnots(); ++knot_idx) {
                    knots(knot_idx) -= splines_v_vector[spline_v_idx]->Knot(1);
                }
            }

            if (std::abs(splines_v_vector[spline_v_idx]->Knot(splines_v_vector[spline_v_idx]->NbKnots()) - 1) > 1e-15) {  // parameter range doesn't end with 1

                for (int knot_idx = 1; knot_idx <= splines_v_vector[spline_v_idx]->NbKnots(); ++knot_idx) {
                    knots(knot_idx) /= splines_v_vector[spline_v_idx]->Knot(splines_v_vector[spline_v_idx]->NbKnots());
                }
            }

            // edit spline
            splines_v_vector[spline_v_idx]->SetKnots(knots);
        }
    }
    // now the parameter range of all v-directional B-splines is [0, 1]

    // now find all intersections of all B-splines with each other
    math_Matrix intersection_params_u(0, splines_u_vector.size() - 1, 0, splines_v_vector.size() - 1);
    math_Matrix intersection_params_v(0, splines_u_vector.size() - 1, 0, splines_v_vector.size() - 1);

    for (unsigned int spline_u_idx = 0; spline_u_idx < splines_u_vector.size(); ++spline_u_idx) {
        for (unsigned int spline_v_idx = 0; spline_v_idx < splines_v_vector.size(); ++spline_v_idx) {
            std::vector<std::pair<double, double> > intersection_params_vector = CTiglBSplineAlgorithms::intersectionFinder(splines_u_vector[spline_u_idx], splines_v_vector[spline_v_idx]);

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
                if (splines_u_vector[0]->IsClosed()) {

                    if (spline_v_idx == 0) {
                        intersection_params_u(spline_u_idx, spline_v_idx) = std::min(intersection_params_vector[0].first, intersection_params_vector[1].first);
                    }
                    else if (spline_v_idx == splines_v_vector.size() - 1) {
                        intersection_params_u(spline_u_idx, spline_v_idx) = std::max(intersection_params_vector[0].first, intersection_params_vector[1].first);
                    }

                    // intersection_params_vector[0].second == intersection_params_vector[1].second
                    intersection_params_v(spline_u_idx, spline_v_idx) = intersection_params_vector[0].second;
                }

                // only the v-directional B-spline curves are closed
                if (splines_v_vector[0]->IsClosed()) {

                    if (spline_u_idx == 0) {
                        intersection_params_v(spline_u_idx, spline_v_idx) = std::min(intersection_params_vector[0].second, intersection_params_vector[1].second);
                    }
                    else if (spline_u_idx == splines_u_vector.size() - 1) {
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

    // create an object of the sorter class we want to use
    std::vector<Handle(Geom_Curve)> splines_u_vector_geomc;
    for (unsigned int curve_idx = 0; curve_idx < splines_u_vector.size(); ++curve_idx) {
        splines_u_vector_geomc.push_back((Handle(Geom_Curve)) splines_u_vector[curve_idx]);
    }

    std::vector<Handle(Geom_Curve)> splines_v_vector_geomc;
    for (unsigned int curve_idx = 0; curve_idx < splines_v_vector.size(); ++curve_idx) {
        splines_v_vector_geomc.push_back((Handle(Geom_Curve)) splines_v_vector[curve_idx]);
    }
    tigl::CTiglCurveNetworkSorter sorterObj(splines_u_vector_geomc, splines_v_vector_geomc, intersection_params_u, intersection_params_v);
    sorterObj.Perform();

    // get the sorted matrices and vectors
    intersection_params_u = sorterObj.ProfileIntersectionParms();
    intersection_params_v = sorterObj.GuideIntersectionParms();
    std::vector<Handle(Geom_BSplineCurve)> sorted_splines_u;
    std::vector<Handle (Geom_Curve)> sorted_splines_u_ = sorterObj.Profiles();
    for (unsigned int curve_idx = 0; curve_idx < sorted_splines_u_.size(); ++curve_idx) {
        sorted_splines_u.push_back(Handle(Geom_BSplineCurve)::DownCast(sorted_splines_u_[curve_idx]));
    }
    std::vector<Handle(Geom_BSplineCurve)> sorted_splines_v;
    std::vector<Handle (Geom_Curve)> sorted_splines_v_ = sorterObj.Guides();
    for (unsigned int curve_idx = 0; curve_idx < sorted_splines_v_.size(); ++curve_idx) {
        sorted_splines_v.push_back(Handle(Geom_BSplineCurve)::DownCast(sorted_splines_v_[curve_idx]));
    }

    // eliminate small inaccuracies of the intersection parameters:
    CTiglBSplineAlgorithms::eliminateInaccuraciesNetworkIntersections(sorted_splines_u, sorted_splines_v, intersection_params_u, intersection_params_v);

    TColStd_Array1OfReal average_intersection_params_u(1, sorted_splines_v.size());
    for (unsigned int spline_v_idx = 1; spline_v_idx <= sorted_splines_v.size(); ++spline_v_idx) {
        double sum = 0;
        for (unsigned int spline_u_idx = 1; spline_u_idx <= sorted_splines_u.size(); ++spline_u_idx) {
            sum += intersection_params_u(spline_u_idx - 1, spline_v_idx - 1);
        }
        average_intersection_params_u(spline_v_idx) = sum / sorted_splines_u.size();
    }

    TColStd_Array1OfReal average_intersection_params_v(1, sorted_splines_u.size());
    for (unsigned int spline_u_idx = 1; spline_u_idx <= sorted_splines_u.size(); ++spline_u_idx) {
        double sum = 0;
        for (unsigned int spline_v_idx = 1; spline_v_idx <= sorted_splines_v.size(); ++spline_v_idx) {
            sum += intersection_params_v(spline_u_idx - 1, spline_v_idx - 1); // TODO: scientific approach for power
        }
        average_intersection_params_v(spline_u_idx) = sum / sorted_splines_v.size();
    }


    if (average_intersection_params_u(1) > 1e-5 || average_intersection_params_v(1) > 1e-5) {
        throw CTiglError("At least one B-splines has no intersection at the beginning.");
    }

    // reparametrize u-directional B-splines
    std::vector<Handle(Geom_BSplineCurve)> reparam_splines_u;
    for (unsigned int spline_u_idx = 0; spline_u_idx < sorted_splines_u.size(); ++spline_u_idx) {

        TColStd_Array1OfReal old_parameters(1, sorted_splines_v.size());
        for (unsigned int spline_v_idx = 1; spline_v_idx <= sorted_splines_v.size(); ++spline_v_idx) {
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


        Handle(Geom_BSplineCurve) reparam_spline_u = CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(sorted_splines_u[spline_u_idx], old_parameters, average_intersection_params_u);
        reparam_splines_u.push_back(reparam_spline_u);
    }

    // reparametrize v-directional B-splines
    std::vector<Handle(Geom_BSplineCurve)> reparam_splines_v;
    for (unsigned int spline_v_idx = 0; spline_v_idx < sorted_splines_v.size(); ++spline_v_idx) {

        TColStd_Array1OfReal old_parameters(1, sorted_splines_u.size());
        for (unsigned int spline_u_idx = 1; spline_u_idx <= sorted_splines_u.size(); ++spline_u_idx) {
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

        Handle(Geom_BSplineCurve) reparam_spline_v = CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(sorted_splines_v[spline_v_idx], old_parameters, average_intersection_params_v);
        reparam_splines_v.push_back(reparam_spline_v);
    }

//    // check that B-spline geometry remains the same
//    std::vector<double> old_parameters;
//    for (unsigned int spline_v_idx = 0; spline_v_idx < sorted_splines_v.size(); ++spline_v_idx) {
//        old_parameters.push_back(intersection_params_u(0, spline_v_idx));
//    }

//    Handle(Geom_BSplineCurve) spline = sorted_splines_u[0];
//    Handle(Geom_BSplineCurve) reparam_spline = reparam_splines_u[0];
//    for (int param_idx = 0; param_idx < old_parameters.size(); ++param_idx) {
//        gp_Pnt old_point = spline->Value(old_parameters[param_idx]);
//        std::cout << std::endl << "average_intersection_params_u(param_idx): " << average_intersection_params_u(param_idx + 1);

//        gp_Pnt new_point = reparam_spline->Value(average_intersection_params_u(param_idx + 1));
//        assert(std::abs(old_point.X() - new_point.X()) < 1e-8);
//        assert(std::abs(old_point.Y() - new_point.Y()) < 1e-8);
//        assert(std::abs(old_point.Z() - new_point.Z()) < 1e-8);
//    }

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
} // namespace tigl
