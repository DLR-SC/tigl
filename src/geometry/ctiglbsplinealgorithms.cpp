#include <CTiglBSplineAlgorithms.h>
#include <CTiglError.h>
#include <CSharedPtr.h>

#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColgp_HArray1OfPnt.hxx>

#include <cmath>

#include <stdexcept>
#include <algorithm>
#include <cassert>


// helper function for std::unique
bool helper_function_unique(double a, double b)
{
    return (std::abs(a - b) < 1e-15);
}

// helper functions for method createCommonKnotsVectorSurfaceOneDir

// typedef knotInsertionCall
void knot_insertion_u(const Handle(Geom_BSplineSurface) surface, double knot_u, int mult)
{
    double tolerance = 1e-15;
    surface->InsertUKnot(knot_u, mult, tolerance);
}

void knot_insertion_v(const Handle(Geom_BSplineSurface) surface, double knot_v, int mult)
{
    double tolerance = 1e-15;
    surface->InsertVKnot(knot_v, mult, tolerance);
}

// typedef getKnotCall
double get_knot_u(const Handle(Geom_BSplineSurface) surface, int index)
{
    return surface->UKnot(index);
}

double get_knot_v(const Handle(Geom_BSplineSurface) surface, int index)
{
    return surface->VKnot(index);
}

// typedef getMultCall
int get_knot_u_mult(const Handle(Geom_BSplineSurface) surface, int index)
{
    return surface->UMultiplicity(index);
}

int get_knot_v_mult(const Handle(Geom_BSplineSurface) surface, int index)
{
    return surface->VMultiplicity(index);
}


// typedef getIntCall
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


namespace tigl
{


CTiglBSplineAlgorithms::CTiglBSplineAlgorithms()
{

}


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


std::vector<Handle(Geom_BSplineCurve)> CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(const std::vector<Handle(Geom_BSplineCurve)>& old_splines_vector)
{
    // all splines must have the same parameter range
    double begin_param = old_splines_vector[0]->Knot(1);
    double end_param = old_splines_vector[0]->Knot(old_splines_vector[0]->NbKnots());
    for (unsigned int spline_idx = 0; spline_idx < old_splines_vector.size(); ++spline_idx) {
        double begin_param_spline = old_splines_vector[spline_idx]->Knot(1);
        double end_param_spline = old_splines_vector[spline_idx]->Knot(old_splines_vector[spline_idx]->NbKnots());
        if (std::abs(begin_param_spline - begin_param) > 1e-15 || std::abs(end_param_spline - end_param) > 1e-15) {
            throw CTiglError("ERROR: B-splines don't have the same parameter range in method createCommonKnotsVectorCurve!");
        }
    }

    // all B-splines must have the same degree
    int degree = old_splines_vector[0]->Degree();
    for (unsigned int spline_idx = 0; spline_idx < old_splines_vector.size(); ++spline_idx) {
        if (old_splines_vector[spline_idx]->Degree() != degree) {
            throw CTiglError("ERROR: B-splines don't have the same degree in method createCommonKnotsVectorCurve!");
        }
    }

    // create a copy of the old B-splines in order to get no shadow effects
    std::vector<Handle(Geom_BSplineCurve)> splines_vector;
    for (unsigned int spline_idx = 0; spline_idx < old_splines_vector.size(); ++spline_idx) {
        Handle(Geom_BSplineCurve) old_curve = old_splines_vector[spline_idx];
        Handle(Geom_BSplineCurve) new_curve = Handle(Geom_BSplineCurve)::DownCast(old_curve->Copy());
        splines_vector.push_back(new_curve);
    }

    // create a vector of all knots of all B-splines
    std::vector<double> all_knots;
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        for (int knot_idx = 1; knot_idx <= splines_vector[spline_idx]->NbKnots(); ++knot_idx) {
            double knot = splines_vector[spline_idx]->Knot(knot_idx);
            all_knots.push_back(knot);
        }
    }

    // sort vector of all knots of all B-splines
    std::sort(all_knots.begin(), all_knots.end());

    // delete duplicate knots, so that in all_knots are all unique knots
    all_knots.erase(std::unique(all_knots.begin(), all_knots.end(), helper_function_unique), all_knots.end());

    // create matrix of multiplicities of all knots for all B-splines and initialize it with zeros
    std::vector<std::vector<int> > mult_vector(splines_vector.size(), std::vector<int> (all_knots.size(), 0));
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        bool is_there = false;
        for (unsigned int all_knot_idx = 0; all_knot_idx < all_knots.size(); ++all_knot_idx) {
            int mult = 0;
            for (int spline_knot_idx = 1; spline_knot_idx <= splines_vector[spline_idx]->NbKnots(); ++spline_knot_idx) {
                if (std::abs(splines_vector[spline_idx]->Knot(spline_knot_idx) - all_knots[all_knot_idx]) < 1e-15) {
                    is_there = true;
                    mult = splines_vector[spline_idx]->Multiplicity(spline_knot_idx);
                }
            }
            if (is_there) {  // otherwise multiplicity remains zero
                    mult_vector[spline_idx][all_knot_idx] = mult;
            }
        }
    }

    // find highest multiplicities of all knots of all splines
    std::vector<int> end_mult_vector(all_knots.size(), 0);
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        for (unsigned int all_knot_idx = 0; all_knot_idx < all_knots.size(); ++all_knot_idx) {
            if (end_mult_vector[all_knot_idx] < mult_vector[spline_idx][all_knot_idx]) {
                // if this is false for all splines multiplicity is zero
                end_mult_vector[all_knot_idx] = mult_vector[spline_idx][all_knot_idx];
            }
        }
    }

    // now insert missing knots in all splines
    for (unsigned int spline_idx = 0; spline_idx < splines_vector.size(); ++spline_idx) {
        for (unsigned int all_knot_idx = 0; all_knot_idx < all_knots.size(); ++all_knot_idx) {
            int diff = end_mult_vector[all_knot_idx] - mult_vector[spline_idx][all_knot_idx];  // always >= 0
            double knot = all_knots[all_knot_idx];
            splines_vector[spline_idx]->InsertKnot(knot, diff);
        }
    }

    return splines_vector;
}


std::vector<Handle(Geom_BSplineSurface) > CTiglBSplineAlgorithms::createCommonKnotsVectorSurfaceOneDir(const std::vector<Handle(Geom_BSplineSurface) > old_surfaces_vector, knotInsertionCall insert_knot, getIntCall get_knot_number, getKnotCall get_knot, getMultCall get_mult, getIntCall get_degree)
{
    // all B-spline surfaces must have the same parameter range in the chosen direction
    double begin_param_dir = get_knot(old_surfaces_vector[0], 1);
    double end_param_dir = get_knot(old_surfaces_vector[0], get_knot_number(old_surfaces_vector[0]));
    for (unsigned int surface_idx = 0; surface_idx < old_surfaces_vector.size(); ++surface_idx) {
        double begin_param_dir_surface = get_knot(old_surfaces_vector[surface_idx], 1);
        double end_param_dir_surface = get_knot(old_surfaces_vector[surface_idx], get_knot_number(old_surfaces_vector[surface_idx]));
        if (std::abs(begin_param_dir_surface - begin_param_dir) > 1e-15 || std::abs(end_param_dir_surface - end_param_dir) > 1e-15) {
            throw CTiglError("ERROR: B-spline surfaces don't have the same parameter range at least in one direction (u / v) in method createCommonKnotsVectorSurfaceOneDir!");
        }
    }

    // all B-spline surfaces must have the same degree in the chosen direction
    int degree = get_degree(old_surfaces_vector[0]);
    for (unsigned int surface_idx = 0; surface_idx < old_surfaces_vector.size(); ++surface_idx) {
        if (get_degree(old_surfaces_vector[surface_idx]) != degree) {
            throw CTiglError("ERROR: B-spline surfaces don't have the same degree at least in one direction (u / v) in method createCommonKnotsVectorSurfaceOneDir!");
        }
    }


    // create a copy of the old B-spline surfaces in order to get no shadow effects
    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector;
    for (unsigned int surface_idx = 0; surface_idx < old_surfaces_vector.size(); ++surface_idx) {
        Handle(Geom_BSplineSurface) old_surface = old_surfaces_vector[surface_idx];
        Handle(Geom_BSplineSurface) new_surface = Handle(Geom_BSplineSurface)::DownCast(old_surface->Copy());
        surfaces_vector.push_back(new_surface);
    }

    // create a vector of all knots in chosen direction (u or v) of all surfaces
    std::vector<double> all_knots_dir;
    for (unsigned int surface_idx = 0; surface_idx < surfaces_vector.size(); ++surface_idx) {
        for (int knot_idx = 1; knot_idx <= get_knot_number(surfaces_vector[surface_idx]); ++knot_idx) {
            double dir_knot = get_knot(surfaces_vector[surface_idx], knot_idx);
            all_knots_dir.push_back(dir_knot);
        }
    }

    // sort vector of all knots in given direction of all surfaces
    std::sort(all_knots_dir.begin(), all_knots_dir.end());

    // delete duplicate knots, so that in all_knots are all unique knots
    all_knots_dir.erase(std::unique(all_knots_dir.begin(), all_knots_dir.end(), helper_function_unique), all_knots_dir.end());

    // create matrix of multiplicities of all knots in given direction for all surfaces and initialize it with zeros
    std::vector<std::vector<int> > mult_vector_dir(surfaces_vector.size(), std::vector<int> (all_knots_dir.size(), 0));
    for (unsigned int surface_idx = 0; surface_idx < surfaces_vector.size(); ++surface_idx) {
        bool is_there = false;
        for (unsigned int all_knot_dir_idx = 0; all_knot_dir_idx < all_knots_dir.size(); ++all_knot_dir_idx) {
            int mult = 0;
            for (int surface_knot_dir_idx = 1; surface_knot_dir_idx <= get_knot_number(surfaces_vector[surface_idx]); ++surface_knot_dir_idx) {
                if (std::abs(get_knot(surfaces_vector[surface_idx], surface_knot_dir_idx) - all_knots_dir[all_knot_dir_idx]) < 1e-15) {
                    is_there = true;
                    mult = get_mult(surfaces_vector[surface_idx], surface_knot_dir_idx);
                }
            }
            if (is_there) {  // otherwise multiplicity remains zero
                    mult_vector_dir[surface_idx][all_knot_dir_idx] = mult;
            }
        }
    }

    // find highest multiplicities
    std::vector<int> end_mult_vector_dir(all_knots_dir.size(), 0);
    for (unsigned int surface_idx = 0; surface_idx < surfaces_vector.size(); ++surface_idx) {
        for (unsigned int all_knot_dir_idx = 0; all_knot_dir_idx < all_knots_dir.size(); ++all_knot_dir_idx) {
            if (end_mult_vector_dir[all_knot_dir_idx] < mult_vector_dir[surface_idx][all_knot_dir_idx]) {
                // if this is false for all splines multiplicity is zero
                end_mult_vector_dir[all_knot_dir_idx] = mult_vector_dir[surface_idx][all_knot_dir_idx];
            }
        }
    }

    // now insert missing knots in all surfaces
    for (unsigned int surface_idx = 0; surface_idx < surfaces_vector.size(); ++surface_idx) {
        for (unsigned int all_knot_dir_idx = 0; all_knot_dir_idx < all_knots_dir.size(); ++all_knot_dir_idx) {
            int diff = end_mult_vector_dir[all_knot_dir_idx] - mult_vector_dir[surface_idx][all_knot_dir_idx];  // always >= 0
            double knot = all_knots_dir[all_knot_dir_idx];
            insert_knot(surfaces_vector[surface_idx], knot, diff);
        }
    }

    return surfaces_vector;
}

std::vector<Handle(Geom_BSplineSurface) > CTiglBSplineAlgorithms::createCommonKnotsVectorSurface(const std::vector<Handle(Geom_BSplineSurface) >& old_surfaces_vector)
{
    // all B-spline surfaces must have the same parameter range in u- and v-direction

    // create a copy of the old B-spline surfaces in order to get no shadow effects
    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector;
    for (unsigned int surface_idx = 0; surface_idx < old_surfaces_vector.size(); ++surface_idx) {
        Handle(Geom_BSplineSurface) new_surface = Handle(Geom_BSplineSurface)::DownCast(old_surfaces_vector[surface_idx]->Copy());
        surfaces_vector.push_back(new_surface);
    }

    // first create a common knot vector in u-direction
    surfaces_vector = createCommonKnotsVectorSurfaceOneDir(surfaces_vector, (knotInsertionCall)knot_insertion_u, (getIntCall)get_NbKnots_u, (getKnotCall)get_knot_u, (getMultCall)get_knot_u_mult, (getIntCall)get_degree_u);

    // now create a common knot vector in v-direction
    surfaces_vector = createCommonKnotsVectorSurfaceOneDir(surfaces_vector, (knotInsertionCall)knot_insertion_v, (getIntCall)get_NbKnots_v, (getKnotCall)get_knot_v, (getMultCall)get_knot_v_mult, (getIntCall)get_degree_v);

    return surfaces_vector;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::skinnedBSplineSurfaceParams(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector, const Handle(TColStd_HArray1OfReal) v_parameters)
{
    // check amount of given parameters
    if (v_parameters->Length() != splines_vector.size()) {
        throw CTiglError("ERROR: The amount of given parameters has to be equal to the amount of given B-splines!");
    }

    // TODO: check that all splines live in a space with the same dimension

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
    TColgp_Array2OfPnt controlPoints(1, splines_vector[0]->NbPoles(), 1, splines_vector.size());
    for (unsigned int spline_idx = 1; spline_idx <= splines_vector.size(); ++spline_idx) {
        for (int point_idx = 1; point_idx <= splines_vector[0]->NbPoles(); ++point_idx) {
            controlPoints(point_idx, spline_idx) = splines_vector[spline_idx - 1]->Pole(point_idx);
        }
    }

    // again create spline to get knots and multiplicities in v-direction
    Handle(TColgp_HArray1OfPnt) controlPoints_spline_v = new TColgp_HArray1OfPnt(controlPoints.LowerCol(), controlPoints.UpperCol());
    for (int point_v_idx = controlPoints.LowerCol(); point_v_idx <= controlPoints.UpperCol(); ++point_v_idx) {
        controlPoints_spline_v->SetValue(point_v_idx, controlPoints(1, point_v_idx));
    }

    // create first spline that interoplates first column of control points in v-direction
    // separately in order to get knots_v and mults_v for creating the Geom_BSplineSurface:
    GeomAPI_Interpolate interpolationObject_v(controlPoints_spline_v, v_parameters, false, 1e-15);
    interpolationObject_v.Perform();

    // check that interpolation was successful
    assert(interpolationObject_v.IsDone());

    Handle(Geom_BSplineCurve) spline_v = interpolationObject_v.Curve();

    unsigned int degree_v = spline_v->Degree();

    for (int i = controlPoints.LowerCol(); i <= controlPoints.UpperCol(); ++i) {
        controlPoints(1, i) = spline_v->Pole(i);
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

        for (int i = controlPoints.LowerCol(); i <= controlPoints.UpperCol(); ++i) {
            controlPoints(point_u_idx, i) = spline->Pole(i);
        }

        // check degree always the same
        assert(degree_v == spline->Degree());
    }

    TColStd_Array1OfReal knots_v(1, spline_v->NbKnots());
    spline_v->Knots(knots_v);
    TColStd_Array1OfInteger mults_v(1, spline_v->NbKnots());
    spline_v->Multiplicities(mults_v);

    const Handle(Geom_BSplineCurve) firstCurve = splines_vector[0];
    TColStd_Array1OfReal knots_u(1, firstCurve->NbKnots());
    firstCurve->Knots(knots_u);
    TColStd_Array1OfInteger mults_u(1, firstCurve->NbKnots());
    firstCurve->Multiplicities(mults_u);
    Handle(Geom_BSplineSurface) skinnedSurface = new Geom_BSplineSurface(controlPoints, knots_u, knots_v, mults_u, mults_v, new_degree, degree_v);

    return skinnedSurface;
}

Handle(Geom_BSplineSurface) CTiglBSplineAlgorithms::skinnedBSplineSurface(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector)
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

    return CTiglBSplineAlgorithms::skinnedBSplineSurfaceParams(ready_splines_vector, v_parameters);
}

Handle(Geom_BSplineCurve) CTiglBSplineAlgorithms::reparametrizeBSpline(const Handle(Geom_BSplineCurve) spline, const TColStd_Array1OfReal& old_parameters, const TColStd_Array1OfReal& new_parameters)
{
    // create a copy of the given B-spline curve to avoid shadow effects
    Handle(Geom_BSplineCurve) copied_spline = Handle(Geom_BSplineCurve)::DownCast(spline->Copy());

    // insert knots at intersections, so that one can reparametrize intervals between these intersections afterwards
    for (int i = old_parameters.Lower(); i < old_parameters.Upper(); ++i) {
        // if between parameter range of B-spline
        if ((old_parameters(i) - copied_spline->Knot(1) > 1e-15) && (copied_spline->Knot(copied_spline->NbKnots())) - old_parameters(i)) {
            // insert knot degree-times
            copied_spline->InsertKnot(old_parameters(i), copied_spline->Degree());
        }
    }

    // move knots to reparametrize
    for (int interval_idx = old_parameters.Lower(); interval_idx <= old_parameters.Upper() - 1; ++interval_idx) {
        double interval_beginning_old = old_parameters(interval_idx);
        double interval_end_old = old_parameters(interval_idx + 1);

        double interval_beginning_new = new_parameters(interval_idx);
        double interval_end_new = new_parameters(interval_idx + 1);

        for (int knot_idx = 1; knot_idx <= copied_spline->NbKnots(); ++knot_idx) {
            double knot = copied_spline->Knot(knot_idx);
            if (interval_beginning_old <= knot && knot < interval_end_old) {
                copied_spline->SetKnot(knot_idx, (knot - interval_beginning_old) / (interval_end_old - interval_beginning_old) * (interval_end_new - interval_beginning_new) + interval_beginning_new, copied_spline->Multiplicity(knot_idx));
            }
        }
    }

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

} // namespace tigl
