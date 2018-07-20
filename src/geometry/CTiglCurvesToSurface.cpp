/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2018-07-18 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include <cassert>

#include "CTiglCurvesToSurface.h"
#include "CTiglError.h"
#include "CTiglBSplineAlgorithms.h"
#include "CTiglLogging.h"

#include <GeomAPI_Interpolate.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColgp_HArray2OfPnt.hxx>
#include <TColStd_HArray1OfInteger.hxx>


//TODO anonymous helper functions duplicated from CTiglBSplineAlgorithms.cpp
namespace {

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

}

namespace tigl {

TIGL_EXPORT CTiglCurvesToSurface::CTiglCurvesToSurface(std::vector<Handle(Geom_BSplineCurve) > const& splines_vector,
                                                       bool continuousIfClosed)
    : _continuousIfClosed(continuousIfClosed)
{
    CTiglBSplineAlgorithms::matchDegree(splines_vector);
    CalculateParameters(splines_vector);
    Perform(splines_vector);
}

TIGL_EXPORT CTiglCurvesToSurface::CTiglCurvesToSurface(std::vector<Handle(Geom_BSplineCurve) > const& splines_vector,
                                                       std::vector<double> const& parameters, bool continuousIfClosed)
    : _parameters(parameters)
    , _continuousIfClosed(continuousIfClosed)
{
    CTiglBSplineAlgorithms::matchDegree(splines_vector);
    Perform(splines_vector);
}

TIGL_EXPORT void CTiglCurvesToSurface::CalculateParameters(std::vector<Handle(Geom_BSplineCurve)> const& splines_vector)
{


    // create a common knot vector for all splines
    if ( _compatibleSplines.size() == 0 ) {
         _compatibleSplines = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(splines_vector, 1e-10);
    }

    // create a matrix of control points of all B-splines (splines do have the same amount of control points now)
    TColgp_Array2OfPnt controlPoints(1, _compatibleSplines[0]->NbPoles(),
                                     1, static_cast<Standard_Integer>(_compatibleSplines.size()));

    for (unsigned int spline_idx = 1; spline_idx <= _compatibleSplines.size(); ++spline_idx) {
        for (int point_idx = 1; point_idx <= _compatibleSplines[0]->NbPoles(); ++point_idx) {
            controlPoints(point_idx, spline_idx) = _compatibleSplines[spline_idx - 1]->Pole(point_idx);
        }
    }

    std::pair<std::vector<double>, std::vector<double> > parameters
            = CTiglBSplineAlgorithms::computeParamsBSplineSurf(controlPoints);
    _parameters = parameters.second;
}

TIGL_EXPORT void CTiglCurvesToSurface::Perform(std::vector<Handle(Geom_BSplineCurve) > const& curves)
{
    // check amount of given parameters
    if (_parameters.size() != curves.size()) {
        throw CTiglError("The amount of given parameters has to be equal to the amount of given B-splines!", TIGL_MATH_ERROR);
    }

    // check if all curves are closed
    double tolerance = CTiglBSplineAlgorithms::scale(curves) * CTiglBSplineAlgorithms::REL_TOL_CLOSED;
    bool makeClosed = _continuousIfClosed & curves.front()->IsEqual(curves.back(), tolerance);

    size_t nCurves = curves.size();

    // create a common knot vector for all splines
    if ( _compatibleSplines.size() == 0 ) {
        _compatibleSplines = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(curves, 1e-10);
    }

    const Handle(Geom_BSplineCurve)& firstCurve = _compatibleSplines[0];
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
            interpPointsVDir->SetValue(cpVIdx, _compatibleSplines[cpVIdx - 1]->Pole(cpUIdx));
        }
        GeomAPI_Interpolate interpolationObject(interpPointsVDir, toArray(_parameters), makeClosed, 1e-5);
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

    _skinnedSurface = new Geom_BSplineSurface(cpSurf->Array2(),
                                              knotsU, knotsV->Array1(),
                                              multsU, multsV->Array1(),
                                              degreeU, degreeV);

}

}
