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
#include <CTiglBSplineFitMod.h>

#include <Geom2d_BSplineCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom2dAPI_Interpolate.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColgp_HArray1OfPnt2d.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
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

    // helper functions for method createCommonKnotsVectorSurfaceOneDir

    // typedef KnotInsertionCall
    void knot_insertion_u(const Handle(Geom_BSplineSurface) surface, double knot_u, int mult)
    {
        double tolerance = 1e-15;
        surface->InsertUKnot(knot_u, mult, tolerance, false);

    }

    void knot_insertion_v(const Handle(Geom_BSplineSurface) surface, double knot_v, int mult)
    {
        double tolerance = 1e-15;
        surface->InsertVKnot(knot_v, mult, tolerance, false);
    }

    // typedef GetKnotCall
    double get_knot_u(const Handle(Geom_BSplineSurface) surface, int index)
    {
        return surface->UKnot(index);
    }

    double get_knot_v(const Handle(Geom_BSplineSurface) surface, int index)
    {
        return surface->VKnot(index);
    }

    // typedef GetMultCall
    int get_knot_u_mult(const Handle(Geom_BSplineSurface) surface, int index)
    {
        return surface->UMultiplicity(index);
    }

    int get_knot_v_mult(const Handle(Geom_BSplineSurface) surface, int index)
    {
        return surface->VMultiplicity(index);
    }


    // typedef GetIntCall
    int get_NbKnots_u(const Handle(Geom_BSplineSurface) surface)
    {
        return surface->NbUKnots();
    }

    int get_NbKnots_v(const Handle(Geom_BSplineSurface) surface)
    {
        return surface->NbVKnots();
    }

    int get_degree_u(const Handle(Geom_BSplineSurface) surface)
    {
        return surface->UDegree();
    }

    int get_degree_v(const Handle(Geom_BSplineSurface) surface)
    {
        return surface->VDegree();
    }

    void insert_knot_curve(const Handle(Geom_BSplineCurve) curve, double knot, int mult)
    {
        curve->InsertKnot(knot, mult, 1e-15, false);
    }

    double get_knot_curve(const Handle(Geom_BSplineCurve) curve, int index)
    {
        return curve->Knot(index);
    }

    int get_mult_curve(const Handle(Geom_BSplineCurve) curve, int index)
    {
        return curve->Multiplicity(index);
    }

    int get_degree_curve(const Handle(Geom_BSplineCurve) curve)
    {
        return curve->Degree();
    }

    int get_nbknots_curve(const Handle(Geom_BSplineCurve) curve)
    {
        return curve->NbKnots();
    }

    // typedefs for createCommonKnotsVectorSurfaceOneDir
    typedef void (* KnotInsertionCall)(const Handle(Geom_Geometry) surface, double knot, int mult);
    typedef double (* GetKnotCall)(const Handle(Geom_Geometry) surface, int index);
    typedef int (* GetMultCall)(const Handle(Geom_Geometry) surface, int index);
    typedef int (* GetIntCall)(const Handle(Geom_Geometry) surface);

    /**
     * @brief createCommonKnotsVectorImpl:
     *          Creates a common knot vector in u- or v-direction of the given vector of B-splines
     *          The common knot vector contains all knots in u- or v-direction of all splines with the highest multiplicity of all splines.
     * @param old_splines_vector:
     *          the given vector of B-spline splines that could have a different knot vector in u- or v-direction
     * @param insert_knot
     *          calls the appropriate function for knot insertion depending on creating a common knot vector in u- (Geom_BSplineSurface::InsertUKnot) or in v-direction (Geom_BSplineSurface::InsertVKnot)
     * @param get_knot_number
     *          calls the appropriate function for the number of knots depending on creating a common knot vector in u- (Geom_BSplineSurface::NbUKnots) or in v-direction (Geom_BSplineSurface::NbVKnots)
     * @param get_knot
     *          calls the appropriate function that returns a knot at an index in the knot vector depending on creating a common knot vector in u- (Geom_BSplineSurface::UKnot) or in v-direction (Geom_BSplineSurface::VKnot)
     * @param get_mult
     *          calls the appropriate function which returns the multiplicity of a certain knot depending on creating a common knot vector in u- (Geom_BSplineSurface::UMultiplicity) or in v-direction (Geom_BSplineSurface::VMultiplicity)
     * @param get_degree
     *          calls the appropriate function which returns the degree of the surface depending on creating a common knot vector in u- (Geom_BSplineSurface::UDegree) or in v-direction (Geom_BSplineSurface::VDegree)
     * @return
     *          the given vector of B-spline splines, now with a common knot vector in u- or in v-direction
     *          The B-spline surface geometry remains the same.
     */
    template <class TGeometry>
    std::vector<TGeometry >
    createCommonKnotsVectorImpl(const std::vector<TGeometry > old_splines,
                                KnotInsertionCall insert_knot,
                                GetIntCall get_knot_number,
                                GetKnotCall get_knot,
                                GetMultCall get_mult,
                                GetIntCall get_degree)
    {
        // all B-spline splines must have the same parameter range in the chosen direction
        double begin_param_dir = get_knot(old_splines[0], 1);
        double end_param_dir = get_knot(old_splines[0], get_knot_number(old_splines[0]));
        for (unsigned int surface_idx = 0; surface_idx < old_splines.size(); ++surface_idx) {
            double begin_param_dir_surface = get_knot(old_splines[surface_idx], 1);
            double end_param_dir_surface = get_knot(old_splines[surface_idx], get_knot_number(old_splines[surface_idx]));
            if (std::abs(begin_param_dir_surface - begin_param_dir) > 1e-5 || std::abs(end_param_dir_surface - end_param_dir) > 1e-5) {
                throw tigl::CTiglError("B-splines don't have the same parameter range at least in one direction (u / v) in method createCommonKnotsVectorImpl!", TIGL_MATH_ERROR);
            }
        }

        // all B-spline splines must have the same degree in the chosen direction
        int degree = get_degree(old_splines[0]);
        for (unsigned int surface_idx = 0; surface_idx < old_splines.size(); ++surface_idx) {
            if (get_degree(old_splines[surface_idx]) != degree) {
                throw tigl::CTiglError("B-splines don't have the same degree at least in one direction (u / v) in method createCommonKnotsVectorImpl!", TIGL_MATH_ERROR);
            }
        }


        // create a copy of the old B-splines in order to get no side effects
        std::vector<TGeometry> splines_vector;
        for (typename std::vector<TGeometry>::const_iterator splineIt = old_splines.begin(); splineIt != old_splines.end(); ++splineIt) {
            TGeometry new_surface = TGeometry::DownCast((*splineIt)->Copy());
            splines_vector.push_back(new_surface);
        }

        // create a vector of all knots in chosen direction (u or v) of all splines
        std::vector<double> all_knots_dir;
        for (typename std::vector<TGeometry>::const_iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            TGeometry spline = *splineIt;
            for (int knot_idx = 1; knot_idx <= get_knot_number(spline); ++knot_idx) {
                double dir_knot = get_knot(spline, knot_idx);
                all_knots_dir.push_back(dir_knot);
            }
        }

        // sort vector of all knots in given direction of all splines
        std::sort(all_knots_dir.begin(), all_knots_dir.end());

        // delete duplicate knots, so that in all_knots are all unique knots
        all_knots_dir.erase(std::unique(all_knots_dir.begin(), all_knots_dir.end(), helper_function_unique), all_knots_dir.end());


        // find highest multiplicities
        std::vector<int> end_mult_vector_dir(all_knots_dir.size(), 0);
        for (typename std::vector<TGeometry>::const_iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            TGeometry spline = *splineIt;

            for (unsigned int all_knot_dir_idx = 0; all_knot_dir_idx < all_knots_dir.size(); ++all_knot_dir_idx) {

                // get multiplicity of current knot in surface
                int mult = 0;
                for (int spline_knot_dir_idx = 1; spline_knot_dir_idx <= get_knot_number(spline); ++spline_knot_dir_idx) {
                    if (std::abs(get_knot(spline, spline_knot_dir_idx) - all_knots_dir[all_knot_dir_idx]) < 1e-15) {
                        mult = get_mult(spline, spline_knot_dir_idx);
                    }
                }

                if (mult > end_mult_vector_dir[all_knot_dir_idx]) {
                    end_mult_vector_dir[all_knot_dir_idx] = mult;
                }
            }
        }

        // now insert missing knots in all splines
        for (typename std::vector<TGeometry>::iterator splineIt = splines_vector.begin(); splineIt != splines_vector.end(); ++splineIt) {
            TGeometry spline = *splineIt;
            for (unsigned int all_knot_dir_idx = 0; all_knot_dir_idx < all_knots_dir.size(); ++all_knot_dir_idx) {
                int mult = end_mult_vector_dir[all_knot_dir_idx]; // always >= 0
                double knot = all_knots_dir[all_knot_dir_idx];
                insert_knot(spline, knot, mult);
            }
        }

        return splines_vector;
    }

}


namespace tigl
{


Handle(TColStd_HArray1OfReal) CTiglBSplineAlgorithms::computeParamsBSplineCurve(const TColgp_Array1OfPnt& points, const double alpha)
{
    Handle(TColStd_HArray1OfReal) parameters(new TColStd_HArray1OfReal(points.Lower(), points.Upper()));

    parameters->SetValue(points.Lower(), 0.);

    for (int i = points.Lower() + 1; i <= points.Upper(); ++i) {
        double length = pow(points(i).SquareDistance(points(i - 1)), alpha / 2.);
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
    // B-splines must have  the same amount of control points

    // first for parameters in u-direction:
    TColStd_Array2OfReal parameters_u(points.LowerRow(), points.UpperRow(), points.LowerCol(), points.UpperCol());
    for (int v_idx = points.LowerCol(); v_idx <= points.UpperCol(); ++v_idx) {
        TColgp_Array1OfPnt points_u_line(points.LowerRow(), points.UpperRow());

        for (int cp_idx = points.LowerRow(); cp_idx <= points.UpperRow(); ++cp_idx) {
            points_u_line(cp_idx) = points(cp_idx, v_idx);
        }

        Handle(TColStd_HArray1OfReal) parameters_u_line = computeParamsBSplineCurve(points_u_line, alpha);

        // save these parameters_spline in parameters_u
        for (int param_idx = parameters_u_line->Lower(); param_idx <= parameters_u_line->Upper(); ++param_idx) {
            parameters_u(param_idx, v_idx) = parameters_u_line->Value(param_idx);
        }
    }

    // averaging along v-direction
    Handle(TColStd_HArray1OfReal) parameters_u_average(new TColStd_HArray1OfReal(points.LowerRow(), points.UpperRow()));
    for (int param_idx = parameters_u.LowerRow(); param_idx <= parameters_u.UpperRow(); ++param_idx) {
        double sum = 0;
        for (int spline_idx = parameters_u.LowerCol(); spline_idx <= parameters_u.UpperCol(); ++spline_idx) {
            sum += parameters_u(param_idx, spline_idx);
        }
        parameters_u_average->SetValue(param_idx, sum / points.RowLength());
    }

    // now for parameters in v-direction:
    TColStd_Array2OfReal parameters_v(points.LowerRow(), points.UpperRow(), points.LowerCol(), points.UpperCol());
    for (int param_idx = points.LowerRow(); param_idx <= points.UpperRow(); ++param_idx) {
        TColgp_Array1OfPnt points_v(points.LowerCol(), points.UpperCol());

        for (int cp_v_idx = points.LowerCol(); cp_v_idx <= points.UpperCol(); ++cp_v_idx) {
            points_v(cp_v_idx) = points(param_idx, cp_v_idx);
        }
        Handle(TColStd_HArray1OfReal) parameters_v_line = computeParamsBSplineCurve(points_v, alpha);

        // save these parameters_v_spline in parameters_v
        for (int spline_idx = points.LowerCol(); spline_idx <= points.UpperCol(); ++spline_idx) {
            parameters_v(param_idx, spline_idx) = parameters_v_line->Value(spline_idx);
        }
    }

    // averaging along u-direction
    Handle(TColStd_HArray1OfReal) parameters_v_average = new TColStd_HArray1OfReal (points.LowerCol(), points.UpperCol());
    for (int spline_idx = points.LowerCol(); spline_idx <= points.UpperCol(); ++spline_idx) {
        double sum = 0;
        for (int param_idx = points.LowerRow(); param_idx <= points.UpperRow(); ++param_idx) {
            sum += parameters_v(param_idx, spline_idx);
        }
        parameters_v_average->SetValue(spline_idx, sum / points.ColLength());
    }

    // put computed parameters for both u- and v-direction in output tuple
    return std::make_pair(parameters_u_average, parameters_v_average);

}


std::vector<Handle(Geom_BSplineCurve)> CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector)
{
    return createCommonKnotsVectorImpl<Handle(Geom_BSplineCurve)>(splines_vector,
                                                                  reinterpret_cast<KnotInsertionCall>(insert_knot_curve),
                                                                  reinterpret_cast<GetIntCall>(get_nbknots_curve),
                                                                  reinterpret_cast<GetKnotCall>(get_knot_curve),
                                                                  reinterpret_cast<GetMultCall>(get_mult_curve),
                                                                  reinterpret_cast<GetIntCall>(get_degree_curve));
}

std::vector<Handle(Geom_BSplineSurface) > CTiglBSplineAlgorithms::createCommonKnotsVectorSurface(const std::vector<Handle(Geom_BSplineSurface) >& old_surfaces_vector)
{
    // all B-spline surfaces must have the same parameter range in u- and v-direction

    // create a copy of the old B-spline surfaces in order to get no shadow effects
    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector;

    // first create a common knot vector in u-direction
    surfaces_vector = createCommonKnotsVectorImpl<Handle(Geom_BSplineSurface)>(old_surfaces_vector,
                                                                               reinterpret_cast<KnotInsertionCall>(knot_insertion_u),
                                                                               reinterpret_cast<GetIntCall>(get_NbKnots_u),
                                                                               reinterpret_cast<GetKnotCall>(get_knot_u),
                                                                               reinterpret_cast<GetMultCall>(get_knot_u_mult),
                                                                               reinterpret_cast<GetIntCall>(get_degree_u));

    // now create a common knot vector in v-direction
    surfaces_vector = createCommonKnotsVectorImpl<Handle(Geom_BSplineSurface)>(surfaces_vector,
                                                                               reinterpret_cast<KnotInsertionCall>(knot_insertion_v),
                                                                               reinterpret_cast<GetIntCall>(get_NbKnots_v),
                                                                               reinterpret_cast<GetKnotCall>(get_knot_v),
                                                                               reinterpret_cast<GetMultCall>(get_knot_v_mult),
                                                                               reinterpret_cast<GetIntCall>(get_degree_v));

    return surfaces_vector;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::skinnedBSplineSurfaceParams(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector,
                                                                                const Handle(TColStd_HArray1OfReal) v_parameters,
                                                                                bool is_closed_v)
{
    // check amount of given parameters
    if (v_parameters->Length() != splines_vector.size()) {
        throw CTiglError("The amount of given parameters has to be equal to the amount of given B-splines!", TIGL_MATH_ERROR);
    }

    // match degree of given B-splines
    int u_degree = 0;
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        if (splines_vector[spline_idx]->Degree() > u_degree) {
            u_degree = splines_vector[spline_idx]->Degree();
        }
    }

    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        splines_vector[spline_idx]->IncreaseDegree(u_degree);
    }

    // create a common knot vector for all splines
    std::vector<Handle(Geom_BSplineCurve) > ready_splines_vector = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(splines_vector);

    // create a matrix of control points of all B-splines (splines do have the same amount of control points now)
    TColgp_Array2OfPnt controlPoints(1, ready_splines_vector[0]->NbPoles(), 1, ready_splines_vector.size());
    for (unsigned int spline_idx = 1; spline_idx <= ready_splines_vector.size(); ++spline_idx) {
        for (int point_idx = 1; point_idx <= ready_splines_vector[0]->NbPoles(); ++point_idx) {
            controlPoints(point_idx, spline_idx) = ready_splines_vector[spline_idx - 1]->Pole(point_idx);
        }
    }

    // again create spline to get knots and multiplicities in v-direction
    Handle(TColgp_HArray1OfPnt) controlPoints_spline_v = new TColgp_HArray1OfPnt(controlPoints.LowerCol(), controlPoints.UpperCol());
    for (int point_v_idx = controlPoints.LowerCol(); point_v_idx <= controlPoints.UpperCol(); ++point_v_idx) {
        controlPoints_spline_v->SetValue(point_v_idx, controlPoints(1, point_v_idx));
    }

    // create first spline that interoplates first column of control points in v-direction
    // separately in order to get knots_v and mults_v for creating the Geom_BSplineSurface:
    GeomAPI_Interpolate interpolationObject_v(controlPoints_spline_v, v_parameters, false /*is_closed_v */, 1e-15);
    interpolationObject_v.Perform();

    // check that interpolation was successful
    assert(interpolationObject_v.IsDone());

    Handle(Geom_BSplineCurve) spline_v = interpolationObject_v.Curve();

    // support for closed v-directional B-splines
    /*if (spline_v->IsClosed()) {
        spline_v->SetNotPeriodic();
    }*/

    unsigned int degree_v = spline_v->Degree();

    // create matrix of new control points with size which is possibly DIFFERENT from the size of controlPoints
    TColgp_Array2OfPnt new_controlPoints(1, ready_splines_vector[0]->NbPoles(), 1, spline_v->NbPoles());
    for (int i = new_controlPoints.LowerCol(); i <= new_controlPoints.UpperCol(); ++i) {
        new_controlPoints(1, i) = spline_v->Pole(i);
    }

    // now continue to create new control points by interpolating the remaining columns of controlPoints in Skinning direction (here v-direction) by B-splines
    for (int point_u_idx = controlPoints.LowerRow() + 1; point_u_idx <= controlPoints.UpperRow(); ++point_u_idx) {
        Handle(TColgp_HArray1OfPnt) controlPoints_spline = new TColgp_HArray1OfPnt(controlPoints.LowerCol(), controlPoints.UpperCol());
        for (int point_v_idx = controlPoints.LowerCol(); point_v_idx <= controlPoints.UpperCol(); ++point_v_idx) {
            controlPoints_spline->SetValue(point_v_idx, controlPoints(point_u_idx, point_v_idx));
        }
        GeomAPI_Interpolate interpolationObject(controlPoints_spline, v_parameters, false, 1e-5);
        interpolationObject.Perform();

        // check that interpolation was successful
        assert(interpolationObject.IsDone());

        Handle(Geom_BSplineCurve) spline = interpolationObject.Curve();

        // support for closed B-spline curves
        /*if (spline->IsClosed()) {
            spline->SetNotPeriodic();
        }*/

        for (int i = new_controlPoints.LowerCol(); i <= new_controlPoints.UpperCol(); ++i) {
            new_controlPoints(point_u_idx, i) = spline->Pole(i);
        }

        // check degree always the same
        assert(degree_v == spline->Degree());
    }

    TColStd_Array1OfReal knots_v(1, spline_v->NbKnots());
    spline_v->Knots(knots_v);
    TColStd_Array1OfInteger mults_v(1, spline_v->NbKnots());
    spline_v->Multiplicities(mults_v);

    const Handle(Geom_BSplineCurve) firstCurve = ready_splines_vector[0];
    TColStd_Array1OfReal knots_u(1, firstCurve->NbKnots());
    firstCurve->Knots(knots_u);
    TColStd_Array1OfInteger mults_u(1, firstCurve->NbKnots());
    firstCurve->Multiplicities(mults_u);

    Handle(Geom_BSplineSurface) skinnedSurface = new Geom_BSplineSurface(new_controlPoints, knots_u, knots_v, mults_u, mults_v, u_degree, degree_v);

    return skinnedSurface;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::skinnedBSplineSurface(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector, bool is_closed_v)
{
    // // // // // // // // // // for computing control points ---beginning

    // match degree of given B-splines
    int new_degree = 0;
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        if (splines_vector[spline_idx]->Degree() > new_degree) {
            new_degree = splines_vector[spline_idx]->Degree();
        }
    }

    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        splines_vector[spline_idx]->IncreaseDegree(new_degree);
    }

    // create a common knot vector for all splines
    std::vector<Handle(Geom_BSplineCurve) > ready_splines_vector = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(splines_vector);

    // // // // // // // // // // for computing control points ---end

    // create a matrix of control points of all B-splines (splines do have the same amount of control points now)
    TColgp_Array2OfPnt controlPoints(1, ready_splines_vector[0]->NbPoles(), 1, ready_splines_vector.size());
    for (unsigned int spline_idx = 1; spline_idx <= ready_splines_vector.size(); ++spline_idx) {
        for (int point_idx = 1; point_idx <= ready_splines_vector[0]->NbPoles(); ++point_idx) {
            controlPoints(point_idx, spline_idx) = ready_splines_vector[spline_idx - 1]->Pole(point_idx);
        }
    }

    std::pair<Handle(TColStd_HArray1OfReal), Handle(TColStd_HArray1OfReal) > parameters = CTiglBSplineAlgorithms::computeParamsBSplineSurf(controlPoints);
    Handle(TColStd_HArray1OfReal) v_parameters = new TColStd_HArray1OfReal(1, parameters.second->Length());
    for (int param_idx = 1; param_idx <= parameters.second->Length(); ++param_idx) {
        v_parameters->SetValue(param_idx, parameters.second->Value(param_idx));
    }

    return CTiglBSplineAlgorithms::skinnedBSplineSurfaceParams(ready_splines_vector, v_parameters, is_closed_v);
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

    BSplineFit approximationObj(3, n_control_pnts, t_vector);
    approximationObj.Fit(points, t_vector);

    Handle(Geom_BSplineCurve) reparametrized_spline = approximationObj.Curve();

    return reparametrized_spline;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::flipSurface(const Handle(Geom_BSplineSurface) surface)
{
    TColgp_Array2OfPnt swapped_control_points(1, surface->NbVPoles(), 1, surface->NbUPoles());
    TColgp_Array2OfPnt controlPoints(1, surface->NbUPoles(), 1, surface->NbVPoles());
    surface->Poles(controlPoints);

    for (int cp_u_idx = 1; cp_u_idx <= surface->NbUPoles(); ++cp_u_idx) {
        for (int cp_v_idx = 1; cp_v_idx <= surface->NbVPoles(); ++cp_v_idx) {
            swapped_control_points(cp_v_idx, cp_u_idx) = controlPoints(cp_u_idx, cp_v_idx);
        }
    }

    // also swap axes of the weights for NURBS
    TColStd_Array2OfReal swapped_weights(1, surface->NbVPoles(), 1, surface->NbUPoles());
    TColStd_Array2OfReal weights(1, surface->NbUPoles(), 1, surface->NbVPoles());
    for (int weight_u_idx = 1; weight_u_idx <= surface->NbUPoles(); ++weight_u_idx) {
        for (int weight_v_idx = 1; weight_v_idx <= surface->NbVPoles(); ++weight_v_idx) {
            swapped_weights(weight_v_idx, weight_u_idx) = weights(weight_u_idx, weight_v_idx);
        }
    }

    TColStd_Array1OfReal knots_u(1, surface->NbUKnots());
    surface->UKnots(knots_u);
    TColStd_Array1OfReal knots_v(1, surface->NbVKnots());
    surface->VKnots(knots_v);

    TColStd_Array1OfInteger mults_u(1, surface->NbUKnots());
    surface->UMultiplicities(mults_u);
    TColStd_Array1OfInteger mults_v(1, surface->NbVKnots());
    surface->VMultiplicities(mults_v);

    Handle(Geom_BSplineSurface) flippedSurface = new Geom_BSplineSurface(swapped_control_points, knots_v, knots_u, mults_v, mults_u, surface->VDegree(), surface->UDegree());

    return flippedSurface;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::interpolatingSurface(const TColgp_Array2OfPnt& points, const Handle(TColStd_HArray1OfReal) parameters_u, const Handle(TColStd_HArray1OfReal) parameters_v, bool is_closed_u, bool is_closed_v) {

    // first interpolate all points by B-splines in u-direction
    std::vector<Handle(Geom_BSplineCurve)> splines_u_vector;
    for (int point_v_idx = points.LowerCol(); point_v_idx <= points.UpperCol(); ++point_v_idx) {
        Handle(TColgp_HArray1OfPnt) points_u = new TColgp_HArray1OfPnt(points.LowerRow(), points.UpperRow());
        for (int point_u_idx = points.LowerRow(); point_u_idx <= points.UpperRow(); ++point_u_idx) {
            points_u->SetValue(point_u_idx, points(point_u_idx, point_v_idx));
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
    Handle(Geom_BSplineSurface) interpolatingSurf = CTiglBSplineAlgorithms::skinnedBSplineSurfaceParams(splines_u_vector, parameters_v, is_closed_v);

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
    // u-directional splines
    double splines_scale = CTiglBSplineAlgorithms::scaleOfBSplines(profiles);

    // v-directional splines
    double splines_v_scale = CTiglBSplineAlgorithms::scaleOfBSplines(guides);

    splines_scale = (splines_scale + splines_v_scale) / 2.;

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
    unsigned int degree_v_dir = profiles.size() - 1;
    Handle(Geom_BSplineSurface) surface_v = skinnedBSplineSurfaceParams(profiles, intersection_params_spline_v, guides[0]->IsClosed());
    // therefore reparametrization before this method

    // Skinning in u-direction with v directional B-Splines
    unsigned int degree_u_dir = guides.size() - 1;
    Handle(Geom_BSplineSurface) surface_u_unflipped = skinnedBSplineSurfaceParams(guides, intersection_params_spline_u, profiles[0]->IsClosed());

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

    // find out the average scale of the two B-splines in order to being able to handle a more approximate dataset and find its intersections
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

    // check parametrization of u-directional B-splines and change it in order to support vectors of B-splines with different parametrizations:
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

    // check parametrization of v-directional B-splines:
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
