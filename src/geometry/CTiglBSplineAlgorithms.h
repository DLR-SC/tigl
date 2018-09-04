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

#ifndef CTIGLBSPLINEALGORITHMS_H
#define CTIGLBSPLINEALGORITHMS_H

#include "tigl_internal.h"

#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HArray1OfReal.hxx>

#include <utility>
#include <vector>
#include <math_Matrix.hxx>

#include <BSplCLib_EvaluatorFunction.hxx>

namespace tigl
{

class CTiglBSplineAlgorithms
{
public:

    /// Tolerance for closed curve detection
    TIGL_EXPORT static const double REL_TOL_CLOSED;

    /**
     * @brief computeParamsBSplineCurve:
     *          Computes the parameters of a Geom_BSplineCurve at the given points
     * @param points:
     *          Given points where new parameters are computed at
     * @param alpha:
     *          Exponent for the computation of the parameters; alpha=0.5 means, that this method uses the centripetal method
     */
    TIGL_EXPORT static std::vector<double> computeParamsBSplineCurve(const Handle(TColgp_HArray1OfPnt)& points, double alpha=0.5);

    /**
     * @brief computeParamsBSplineCurve:
     *          Computes the parameters of a Geom_BSplineCurve at the given points
     * @param points:
     *          Given points where new parameters are computed at
     * @param alpha:
     *          Exponent for the computation of the parameters; alpha=0.5 means, that this method uses the centripetal method
     * @param umin:
     *          First parameter of the curve
     * @param umax:
     *          Last parameter of the curve
     */
    TIGL_EXPORT static std::vector<double> computeParamsBSplineCurve(const Handle(TColgp_HArray1OfPnt)& points, double umin, double umax, double alpha=0.5);
    
    /**
     * @brief Computes a full blown bspline basis matrix of size (params.Length(), flatKnots.Length() + degree + 1)
     * @param degree    Degree of the bspline
     * @param flatKnots Flatted know vector
     * @param params    Parameters of B-Spline evaluation
     * @return          The B-spline matrix
     */
    TIGL_EXPORT static math_Matrix bsplineBasisMat(int degree, const TColStd_Array1OfReal& flatKnots, const TColStd_Array1OfReal& params, unsigned int derivOrder=0);

    /**
     * @brief computeParamsBSplineSurf:
     *          Computes the parameters of a Geom_BSplineSurface at the given points
     * @param points:
     *          Given points where new parameters are computed at
     * @param alpha:
     *          Exponent for the computation of the parameters; alpha=0.5 means, that this method uses the centripetal method
     * @return  a std::pair of Handle(TColStd_HArray1OfReal) of the parameters in u- and in v-direction
     */
    TIGL_EXPORT static std::pair<std::vector<double>, std::vector<double> >
    computeParamsBSplineSurf(const TColgp_Array2OfPnt& points, double alpha=0.5);

    /**
     * @brief Matches the parameter range of all b-splines to the parameter range of the first b-spline
     *
     * @param bsplines The splines to be matched (in/out)
     */
    TIGL_EXPORT static void matchParameterRange(const std::vector<Handle(Geom_BSplineCurve) >& bsplines, double tolerance=1e-15);

    /**
     * @brief Matches the degree of all b-splines by raising the degree to the maximum degree
     * 
     * @param bsplines The splines to be matched (in/out)
     */
    TIGL_EXPORT static void matchDegree(const std::vector<Handle(Geom_BSplineCurve) >& bsplines);
    
    /**
     * @brief createCommonKnotsVectorCurve:
     *          Creates a common knots vector of the given vector of B-splines
     *          The common knot vector contains all knots of all splines with the highest multiplicity of all splines.
     * @param splines_vector:
     *          vector of B-splines that could have a different knot vector
     * @return the given vector of B-splines with a common knot vector, the B-spline geometry isn't changed
     */
    TIGL_EXPORT static std::vector<Handle(Geom_BSplineCurve)> createCommonKnotsVectorCurve(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector, double tol);

    /**
     * @brief createCommonKnotsVectorSurface:
     *          Creates a common knot vector in both u- and v-direction of the given vector of B-spline surfaces
     *          The common knot vector contains all knots in u- and v-direction of all surfaces with the highest multiplicity of all surfaces.
     *          !!! This method calls the method createCommonKnotsVectorSurfaceOneDir two times to create a common knot vector in both directions !!!
     * @param old_surfaces_vector:
     *          the given vector of B-spline surfaces that could have a different knot vector in u- and v-direction
     * @return
     *          the given vector of B-spline surfaces, now with a common knot vector
     *          The B-spline surface geometry remains the same.
     */
    TIGL_EXPORT static std::vector<Handle(Geom_BSplineSurface) > createCommonKnotsVectorSurface(const std::vector<Handle(Geom_BSplineSurface)>& old_surfaces_vector);

    /**
     */
    TIGL_EXPORT static void reparametrizeBSpline(Geom_BSplineCurve& spline, double umin, double umax, double tol=1e-15);

    /**
     * @brief reparametrizeBSplineContinuouslyApprox:
     *          Reparametrizes a given B-spline by giving an array of its old parameters that should have the values of the given array of new parameters after this function call.
     *          The B-spline geometry remains approximately the same, and:
     *          After this reparametrization the spline is continuously differentiable considering its parametrization
     * @param old_parameters:
     *          array of the old parameters that shall have the values of the new parameters
     * @param new_parameters:
     *          array of the new parameters the old parameters should become
     * @return
     *          the continuously reparametrized given B-spline
     */
    TIGL_EXPORT static Handle(Geom_BSplineCurve) reparametrizeBSplineContinuouslyApprox(const Handle(Geom_BSplineCurve) spline, const std::vector<double>& old_parameters,
                                                                                        const std::vector<double>& new_parameters, size_t n_control_pnts);

    /**
     * @brief flipSurface:
     *          swaps axes of the given surface, i.e., surface(u-coord, v-coord) becomes surface(v-coord, u-coord)
     * @param surface:
     *          B-spline surface that shall be flipped
     * @return
     *          the given surface, but flipped
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) flipSurface(const Handle(Geom_BSplineSurface) surface);

    /**
     * @brief pointsToSurface:
     *          interpolates a matrix of points by a B-spline surface with parameters in u- and in v-direction where the points shall be at
     *          ! Uses a skinned surface !
     * @param points:
     *          matrix of points that shall be interpolated
     * @param uParams:
     *          parameters in u-direction where the points shall be at on the interpolating surface
     * @param vParams:
     *          parameters in v-direction where the points shall be at on the interpolating surface
     * @param uContinousIfClosed:
     *          Make a continous junction in u d-directions, if the u direction is closed
     * @param vContinousIfClosed:
     *          Make a continous junction in v d-directions, if the v direction is closed
     * @return
     *          B-spline surface which interpolates the given points with the given parameters
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) pointsToSurface(const TColgp_Array2OfPnt& points,
                                                                   const std::vector<double>& uParams,
                                                                   const std::vector<double>& vParams,
                                                                   bool uContinousIfClosed, bool vContinousIfClosed);

    /**
     * @brief intersections:
     *          Returns all intersections of two B-splines
     * @param spline1:
     *          first B-spline
     * @param spline2:
     *          second B-spline
     * @param tolerance
     *          relative tolerance to check intersection (relative to overall size)
     * @return:
     *          intersections of spline1 with spline2 as a vector of (parameter of spline1, parameter of spline2)-pairs
     */
    TIGL_EXPORT static std::vector<std::pair<double, double> > intersections(const Handle(Geom_BSplineCurve) spline1, const Handle(Geom_BSplineCurve) spline2, double tolerance=3e-4);

    /**
     * @brief scale:
     *          Returns the approximate scale of the biggest given B-spline curve
     * @param splines_vector:
     *          vector of B-spline curves
     * @return:
     *          the scale
     */
    TIGL_EXPORT static double scale(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector);

    /**
     * @brief scale:
     *          Returns the approximate scale of the B-spline curve
     * @param spline:
     *          B-spline curve
     * @return:
     *          the scale
     */
    TIGL_EXPORT static double scale(const Handle(Geom_BSplineCurve)& spline);

    /// Returns the scale of the point matrix
    TIGL_EXPORT static double scale(const TColgp_Array2OfPnt& points);

    /**
     * Returns positions, where the curve has kinks (C1 Discontinuities)
     */
    TIGL_EXPORT static std::vector<double> getKinkParameters(const Handle(Geom_BSplineCurve)& curve);

    /// Checks, whether the point matrix points is closed in u direction
    TIGL_EXPORT static bool isUDirClosed(const TColgp_Array2OfPnt& points, double tolerance);

    /// Checks, whether the point matrix points is closed in u direction
    TIGL_EXPORT static bool isVDirClosed(const TColgp_Array2OfPnt& points, double tolerance);

    /**
     * Computes the knot vector for curve interpolation using parameter averaging
     * This is required to prevent singular systems during interpolation.
     */
    TIGL_EXPORT static std::vector<double> knotsFromCurveParameters(std::vector<double>& params, unsigned int degree, bool closedCurve);

    /// Trims a bspline surface
    TIGL_EXPORT static Handle(Geom_BSplineSurface) trimSurface(const Handle(Geom_Surface)& surface, double umin, double umax, double vmin, double vmax);
};
} // namespace tigl

#endif // CTIGLBSPLINEALGORITHMS_H
