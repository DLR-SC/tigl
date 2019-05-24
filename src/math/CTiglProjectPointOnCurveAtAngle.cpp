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

#include "CTiglProjectPointOnCurveAtAngle.h"

#include "tigl_internal.h"

#include "CTiglError.h"

#include <math_MultipleVarFunctionWithGradient.hxx>
#include <math_BFGS.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax1.hxx>

#include <algorithm>


namespace 
{
    gp_Vec rotateAx(const gp_Vec& v, const gp_Dir& refNormal, double angle)
    {
        gp_Trsf transform;
        transform.SetRotation(gp_Ax1(gp_Pnt(0,0,0), refNormal), angle);
        
        gp_XYZ result(v.XYZ());
        transform.Transforms(result);
        return result;
    }
    
    tigl::ValueWithDerivative objFun(const gp_Pnt& m_pointToProject,
                                     double u, Handle(Geom_Curve) m_curve,
                                     const gp_Dir& m_refNormal, double m_angle)
    {
        // Compute point, tangent and second derivative
        gp_Pnt c; gp_Vec c1; gp_Vec c2;
        m_curve->D2(u, c, c1, c2);
        
        // rotate tangent around normal axis
        gp_Vec planeNormal = rotateAx(c1, m_refNormal, m_angle - M_PI/2.);
        gp_Vec planeNormal_deriv = rotateAx(c2, m_refNormal, m_angle - M_PI/2.);

        tigl::normalize(planeNormal, planeNormal_deriv);

        tigl::ValueWithDerivative distance = tigl::distancePlanePoint_deriv(planeNormal, planeNormal_deriv,
                                                                            c, c1,
                                                                            m_pointToProject);

        // compute square distance to improve convergence
        tigl::ValueWithDerivative result;
        result.value() = distance.value()*distance.value();
        result.derivative() = 2. * distance.value() * distance.derivative();
        return result;
    }

    class ProjectObjective : public math_MultipleVarFunctionWithGradient
    {
    public:
        ProjectObjective(const gp_Pnt& p,
                         const Handle(Geom_Curve)& curve,
                         double angle,
                         gp_Dir planeRefNormal)
            : m_curve(curve)
            , m_angle(angle)
            , m_refNormal(planeRefNormal)
            , m_pointToProject(p)
        {}
        
        virtual Standard_Integer NbVariables()  const OVERRIDE
        {
            return 1;
        }

        Standard_Boolean Value (const math_Vector& X, Standard_Real& F) OVERRIDE
        {
            math_Vector G(1, 1);
            return Values(X, F, G);
        }

        Standard_Boolean Gradient (const math_Vector& X, math_Vector& G) OVERRIDE
        {
            Standard_Real F = 0.;
            return Values(X, F, G);
        }

        virtual  Standard_Boolean Values (const math_Vector& X, Standard_Real& F, math_Vector& G)
        {
            double u = X.Value(1);

            tigl::ValueWithDerivative result = objFun(m_pointToProject, u, m_curve, m_refNormal, m_angle);

            // compute square distance to improve convergence
            F = result.value();
            G(1) = result.derivative();

            return true;
        }

    private:
        const Handle(Geom_Curve) m_curve;
        const double m_angle;
        const gp_Dir m_refNormal;
        const gp_Pnt m_pointToProject;
    };
}

namespace tigl
{

ValueWithDerivative distancePlanePoint_deriv(gp_Dir planeNormal, gp_Vec planeNormal_deriv,
                                             gp_Pnt pointOnPlane, gp_Vec pointOnPlane_deriv,
                                             gp_Pnt p)
{
    double distance = (pointOnPlane.XYZ() - p.XYZ()).Dot(planeNormal.XYZ());
    double distance_deriv = (pointOnPlane.XYZ() - p.XYZ()).Dot(planeNormal_deriv.XYZ()) 
                          + pointOnPlane_deriv.XYZ().Dot(planeNormal.XYZ());

    return ValueWithDerivative(distance, distance_deriv);
}

void normalize(gp_Vec& v, gp_Vec& v_deriv)
{
    gp_Vec vCopy(v);
    gp_Vec vCopy_deriv(v_deriv);
    
    double length = vCopy.Magnitude();
    double length_deriv = vCopy_deriv.XYZ().Dot(vCopy.XYZ()) / length;

    v = vCopy.Normalized();
    v_deriv = vCopy_deriv.XYZ() / length - vCopy.XYZ() / (length * length) * length_deriv;
}


CTiglProjectPointOnCurveAtAngle::CTiglProjectPointOnCurveAtAngle(const gp_Pnt& p,
                                                                 const Handle(Geom_Curve)& curve,
                                                                 double angle,
                                                                 const gp_Dir& planeRefNormal)
    : m_curve(curve)
    , m_angle(angle)
    , m_refNormal(planeRefNormal)
    , m_pointToProject(p)
    , m_hasComputed(false)
{
}

bool tigl::CTiglProjectPointOnCurveAtAngle::IsDone() const
{
    Compute();

    return resultParameter.size() > 0;
}

gp_Pnt tigl::CTiglProjectPointOnCurveAtAngle::Point(int i) const
{
    if (!IsDone()) {
        throw CTiglError("Error computing CTiglProjectPointOnCurveAtAngle::Point");
    }
    
    return m_curve->Value(resultParameter.at(i-1));
}

double tigl::CTiglProjectPointOnCurveAtAngle::Parameter(int i) const
{
    if (!IsDone()) {
        throw CTiglError("Error computing CTiglProjectPointOnCurveAtAngle::Point");
    }
    
    return resultParameter.at(i-1);
}

int tigl::CTiglProjectPointOnCurveAtAngle::NbPoints() const
{
    Compute();

    return static_cast<int>(resultParameter.size());
}

void CTiglProjectPointOnCurveAtAngle::CTiglProjectPointOnCurveAtAngle::Compute() const
{
    if (m_hasComputed) {
        return;
    }

    double umin = m_curve->FirstParameter();
    double umax = m_curve->LastParameter();
    
    int nSamples = 10;
    
    for (double u = umin; u <= umax; u += (umax-umin)/ (double)(nSamples - 1)) {
        FindPoint(u);
    }

    // sort accordining to distance of point
    std::sort(resultParameter.begin(), resultParameter.end(), [this](const double& parm1, const double& parm2) {
        // p1 > p2 ?
        gp_Pnt p1 = m_curve->Value(parm1);
        gp_Pnt p2 = m_curve->Value(parm2);
        
        return m_pointToProject.Distance(p1) > m_pointToProject.Distance(p2);
    });

    m_hasComputed = true;
}

void tigl::CTiglProjectPointOnCurveAtAngle::FindPoint(double ustart) const
{
    ProjectObjective objFun(m_pointToProject, m_curve, m_angle, m_refNormal);

    math_Vector xstart(1,1);
    xstart(1) =  ustart;

    // TODO: how to use bounded optimization?
    math_BFGS optimizer(objFun, xstart);
    
    if (!optimizer.IsDone()) {
        return;
    }

    AddSolution(optimizer.Location().Value(1), 1e-4 * (m_curve->LastParameter() - m_curve->FirstParameter()));
}

void tigl::CTiglProjectPointOnCurveAtAngle::AddSolution(double value, double tol) const
{
    // check if really a solution
    // get distance of plane to point
    tigl::ValueWithDerivative result = objFun(m_pointToProject, value, m_curve, m_refNormal, m_angle);
    if (result.value() > 1e-4 || value < m_curve->FirstParameter() || value > m_curve->LastParameter()) {
        return;
    }

    auto parm_it = std::find_if(resultParameter.begin(), resultParameter.end(), [&](double v) {
        return fabs(v-value) < tol;
    });
    
    if (parm_it == resultParameter.end()) {
        resultParameter.push_back(value);
    }
}



} // namespace tigl
