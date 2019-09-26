/* 
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2019-02-04 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLPROJECTPOINTONCURVEATANGLE_H
#define CTIGLPROJECTPOINTONCURVEATANGLE_H

#include "tigl_internal.h"
#include <Geom_Curve.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

#include <vector>

namespace tigl
{

class ValueWithDerivative
{
public:
    TIGL_EXPORT ValueWithDerivative()
        : m_value(0), m_derivative(0)
    {}

    TIGL_EXPORT ValueWithDerivative(double v, double d)
        : m_value(v), m_derivative(d)
    {}
    
    double& value()
    {
        return m_value;
    }
    const double value() const
    {
        return m_value;
    }
    
    double& derivative()
    {
        return m_derivative;
    }
    const double derivative() const
    {
        return m_derivative;
    }

private:
    double m_value;
    double m_derivative;
};

class CTiglProjectPointOnCurveAtAngle
{
public:
    /**
     * Projects a point a curve such that the vector from the point p to the point on the
     * curve has a certain angle to the curve's direction.
     * 
     * @param P Point to project on the curve
     * @param curve Curve on which p is projected on
     * @param angle Angle, at which the curve is projected on. Angle in range [0,pi].
     *        0 means in curve direction, pi against curve direction. Pi/2 perpendicular to the curve
     *        corresponds to standard projection.
     * @param planeRefNormal Normal of the reference plane, in which the angle is measured.
     */
    TIGL_EXPORT CTiglProjectPointOnCurveAtAngle(const gp_Pnt& p, const Handle(Geom_Curve)& curve, double angle, const gp_Dir& planeRefNormal);
    
    /// Returns true, if the computation is successful. Note: It might be, that
    /// there is no solution possible. In this case, IsDone() returns false.
    TIGL_EXPORT bool IsDone() const;
    
    /// Returns the point on the curve. Throws, if no solution exists
    TIGL_EXPORT gp_Pnt Point(int i) const;
    
    /// Returns the parameter on the curve
    TIGL_EXPORT double Parameter(int i) const;

    TIGL_EXPORT int NbPoints() const;
    
private:
    void Compute() const;
    void FindPoint(double ustart) const;
    void AddSolution(double value, double tol) const;

    const Handle(Geom_Curve) m_curve;
    const double m_angle;
    const gp_Dir m_refNormal;
    const gp_Pnt m_pointToProject;

    mutable std::vector<double> resultParameter;
    mutable bool m_hasComputed = true;
};


/**
 * @brief Computes the distance of a point to a plane
 * All parameters _deriv are forward differences to compute gradients
 * @param planeNormal        Normal of the plane
 * @param planeNormal_deriv
 * @param pointOnPlane       Point on the plane
 * @param pointOnPlane_deriv
 * @param p                  A point, which distance to the plane has to be computed
 * @return 
 */
TIGL_EXPORT ValueWithDerivative distancePlanePoint_deriv(gp_Dir planeNormal, gp_Vec planeNormal_deriv,
                                             gp_Pnt pointOnPlane, gp_Vec pointOnPlane_deriv,
                                             gp_Pnt p);

TIGL_EXPORT void normalize(gp_Vec& v, gp_Vec& v_deriv);

} // namespace tigl

#endif // CTIGLPROJECTPOINTONCURVEATANGLE_H
