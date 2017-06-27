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

#include <CTiglBSplineAlgorithms.h>
#include <CTiglError.h>
#include <CSharedPtr.h>

#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColgp_HArray1OfPnt.hxx>

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Precision.hxx>

namespace
{

    // helper function for std::unique
    bool helper_function_unique(double a, double b)
    {
        return (std::abs(a - b) < 1e-15);
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

    // helper function for sorting the given B-splines in createGordonSurface()
    bool pairCompare(const std::pair<unsigned int, double> first_pair, const std::pair<unsigned int, double> second_pair) {
      return first_pair.second < second_pair.second;
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

    parameters->SetValue(1, 0.);

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
    TColStd_Array2OfReal parameters_u(points.LowerCol(), points.UpperCol(), points.LowerRow(), points.UpperRow());
    for (int v_idx = points.LowerCol(); v_idx <= points.UpperCol(); ++v_idx) {
        TColgp_Array1OfPnt points_u_line(points.LowerRow(), points.UpperRow());

        for (int cp_idx = points.LowerRow(); cp_idx <= points.UpperRow(); ++cp_idx) {
            points_u_line(cp_idx) = points(cp_idx, v_idx);
        }

        Handle(TColStd_HArray1OfReal) parameters_u_line = computeParamsBSplineCurve(points_u_line, alpha);

        // save these parameters_spline in parameters_u
        for (int param_idx = parameters_u_line->Lower(); param_idx <= parameters_u_line->Upper(); ++param_idx) {
            parameters_u(v_idx, param_idx) = parameters_u_line->Value(param_idx);
        }
    }

    // averaging along v-direction
    Handle(TColStd_HArray1OfReal) parameters_u_average(new TColStd_HArray1OfReal(points.LowerRow(), points.UpperRow()));
    for (int param_idx = parameters_u.LowerCol(); param_idx <= parameters_u.UpperCol(); ++param_idx) {
        double sum = 0;
        for (int spline_idx = parameters_u.LowerRow(); spline_idx <= parameters_u.UpperRow(); ++spline_idx) {
            sum += parameters_u(spline_idx, param_idx);
        }
        parameters_u_average->SetValue(param_idx, sum / (parameters_u.UpperRow() - parameters_u.LowerRow() + 1.));
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
        parameters_v_average->SetValue(spline_idx, sum / (points.UpperRow() - points.LowerRow() + 1.));
    }

    // put computed parameters for both u- and v-direction in output tuple
    return std::make_pair(parameters_u_average, parameters_v_average);

}


std::vector<Handle(Geom_BSplineCurve)> CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector)
{
    return createCommonKnotsVectorImpl<Handle(Geom_BSplineCurve)>(splines_vector,
                                                                           (KnotInsertionCall) insert_knot_curve,
                                                                           (GetIntCall) get_nbknots_curve,
                                                                           (GetKnotCall) get_knot_curve,
                                                                           (GetMultCall) get_mult_curve,
                                                                           (GetIntCall) get_degree_curve);
}

std::vector<Handle(Geom_BSplineSurface) > CTiglBSplineAlgorithms::createCommonKnotsVectorSurface(const std::vector<Handle(Geom_BSplineSurface) >& old_surfaces_vector)
{
    // all B-spline surfaces must have the same parameter range in u- and v-direction

    // create a copy of the old B-spline surfaces in order to get no shadow effects
    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector;

    // first create a common knot vector in u-direction
    surfaces_vector = createCommonKnotsVectorImpl<Handle(Geom_BSplineSurface)>(old_surfaces_vector, (KnotInsertionCall)knot_insertion_u, (GetIntCall)get_NbKnots_u, (GetKnotCall)get_knot_u, (GetMultCall)get_knot_u_mult, (GetIntCall)get_degree_u);

    // now create a common knot vector in v-direction
    surfaces_vector = createCommonKnotsVectorImpl<Handle(Geom_BSplineSurface)>(surfaces_vector, (KnotInsertionCall)knot_insertion_v, (GetIntCall)get_NbKnots_v, (GetKnotCall)get_knot_v, (GetMultCall)get_knot_v_mult, (GetIntCall)get_degree_v);

    return surfaces_vector;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::skinnedBSplineSurfaceParams(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector, const Handle(TColStd_HArray1OfReal) v_parameters, bool is_closed_v)
{
    // check amount of given parameters
    if (v_parameters->Length() != splines_vector.size()) {
        throw CTiglError("The amount of given parameters has to be equal to the amount of given B-splines!", TIGL_MATH_ERROR);
    }

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

    Handle(Geom_BSplineSurface) skinnedSurface = new Geom_BSplineSurface(new_controlPoints, knots_u, knots_v, mults_u, mults_v, new_degree, degree_v);

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

    // insert knots at intersections, so that one can reparametrize intervals between these intersections afterwards
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

    return copied_spline;
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

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::createGordonSurface(const std::vector<Handle(Geom_BSplineCurve) >& compatible_splines_u_vector,
                                                                        const std::vector<Handle(Geom_BSplineCurve) >& compatible_splines_v_vector,
                                                                        const Handle(TColStd_HArray1OfReal) intersection_params_spline_u,
                                                                        const Handle(TColStd_HArray1OfReal) intersection_params_spline_v)
{
    // check whether there are any u-directional and v-directional B-splines in the vectors
    if (compatible_splines_u_vector.size() == 0) {
        throw CTiglError("There are no u-directional B-splines!");
    }

    if (compatible_splines_v_vector.size() == 0) {
        throw CTiglError("There are no v-directional B-splines!");
    }

    // check B-spline parametrization (should be from 0 to 1):
    for (unsigned int spline_u_idx = 0; spline_u_idx < compatible_splines_u_vector.size(); ++spline_u_idx) {
        TColStd_Array1OfReal knots(1, compatible_splines_u_vector[spline_u_idx]->NbKnots());
        compatible_splines_u_vector[spline_u_idx]->Knots(knots);
        assert(std::abs(knots(1)) < 1e-15);
        assert(std::abs(knots(compatible_splines_u_vector[spline_u_idx]->NbKnots()) - 1) < 1e-15);
    }

    for (unsigned int spline_v_idx = 0; spline_v_idx < compatible_splines_v_vector.size(); ++spline_v_idx) {
        TColStd_Array1OfReal knots(1, compatible_splines_v_vector[spline_v_idx]->NbKnots());
        compatible_splines_v_vector[spline_v_idx]->Knots(knots);
        assert(std::abs(knots(1)) < 1e-15);
        assert(std::abs(knots(compatible_splines_v_vector[spline_v_idx]->NbKnots()) - 1) < 1e-15);
    }

    // bring the splines and the intersection parameters in the right order
    // for the u-directional B-splines
    std::pair<Handle(TColStd_HArray1OfReal), std::vector<Handle(Geom_BSplineCurve)> > params_spline_u_pair = CTiglBSplineAlgorithms::sortBSplines(intersection_params_spline_v, compatible_splines_u_vector);
    Handle(TColStd_HArray1OfReal) sorted_intersection_params_v = params_spline_u_pair.first;
    std::vector<Handle(Geom_BSplineCurve)> sorted_splines_u_vector = params_spline_u_pair.second;

    // for the v-directional B-splines
    std::pair<Handle(TColStd_HArray1OfReal), std::vector<Handle(Geom_BSplineCurve)> > params_spline_v_pair = CTiglBSplineAlgorithms::sortBSplines(intersection_params_spline_u, compatible_splines_v_vector);
    Handle(TColStd_HArray1OfReal) sorted_intersection_params_u = params_spline_v_pair.first;
    std::vector<Handle(Geom_BSplineCurve)> sorted_splines_v_vector = params_spline_v_pair.second;

    // find out the 'average' scale of the B-splines in order to being able to handle a more approximate dataset and find its intersections
    // u-directional splines
    double splines_scale = CTiglBSplineAlgorithms::scaleOfBSplines(sorted_splines_u_vector);

    // v-directional splines
    double splines_v_scale = CTiglBSplineAlgorithms::scaleOfBSplines(sorted_splines_v_vector);

    splines_scale = (splines_scale + splines_v_scale) / 2.;

    if (std::abs(sorted_intersection_params_u->Value(1)) > splines_scale * 1e-5 || std::abs(sorted_intersection_params_u->Value(sorted_intersection_params_u->Upper()) - 1) > splines_scale * 1e-5) {
        throw tigl::CTiglError("WARNING: B-splines in u-direction mustn't stick out, spline network must be 'closed'!");
    }

    if (std::abs(sorted_intersection_params_v->Value(1)) > splines_scale * 1e-5 || std::abs(sorted_intersection_params_v->Value(sorted_intersection_params_v->Upper()) - 1) > splines_scale * 1e-5) {
        throw tigl::CTiglError("WARNING: B-splines in v-direction mustn't stick out, spline network must be 'closed'!");
    }

    // the B-spline network must be compatible
    for (int u_param_idx = sorted_intersection_params_u->Lower(); u_param_idx <= sorted_intersection_params_u->Upper(); ++u_param_idx) {
        for (int v_param_idx = sorted_intersection_params_v->Lower(); v_param_idx <= sorted_intersection_params_v->Upper(); ++v_param_idx) {
            Handle(Geom_BSplineCurve) spline_u = sorted_splines_u_vector[v_param_idx - 1];
            double spline_u_param = sorted_intersection_params_u->Value(u_param_idx);
            Handle(Geom_BSplineCurve) spline_v = sorted_splines_v_vector[u_param_idx - 1];
            double spline_v_param = sorted_intersection_params_v->Value(v_param_idx);

            gp_Pnt point_spline_u = spline_u->Value(spline_u_param);
            gp_Pnt point_spline_v = spline_v->Value(spline_v_param);

            if (std::abs(point_spline_u.X() - point_spline_v.X()) > splines_scale * 1e-5 || std::abs(point_spline_u.Y() - point_spline_v.Y()) > splines_scale * 1e-5 || std::abs(point_spline_u.Z() - point_spline_v.Z()) > splines_scale * 1e-5) {
                throw tigl::CTiglError("B-spline network is incompatible (e.g. wrong parametrization) or intersection parameters are in a wrong order!");
            }
        }
    }

    // Skinning in v-direction with u directional B-Splines
    unsigned int degree_v_dir = sorted_splines_u_vector.size() - 1;
    Handle(Geom_BSplineSurface) surface_v_unmod = skinnedBSplineSurfaceParams(sorted_splines_u_vector, sorted_intersection_params_v, sorted_splines_v_vector[0]->IsClosed());
    // therefore reparametrization before this method

    // Skinning in u-direction with v directional B-Splines
    unsigned int degree_u_dir = sorted_splines_v_vector.size() - 1;
    Handle(Geom_BSplineSurface) surface_u_unmod_unflipped = skinnedBSplineSurfaceParams(sorted_splines_v_vector, sorted_intersection_params_u, sorted_splines_u_vector[0]->IsClosed());

    // flipping of the surface in v-direction; flipping is redundant here, therefore the next line is a comment!
    Handle(Geom_BSplineSurface) surface_u_unmod = flipSurface(surface_u_unmod_unflipped);

    // setting everything up for creating Tensor Product Surface by interpolating intersection points of profiles and guides with B-Spline surface
    // find the intersection points:
        unsigned int dim = 3;  // dimension of space
    TColgp_Array2OfPnt intersection_pnts(1, sorted_intersection_params_u->Upper(), 1, sorted_intersection_params_v->Upper());

    // use splines in u-direction to get intersection points
    for (unsigned int spline_idx = 0; spline_idx < sorted_splines_u_vector.size(); ++spline_idx) {
        for (int intersection_idx = sorted_intersection_params_u->Lower(); intersection_idx <= sorted_intersection_params_u->Upper(); ++intersection_idx) {
            Handle(Geom_BSplineCurve) spline_u = sorted_splines_u_vector[spline_idx];
            double parameter = sorted_intersection_params_u->Value(intersection_idx);
            intersection_pnts(intersection_idx, spline_idx + 1) = spline_u->Value(parameter);
        }
    }

    // if there are too little points for degree in u-direction = 3 and degree in v-direction=3 creating an interpolation B-spline surface isn't possible in Open CASCADE

    // Open CASCADE doesn't have a B-spline surface interpolation method where one can give the u- and v-directional parameters as arguments
    Handle(Geom_BSplineSurface) tensorProdSurf_unmod = CTiglBSplineAlgorithms::interpolatingSurface(intersection_pnts, sorted_intersection_params_u, sorted_intersection_params_v, sorted_splines_u_vector[0]->IsClosed(), sorted_splines_v_vector[0]->IsClosed());

    // match degree of all three surfaces
    // degree elevation in u- and v-direction
    // highest degree in  both directions
    Standard_Integer new_degree_u = std::max(surface_u_unmod->UDegree(), surface_v_unmod->UDegree());
    new_degree_u = std::max(new_degree_u, tensorProdSurf_unmod->UDegree());
    Standard_Integer new_degree_v = std::max(surface_u_unmod->VDegree(), surface_v_unmod->VDegree());
    new_degree_v = std::max(new_degree_v, tensorProdSurf_unmod->VDegree());

    // check whether degree elevation is necessary (does method elevate_degree_u()) and if yes, elevate degree
    surface_u_unmod->IncreaseDegree(new_degree_u, new_degree_v);
    surface_v_unmod->IncreaseDegree(new_degree_u, new_degree_v);
    tensorProdSurf_unmod->IncreaseDegree(new_degree_u, new_degree_v);

    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector_unmod;
    surfaces_vector_unmod.push_back(surface_u_unmod);
    surfaces_vector_unmod.push_back(surface_v_unmod);
    surfaces_vector_unmod.push_back(tensorProdSurf_unmod);

    // change small inaccuracies of the first knot:
    if (std::abs(surfaces_vector_unmod[0]->UKnot(1)) < 1e-5) {
        surfaces_vector_unmod[0]->SetUKnot(1, 0);
    }
    if (std::abs(surfaces_vector_unmod[1]->UKnot(1)) < 1e-5) {
        surfaces_vector_unmod[1]->SetUKnot(1, 0);
    }
    if (std::abs(surfaces_vector_unmod[2]->UKnot(1)) < 1e-5) {
        surfaces_vector_unmod[2]->SetUKnot(1, 0);
    }

    // create common knot vector for all three surfaces
    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector = createCommonKnotsVectorSurface(surfaces_vector_unmod);

    Handle(Geom_BSplineSurface) surface_u = surfaces_vector[0];
    Handle(Geom_BSplineSurface) surface_v = surfaces_vector[1];
    Handle(Geom_BSplineSurface) tensorProdSurf = surfaces_vector[2];

    // creating the Gordon Surface
    // Add control points of the two skinned surfaces (skinned in v-direction (surface_u) and skinned in u-direction
    // (surface_v)) and subtract control point of the Tensor Product Surface (surface interpolating intersection
    // points of profile splines with guide splines):
    // All surfaces have the same amount of control points and are in the SAME ORDER, therefore the next step is
    // possible

    TColgp_Array2OfPnt gordonCps(1, surface_u->NbUPoles(), 1, surface_u->NbVPoles());
    for (int cp_u_idx = 1; cp_u_idx <= surface_u->NbUPoles(); ++cp_u_idx) {
        for (int cp_v_idx = 1; cp_v_idx <= surface_u->NbVPoles(); ++cp_v_idx) {
            gp_Pnt cp_surf_u = surface_u->Pole(cp_u_idx, cp_v_idx);
            gp_Pnt cp_surf_v = surface_v->Pole(cp_u_idx, cp_v_idx);
            gp_Pnt cp_tensor = tensorProdSurf->Pole(cp_u_idx, cp_v_idx);

            gp_Pnt gordon_cp(cp_surf_u.X() + cp_surf_v.X() - cp_tensor.X(), cp_surf_u.Y() + cp_surf_v.Y() - cp_tensor.Y(), cp_surf_u.Z() + cp_surf_v.Z() - cp_tensor.Z());
            gordonCps(cp_u_idx, cp_v_idx) = gordon_cp;
        }
    }


    TColStd_Array1OfReal knots_u_surf(1, surface_u->NbUKnots());
    surface_u->UKnots(knots_u_surf);
    TColStd_Array1OfInteger mults_u_surf(1, surface_u->NbUKnots());
    surface_u->UMultiplicities(mults_u_surf);

    TColStd_Array1OfReal knots_v_surf(1, surface_u->NbVKnots());
    surface_u->VKnots(knots_v_surf);
    TColStd_Array1OfInteger mults_v_surf(1, surface_u->NbVKnots());
    surface_u->VMultiplicities(mults_v_surf);

    Handle(Geom_BSplineSurface) gordonSurface = new Geom_BSplineSurface(gordonCps, knots_u_surf, knots_v_surf, mults_u_surf, mults_v_surf, surface_u->UDegree(), surface_u->VDegree());

    return gordonSurface;
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
    }

    return intersection_params_vector;
}

std::pair<Handle(TColStd_HArray1OfReal), std::vector<Handle(Geom_BSplineCurve)> > CTiglBSplineAlgorithms::sortBSplines(
                                                                                     const Handle(TColStd_HArray1OfReal) parameters,
                                                                                     const std::vector<Handle(Geom_BSplineCurve)>& splines_vector) {
    std::vector<std::pair<unsigned int, double> > idx_params_pairs;
    for (unsigned int i = 1; i <= (unsigned int) parameters->Length(); ++i) {
        idx_params_pairs.push_back(std::make_pair(i, parameters->Value(i)));
    }

    std::sort(idx_params_pairs.begin(), idx_params_pairs.end(), pairCompare);

    std::vector<Handle(Geom_BSplineCurve)> sorted_splines_vector;
    Handle(TColStd_HArray1OfReal) sorted_params = new TColStd_HArray1OfReal(1, splines_vector.size());
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        unsigned int sorted_idx = idx_params_pairs[spline_idx].first - 1;
        sorted_splines_vector.push_back(splines_vector[sorted_idx]);

        double sorted_parameter = idx_params_pairs[spline_idx].second;
        sorted_params->SetValue(spline_idx + 1, sorted_parameter);
    }

    return std::make_pair(sorted_params, sorted_splines_vector);
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
    // now the parameter range of all u-directional B-splines should be [0, 1]

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
    // now the parameter range of all v-directional B-splines should be [0, 1]

    // now find all intersections of all B-splines with each other
    std::vector<std::vector<double> > intersection_params_u(splines_u_vector.size(), std::vector<double> (splines_v_vector.size()));
    std::vector<std::vector<double> > intersection_params_v(splines_v_vector.size(), std::vector<double> (splines_u_vector.size()));

    for (unsigned int spline_u_idx = 0; spline_u_idx < splines_u_vector.size(); ++spline_u_idx) {
        for (unsigned int spline_v_idx = 0; spline_v_idx < splines_v_vector.size(); ++spline_v_idx) {
            std::vector<std::pair<double, double> > intersection_params_vector = CTiglBSplineAlgorithms::intersectionFinder(splines_u_vector[spline_u_idx], splines_v_vector[spline_v_idx]);

            if (intersection_params_vector.size() < 1) {
                throw tigl::CTiglError("U-directional B-spline and v-directional B-spline don't intersect each other!");
            }

            if (intersection_params_vector.size() > 1) {
                // for closed curves
                if (intersection_params_vector.size() == 2) {
                    gp_Pnt intersection_point1_u = splines_u_vector[spline_u_idx]->Value(intersection_params_vector[0].first);
                    gp_Pnt intersection_point2_u = splines_u_vector[spline_u_idx]->Value(intersection_params_vector[1].first);

                    gp_Pnt intersection_point1_v = splines_v_vector[spline_v_idx]->Value(intersection_params_vector[0].second);
                    gp_Pnt intersection_point2_v = splines_v_vector[spline_v_idx]->Value(intersection_params_vector[1].second);

                    // if u-directional or v-directional splines are closed
                    if ((std::abs(intersection_params_vector[0].first - intersection_params_vector[1].first) > 1e-7 && std::abs(intersection_point1_u.X() - intersection_point2_u.X()) < 1e-10 && std::abs(intersection_point1_u.Y() - intersection_point2_u.Y()) < 1e-10 && std::abs(intersection_point1_u.Z() - intersection_point2_u.Z()) < 1e-10) ||
                        (std::abs(intersection_params_vector[0].second - intersection_params_vector[1].second) > 1e-7 && std::abs(intersection_point1_v.X() - intersection_point2_v.X()) < 1e-10 && std::abs(intersection_point1_v.Y() - intersection_point2_v.Y()) < 1e-10 && std::abs(intersection_point1_v.Z() - intersection_point2_v.Z()) < 1e-10)) {

                        if (splines_u_vector[0]->IsClosed() && spline_u_idx == splines_u_vector.size() ||
                            splines_v_vector[0]->IsClosed() && spline_v_idx == splines_v_vector.size()) {
                            // delete first element
                            intersection_params_vector[0] = intersection_params_vector[1];
                        }
                        intersection_params_vector.pop_back();
                    }
                }
//                // TODO: both u-directional splines and v-directional splines are closed
//               else if (intersection_params_vector.size() == 4) {

//                }
                else {
                    throw tigl::CTiglError("U-directional B-spline and v-directional B-spline have more than one intersection with each other!");
                }
            }

            intersection_params_u[spline_u_idx][spline_v_idx] = intersection_params_vector[0].first;
            intersection_params_v[spline_v_idx][spline_u_idx] = intersection_params_vector[0].second;
        }
    }

    // eliminate small inaccuracies of the intersection parameters:
    // first intersection
    for (unsigned int spline_u_idx = 0; spline_u_idx < splines_u_vector.size(); ++spline_u_idx) {
        if (std::abs(intersection_params_u[spline_u_idx][0] - splines_u_vector[0]->Knot(1)) < 0.001) {
            intersection_params_u[spline_u_idx][0] = splines_u_vector[0]->Knot(1);
        }
    }

    for (unsigned int spline_v_idx = 0; spline_v_idx < splines_v_vector.size(); ++spline_v_idx) {
        if (std::abs(intersection_params_v[spline_v_idx][0] - splines_v_vector[0]->Knot(1)) < 0.001) {
            intersection_params_v[spline_v_idx][0] = splines_v_vector[0]->Knot(1);
        }
    }

    // last intersection
    for (unsigned int spline_u_idx = 0; spline_u_idx < splines_u_vector.size(); ++spline_u_idx) {
        if (std::abs(intersection_params_u[spline_u_idx][splines_v_vector.size() - 1] - splines_u_vector[0]->Knot(splines_u_vector[0]->NbKnots())) < 0.001) {
            intersection_params_u[spline_u_idx][splines_v_vector.size() - 1] = splines_u_vector[0]->Knot(splines_u_vector[0]->NbKnots());
        }
    }

    for (unsigned int spline_v_idx = 0; spline_v_idx < splines_v_vector.size(); ++spline_v_idx) {
        if (std::abs(intersection_params_v[spline_v_idx][splines_u_vector.size() - 1] - splines_v_vector[0]->Knot(splines_v_vector[0]->NbKnots())) < 0.001) {
            intersection_params_v[spline_v_idx][splines_u_vector.size() - 1] = splines_v_vector[0]->Knot(splines_v_vector[0]->NbKnots());
        }
    }

    TColStd_Array1OfReal average_intersection_params_u(1, splines_v_vector.size());
    for (unsigned int spline_v_idx = 1; spline_v_idx <= splines_v_vector.size(); ++spline_v_idx) {
        double sum = 0;
        for (unsigned int spline_u_idx = 1; spline_u_idx <= splines_u_vector.size(); ++spline_u_idx) {
            sum += intersection_params_u[spline_u_idx - 1][spline_v_idx - 1];
        }
        average_intersection_params_u(spline_v_idx) = sum / splines_u_vector.size();
    }

    TColStd_Array1OfReal average_intersection_params_v(1, splines_u_vector.size());
    for (unsigned int spline_u_idx = 1; spline_u_idx <= splines_u_vector.size(); ++spline_u_idx) {
        double sum = 0;
        for (unsigned int spline_v_idx = 1; spline_v_idx <= splines_v_vector.size(); ++spline_v_idx) {
            sum += intersection_params_v[spline_v_idx - 1][spline_u_idx - 1];
        }
        average_intersection_params_v(spline_u_idx) = sum / splines_v_vector.size();
    }


    // convert types of average_intersection_params
    Handle(TColStd_HArray1OfReal) av_intersection_params_u = new TColStd_HArray1OfReal(1, average_intersection_params_u.Length());
    for (int i = 1; i <= average_intersection_params_u.Length(); ++i) {
        av_intersection_params_u->SetValue(i, average_intersection_params_u(i));
    }

    Handle(TColStd_HArray1OfReal) av_intersection_params_v = new TColStd_HArray1OfReal(1, average_intersection_params_v.Length());
    for (int i = 1; i <= average_intersection_params_v.Length(); ++i) {
        av_intersection_params_v->SetValue(i, average_intersection_params_v(i));
    }
    // sort u-directional B-splines and their corresponding parameters
    std::pair<Handle(TColStd_HArray1OfReal), std::vector<Handle(Geom_BSplineCurve)> > params_spline_u_pair = CTiglBSplineAlgorithms::sortBSplines(av_intersection_params_v, splines_u_vector);
    Handle(TColStd_HArray1OfReal) sorted_av_intersection_params_v = params_spline_u_pair.first;
    std::vector<Handle(Geom_BSplineCurve)> sorted_splines_u_vector = params_spline_u_pair.second;

    // sort v-directional B-splines and their corresponding parameters
    std::pair<Handle(TColStd_HArray1OfReal), std::vector<Handle(Geom_BSplineCurve)> > params_spline_v_pair = CTiglBSplineAlgorithms::sortBSplines(av_intersection_params_u, splines_v_vector);
    Handle(TColStd_HArray1OfReal) sorted_av_intersection_params_u = params_spline_v_pair.first;
    std::vector<Handle(Geom_BSplineCurve)> sorted_splines_v_vector = params_spline_v_pair.second;

    // now save sorted parameters in average_intersection_params_u and average_intersection_params_v
    for (int i = 1; i <= sorted_av_intersection_params_u->Length(); ++i) {
        average_intersection_params_u(i) = sorted_av_intersection_params_u->Value(i);
    }

    for (int i = 1; i <= sorted_av_intersection_params_v->Length(); ++i) {
        average_intersection_params_v(i) = sorted_av_intersection_params_v->Value(i);
    }

    // reparametrize u-directional B-splines
    std::vector<Handle(Geom_BSplineCurve)> reparam_splines_u;
    for (unsigned int spline_u_idx = 0; spline_u_idx < sorted_splines_u_vector.size(); ++spline_u_idx) {

        TColStd_Array1OfReal old_parameters(1, sorted_splines_v_vector.size());
        for (unsigned int spline_v_idx = 1; spline_v_idx <= sorted_splines_v_vector.size(); ++spline_v_idx) {
            old_parameters(spline_v_idx) = intersection_params_u[spline_u_idx][spline_v_idx - 1];
        }

        Handle(Geom_BSplineCurve) reparam_spline_u = CTiglBSplineAlgorithms::reparametrizeBSpline(sorted_splines_u_vector[spline_u_idx], old_parameters, average_intersection_params_u);
        reparam_splines_u.push_back(reparam_spline_u);
    }

    // reparametrize v-directional B-splines
    std::vector<Handle(Geom_BSplineCurve)> reparam_splines_v;
    for (unsigned int spline_v_idx = 0; spline_v_idx < sorted_splines_v_vector.size(); ++spline_v_idx) {

        TColStd_Array1OfReal old_parameters(1, sorted_splines_u_vector.size());
        for (unsigned int spline_u_idx = 1; spline_u_idx <= sorted_splines_u_vector.size(); ++spline_u_idx) {
            old_parameters(spline_u_idx) = intersection_params_v[spline_v_idx][spline_u_idx - 1];
        }

        Handle(Geom_BSplineCurve) reparam_spline_v = CTiglBSplineAlgorithms::reparametrizeBSpline(sorted_splines_v_vector[spline_v_idx], old_parameters, average_intersection_params_v);
        reparam_splines_v.push_back(reparam_spline_v);
    }

    // splines are reparametrized, but don't have common knot vector yet, in this sense not compatible

    Handle(Geom_BSplineSurface) gordonSurface = CTiglBSplineAlgorithms::createGordonSurface(reparam_splines_u, reparam_splines_v, sorted_av_intersection_params_u, sorted_av_intersection_params_v);

    return gordonSurface;
}
} // namespace tigl
