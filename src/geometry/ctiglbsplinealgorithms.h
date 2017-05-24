#ifndef CTIGLBSPLINEALGORITHMS_H
#define CTIGLBSPLINEALGORITHMS_H

#include "tigl_internal.h"

#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>

#include <utility>
#include <vector>

namespace tigl
{

class CTiglBSplineAlgorithms
{
public:
    TIGL_EXPORT CTiglBSplineAlgorithms();

    /**
     * @brief computeParamsBSplineCurve:
     *          Computes the parameters of a Geom_BSplineCurve at the given points
     * @param points:
     *          Given points where new parameters are computed at
     * @param alpha:
     *          parameters for the computation of the parameters; alpha=0.5 means, that this method uses the centripetal method
     * @param parameters
     *          reference of the TColStd_Array1OfReal parameters, that is created beforehand
     */
    TIGL_EXPORT static Handle(TColStd_HArray1OfReal) computeParamsBSplineCurve(const TColgp_Array1OfPnt& points, double alpha);

    /**
     * @brief computeParamsBSplineCurveCentripetal:
     *          Computes the parameters of a Geom_BSplineCurve at the given points by the centripetal method
     * @param points:
     *          Given points where new parameters are computed at
     * @param parameters:
     *          reference of the TColStd_Array1OfReal parameters, that is created beforehand
     */
    TIGL_EXPORT static Handle(TColStd_HArray1OfReal) computeParamsBSplineCurveCentripetal(const TColgp_Array1OfPnt& points);

    /**
     * @brief computeParamsBSplineSurf:
     *          Computes the parameters of a Geom_BSplineSurface at the given points
     * @param points:
     *          Given points where new parameters are computed at
     * @return  a std::pair of Handle(TColStd_HArray1OfReal) of the parameters in u- and in v-direction
     */
    TIGL_EXPORT static std::pair<Handle(TColStd_HArray1OfReal), Handle(TColStd_HArray1OfReal) >
    computeParamsBSplineSurf(const TColgp_Array2OfPnt& points, double alpha);

    /**
     * @brief computeParamsBSplineSurfCentripetal:
     *          Computes the parameters of a Geom_BSplineSurface at the given points by the centripetal method
     * @param points:
     *          Given points where new parameters are computed at
     * @return  a std::pair of Handle(TColStd_HArray1OfReal) of the parameters in u- and in v-direction
     */
    TIGL_EXPORT static std::pair<Handle(TColStd_HArray1OfReal), Handle(TColStd_HArray1OfReal) >
    computeParamsBSplineSurfCentripetal(const TColgp_Array2OfPnt& points);

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
    TIGL_EXPORT static Handle(Geom_BSplineSurface) skinnedBSplineSurfaceParams(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector, const Handle(TColStd_HArray1OfReal) v_params);

    /**
     * @brief skinnedBSplineSurfaceParams:
     *          Skinning of a vector of B-splines and creating a common knot vector for all the B-splines beforehand
     *          !!! Computes the parameters by the centripetal method which are used for the given B-splines for the skinned surface and then uses the method skinnedBSplineSurfaceParams !!!
     * @param splines_vector:
     *          given vector of B-splines (which are parametrized in u-direction)
     * @return
     *          the created skinned surface
     */
    TIGL_EXPORT static Handle(Geom_BSplineSurface) skinnedBSplineSurface(const std::vector<Handle(Geom_BSplineCurve) >& splines_vector);

    /**
     * @brief Reparametrizes a given B-spline by giving an array of its old parameters that should have the values of the given array of new parameters after this function call.
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
    TIGL_EXPORT static Handle(Geom_BSplineCurve) reparametrizeBSpline(const Handle(Geom_BSplineCurve) spline, const TColStd_Array1OfReal& old_parameters, const TColStd_Array1OfReal& new_parameters);
    TIGL_EXPORT static Handle(Geom_BSplineSurface) flipSurface(const Handle(Geom_BSplineSurface) surface);

private:
    // typedefs for createCommonKnotsVectorSurfaceOneDir
    typedef void (* knotInsertionCall)(const Handle(Geom_BSplineSurface) surface, double knot, int mult);
    typedef double (* getKnotCall)(const Handle(Geom_BSplineSurface) surface, int index);
    typedef int (* getMultCall)(const Handle(Geom_BSplineSurface) surface, int index);
    typedef int (* getIntCall)(const Handle(Geom_BSplineSurface) surface);
    /**
     * @brief createCommonKnotsVectorSurfaceOneDir:
     *          Creates a common knot vector in u- or v-direction of the given vector of B-spline surfaces
     *          The common knot vector contains all knots in u- or v-direction of all surfaces with the highest multiplicity of all surfaces.
     * @param old_surfaces_vector:
     *          the given vector of B-spline surfaces that could have a different knot vector in u- or v-direction
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
     *          the given vector of B-spline surfaces, now with a common knot vector in u- or in v-direction
     *          The B-spline surface geometry remains the same.
     */
    static std::vector<Handle(Geom_BSplineSurface) > createCommonKnotsVectorSurfaceOneDir(const std::vector<Handle(Geom_BSplineSurface) > old_surfaces_vector, knotInsertionCall insert_knot, getIntCall get_knot_number, getKnotCall get_knot, getMultCall get_mult, getIntCall get_degree);

};

} // namespace tigl

#endif // CTIGLBSPLINEALGORITHMS_H
