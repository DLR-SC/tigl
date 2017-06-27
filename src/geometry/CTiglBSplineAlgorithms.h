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
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HArray1OfReal.hxx>

#include <utility>
#include <vector>

namespace tigl
{

class CTiglBSplineAlgorithms
{
public:

    /**
     * @brief computeParamsBSplineCurve:
     *          Computes the parameters of a Geom_BSplineCurve at the given points
     * @param points:
     *          Given points where new parameters are computed at
     * @param alpha:
     *          Exponent for the computation of the parameters; alpha=0.5 means, that this method uses the centripetal method
     * @param parameters
     *          reference of the TColStd_Array1OfReal parameters, that is created beforehand
     */
    TIGL_EXPORT static Handle(TColStd_HArray1OfReal) computeParamsBSplineCurve(const TColgp_Array1OfPnt& points, double alpha=0.5);


    /**
     * @brief computeParamsBSplineSurf:
     *          Computes the parameters of a Geom_BSplineSurface at the given points
     * @param points:
     *          Given points where new parameters are computed at
     * @param alpha:
     *          Exponent for the computation of the parameters; alpha=0.5 means, that this method uses the centripetal method
     * @return  a std::pair of Handle(TColStd_HArray1OfReal) of the parameters in u- and in v-direction
     */
    TIGL_EXPORT static std::pair<Handle(TColStd_HArray1OfReal), Handle(TColStd_HArray1OfReal) >
    computeParamsBSplineSurf(const TColgp_Array2OfPnt& points, double alpha=0.5);


    /**
     * @brief createCommonKnotsVectorCurve:
     *          Creates a common knots vector of the given vector of B-splines
     *          The common knot vector contains all knots of all splines with the highest multiplicity of all splines.
     * @param splines_vector:
     *          vector of B-splines that could have a different knot vector
     * @return the given vector of B-splines with a common knot vector, the B-spline geometry isn't changed
     */
    TIGL_EXPORT static std::vector<Handle(Geom_BSplineCurve)> createCommonKnotsVectorCurve(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector);

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
     * @brief skinnedBSplineSurfaceParams:
     *          Skinning of a vector of B-splines and creating a common knot vector for all the B-splines beforehand
     *          !!! Uses the method createCommonKnotsVectorCurve !!!
     * @param splines_vector:
     *          given vector of B-splines (which are parametrized in u-direction)
     * @param v_params
     *          array of parameters that each given B-spline should have as an isocurve of the created skinned surface
     * @return
     *          the created skinned surface
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) skinnedBSplineSurfaceParams(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector,
                                                                               const Handle(TColStd_HArray1OfReal) v_params, bool is_closed_v);

    /**
     * @brief skinnedBSplineSurfaceParams:
     *          Skinning of a vector of B-splines and creating a common knot vector for all the B-splines beforehand
     *          !!! Computes the parameters by the centripetal method which are used for the given B-splines for the skinned surface and then uses the method skinnedBSplineSurfaceParams !!!
     * @param splines_vector:
     *          given vector of B-splines (which are parametrized in u-direction)
     * @return
     *          the created skinned surface
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) skinnedBSplineSurface(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector, bool is_closed_v);

    /**
     * @brief reparametrizeBSpline:
     *        Reparametrizes a given B-spline by giving an array of its old parameters that should have the values of the given array of new parameters after this function call.
     *        The B-spline geometry remains the same, but:
     *        In general after this reparametrization the spline isn't continuously differentiable considering its parametrization anymore
     * @param spline:
     *          B-spline which shall be reparametrized
     * @param old_parameters:
     *          array of the old parameters that shall have the values of the new parameters
     * @param new_parameters:
     *          array of the new parameters the old parameters should become
     * @return
     *          the reparametrized given B-spline
     */
    TIGL_EXPORT static Handle(Geom_BSplineCurve) reparametrizeBSpline(const Handle(Geom_BSplineCurve) spline, const TColStd_Array1OfReal& old_parameters,
                                                                      const TColStd_Array1OfReal& new_parameters);

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
     * @brief interpolatingSurface:
     *          interpolates a matrix of points by a B-spline surface with parameters in u- and in v-direction where the points shall be at
     *          ! Uses a skinned surface !
     * @param points:
     *          matrix of points that shall be interpolated
     * @param parameters_u:
     *          parameters in u-direction where the points shall be at on the interpolating surface
     * @param parameters_v:
     *          parameters in v-direction where the points shall be at on the interpolating surface
     * @return
     *          B-spline surface which interpolates the given points with the given parameters
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) interpolatingSurface(const TColgp_Array2OfPnt& points, const Handle(TColStd_HArray1OfReal) parameters_u,
                                                                        const Handle(TColStd_HArray1OfReal) parameters_v, bool is_closed_u, bool is_closed_v);

    /**
     * @brief createGordonSurface:
     *          Returns a Gordon Surface of a given compatible network of B-splines
     *          All parameters must be in the right order and the B-spline network must be 'closed', i.e., B-splines mustn't stick out!
     * @param compatible_splines_u_vector:
     *          vector of B-splines in u-direction
     *          compatible means: intersection parameters with v-directional B-splines are equal
     *                            (if not: reparametrize -> change B-spline knots)
     *                            DON'T need to have common knot vector because skinning method is creating it when
     *                            needed (for surface_u)
     * @param compatible_splines_v_vector:
     *          vector of B-splines in v-direction, orthogonal to u-direction
     *          compatible means: intersection parameters with u-directional B-splines are equal
     *                            (if not: reparametrize -> change B-spline knots)
     *                            DON'T need to have common knot vector because skinning method is creating it when
     *                            needed (for surface_v)
     * @param intersection_params_spline_u:
     *          array of intersection parameters of the u-directional B-splines with all the v-directional B-splines
     *          These intersection parameters must be the same for all u-directional B-splines (because compatible).
     * @param intersection_params_spline_v:
     *          array of intersection parameters of the v-directional B-splines with all the u-directional B-splines
     *          These intersection parameters are the same for all v-directional B-splines (because compatible).
     * @return:
     *          the Gordon Surface as a B-spline surface
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) createGordonSurface(const std::vector<Handle(Geom_BSplineCurve) >& compatible_splines_u_vector,
                                                                       const std::vector<Handle(Geom_BSplineCurve) >& compatible_splines_v_vector,
                                                                       const Handle(TColStd_HArray1OfReal) intersection_params_spline_u,
                                                                       const Handle(TColStd_HArray1OfReal) intersection_params_spline_v);

    /**
     * @brief intersectionFinder:
     *          Returns all intersections of two B-splines
     * @param spline1:
     *          first B-spline
     * @param spline2:
     *          second B-spline
     * @return:
     *          intersections of spline1 with spline2 as a vector of (parameter of spline1, parameter of spline2)-pairs
     */
    TIGL_EXPORT static std::vector<std::pair<double, double> > intersectionFinder(const Handle(Geom_BSplineCurve) spline1, const Handle(Geom_BSplineCurve) spline2);

    /**
     * @brief sortBSplines:
     *          Returns a pair of parameters and its correlating vector of B-splines in such a sorted way so that the parameters are in an ascending order
     * @param parameters:
     *          array of parameters that correlate to the given vector of B-splines
     *          Each B-spline has its own parameter
     * @param splines_vector:
     *          vector of B-splines
     *          There must be as many B-splines as parameters.
     * @return:
     *          a pair of the sorted parameters and their sorted vector of B-spline curves
     */
    TIGL_EXPORT static std::pair<Handle(TColStd_HArray1OfReal), std::vector<Handle(Geom_BSplineCurve)> > sortBSplines(const Handle(TColStd_HArray1OfReal) parameters, const std::vector<Handle(Geom_BSplineCurve)>& splines_vector);

    /**
     * @brief scaleOfBSplines:
     *          Returns the approximate scale of the biggest given B-spline curve
     * @param splines_vector:
     *          vector of B-spline curves
     * @return:
     *          the scale
     */
    TIGL_EXPORT static double scaleOfBSplines(const std::vector<Handle(Geom_BSplineCurve)>& splines_vector);

    /**
     * @brief createGordonSurfaceGeneral:
     *          Returns a Gordon Surface of a given compatible network of B-splines
     * @param splines_u_vector:
     *          vector of B-splines parametrized in u-direction which don't need to be compatible yet
     *          ! Have to be in the right order (from small v to bigger v) !
     * @param splines_v_vector:
     *          vector of B-splines parametrized in v-direction which don't need to be compatible yet
     *          ! Have to be in the right order (from small u to bigger u) !
     * @return:
     *          the Gordon Surface as a B-spline surface
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) createGordonSurfaceGeneral(const std::vector<Handle(Geom_BSplineCurve) >& splines_u_vector,
                                                                              const std::vector<Handle(Geom_BSplineCurve) >& splines_v_vector);
};
} // namespace tigl

#endif // CTIGLBSPLINEALGORITHMS_H
