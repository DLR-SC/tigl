/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-05-23 Martin Siggel <Martin.Siggel@dlr.de>
*                with Merlin Pelz   <Merlin.Pelz@dlr.de>
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

#include "CTiglGordonSurfaceBuilder.h"

#include "CTiglError.h"
#include "CTiglBSplineAlgorithms.h"
#include "CTiglCurvesToSurface.h"
#include "tiglcommonfunctions.h"
#include <TColgp_Array2OfPnt.hxx>

#include <algorithm>

namespace
{
    void assertRange(const Handle(Geom_Curve)& curve, double umin, double umax, double tol=1e-7)
    {
        if (std::abs(curve->FirstParameter() - umin) > tol || std::abs(curve->LastParameter() - umax) > tol) {
            throw tigl::CTiglError("Curve not in range [" + tigl::std_to_string(umin) + ", " + tigl::std_to_string(umax) + "].");
        }
    }
}

namespace tigl
{

CTiglGordonSurfaceBuilder::CTiglGordonSurfaceBuilder(const CurveArray &profiles, const CurveArray &guides,
                                                     const std::vector<double> &intersection_params_spline_u,
                                                     const std::vector<double> &intersection_params_spline_v,
                                                     double tol)
    : m_profiles(profiles)
    , m_guides(guides)
    , m_intersection_params_spline_u(intersection_params_spline_u)
    , m_intersection_params_spline_v(intersection_params_spline_v)
    , m_hasPerformed(false)
    , m_tol(tol)
{
}

Handle(Geom_BSplineSurface) CTiglGordonSurfaceBuilder::SurfaceGordon()
{
    Perform();

    return m_gordonSurf;
}


Handle(Geom_BSplineSurface) CTiglGordonSurfaceBuilder::SurfaceProfiles()
{
    Perform();
    
    return m_skinningSurfProfiles;
}

Handle(Geom_BSplineSurface) CTiglGordonSurfaceBuilder::SurfaceGuides()
{
    Perform();
    
    return m_skinningSurfGuides;
}

Handle(Geom_BSplineSurface) CTiglGordonSurfaceBuilder::SurfaceIntersections()
{
    Perform();
    
    return m_tensorProdSurf;
}

void CTiglGordonSurfaceBuilder::Perform()
{
    if (m_hasPerformed) {
        return;
    }

    CreateGordonSurface(m_profiles, m_guides, m_intersection_params_spline_u, m_intersection_params_spline_v);

    m_hasPerformed = true;
}

void CTiglGordonSurfaceBuilder::CreateGordonSurface(const std::vector<Handle(Geom_BSplineCurve) >& profiles,
                                                    const std::vector<Handle(Geom_BSplineCurve) >& guides,
                                                    const std::vector<double>& intersection_params_spline_u,
                                                    const std::vector<double>& intersection_params_spline_v)
{
    // check whether there are any u-directional and v-directional B-splines in the vectors
    if (profiles.size() < 2) {
        throw CTiglError("There must be at least two profiles for the gordon surface.", TIGL_MATH_ERROR);
    }

    if (guides.size()  < 2) {
        throw CTiglError("There must be at least two guides for the gordon surface.", TIGL_MATH_ERROR);
    }

    // check B-spline parametrization is equal among all curves
    double umin = profiles.front()->FirstParameter();
    double umax = profiles.front()->LastParameter();
    for (CurveArray::const_iterator it = m_profiles.begin(); it != m_profiles.end(); ++it) {
        assertRange(*it, umin, umax, 1e-5);
    }

    double vmin = guides.front()->FirstParameter();
    double vmax = guides.front()->LastParameter();
    for (CurveArray::const_iterator it = m_guides.begin(); it != m_guides.end(); ++it) {
        assertRange(*it, vmin, vmax, 1e-5);
    }

    // TODO: Do we really need to check compatibility?
    // We don't need to do this, if the curves were reparametrized before
    // In this case, they might be even incompatible, as the curves have been approximated
    CheckCurveNetworkCompatibility(profiles, guides,
                                   intersection_params_spline_u,
                                   intersection_params_spline_v,
                                   m_tol);

    // setting everything up for creating Tensor Product Surface by interpolating intersection points of profiles and guides with B-Spline surface
    // find the intersection points:
    TColgp_Array2OfPnt intersection_pnts(1, static_cast<Standard_Integer>(intersection_params_spline_u.size()),
                                         1, static_cast<Standard_Integer>(intersection_params_spline_v.size()));

    // use splines in u-direction to get intersection points
    for (size_t spline_idx = 0; spline_idx < profiles.size(); ++spline_idx) {
        for (size_t intersection_idx = 0; intersection_idx < intersection_params_spline_u.size(); ++intersection_idx) {
            Handle(Geom_BSplineCurve) spline_u = profiles[spline_idx];
            double parameter = intersection_params_spline_u[intersection_idx];
            intersection_pnts(static_cast<Standard_Integer>(intersection_idx + 1),
                              static_cast<Standard_Integer>(spline_idx + 1)) = spline_u->Value(parameter);
        }
    }

    // check, whether to build a closed continous surface
    double curve_u_tolerance = CTiglBSplineAlgorithms::REL_TOL_CLOSED * CTiglBSplineAlgorithms::scale(guides);
    double curve_v_tolerance = CTiglBSplineAlgorithms::REL_TOL_CLOSED * CTiglBSplineAlgorithms::scale(profiles);
    double tp_tolerance      = CTiglBSplineAlgorithms::REL_TOL_CLOSED * CTiglBSplineAlgorithms::scale(intersection_pnts);

    bool makeUClosed = CTiglBSplineAlgorithms::isUDirClosed(intersection_pnts, tp_tolerance) && guides.front()->IsEqual(guides.back(), curve_u_tolerance);
    bool makeVClosed = CTiglBSplineAlgorithms::isVDirClosed(intersection_pnts, tp_tolerance) && profiles.front()->IsEqual(profiles.back(), curve_v_tolerance);

    // Skinning in v-direction with u directional B-Splines
    CTiglCurvesToSurface surfProfilesSkinner(std::vector<Handle(Geom_Curve)>(profiles.begin(), profiles.end()), intersection_params_spline_v, makeVClosed);
    Handle(Geom_BSplineSurface) surfProfiles = surfProfilesSkinner.Surface();
    // therefore reparametrization before this method

    // Skinning in u-direction with v directional B-Splines
    CTiglCurvesToSurface surfGuidesSkinner(std::vector<Handle(Geom_Curve)>(guides.begin(), guides.end()), intersection_params_spline_u, makeUClosed);
    Handle(Geom_BSplineSurface) surfGuides = surfGuidesSkinner.Surface();

    // flipping of the surface in v-direction; flipping is redundant here, therefore the next line is a comment!
    surfGuides = CTiglBSplineAlgorithms::flipSurface(surfGuides);

    // if there are too little points for degree in u-direction = 3 and degree in v-direction=3 creating an interpolation B-spline surface isn't possible in Open CASCADE

    // Open CASCADE doesn't have a B-spline surface interpolation method where one can give the u- and v-directional parameters as arguments
    Handle(Geom_BSplineSurface) tensorProdSurf = CTiglBSplineAlgorithms::pointsToSurface(intersection_pnts,
                                                                                         intersection_params_spline_u, intersection_params_spline_v,
                                                                                         makeUClosed, makeVClosed);
    // match degree of all three surfaces
    Standard_Integer degreeU = std::max(std::max(surfGuides->UDegree(),
                                                 surfProfiles->UDegree()),
                                                 tensorProdSurf->UDegree());

    Standard_Integer degreeV = std::max(std::max(surfGuides->VDegree(),
                                                 surfProfiles->VDegree()),
                                                 tensorProdSurf->VDegree());

    // check whether degree elevation is necessary (does method elevate_degree_u()) and if yes, elevate degree
    surfGuides->IncreaseDegree(degreeU, degreeV);
    surfProfiles->IncreaseDegree(degreeU, degreeV);
    tensorProdSurf->IncreaseDegree(degreeU, degreeV);

    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector_unmod;
    surfaces_vector_unmod.push_back(surfGuides);
    surfaces_vector_unmod.push_back(surfProfiles);
    surfaces_vector_unmod.push_back(tensorProdSurf);

    // create common knot vector for all three surfaces
    std::vector<Handle(Geom_BSplineSurface)> surfaces_vector = CTiglBSplineAlgorithms::createCommonKnotsVectorSurface(surfaces_vector_unmod);

    assert(surfaces_vector.size() == 3);

    m_skinningSurfGuides = surfaces_vector[0];
    m_skinningSurfProfiles = surfaces_vector[1];
    m_tensorProdSurf = surfaces_vector[2];

    assert(m_skinningSurfGuides->NbUPoles() == m_skinningSurfProfiles->NbUPoles() && m_skinningSurfProfiles->NbUPoles() == m_tensorProdSurf->NbUPoles());
    assert(m_skinningSurfGuides->NbVPoles() == m_skinningSurfProfiles->NbVPoles() && m_skinningSurfProfiles->NbVPoles() == m_tensorProdSurf->NbVPoles());

    m_gordonSurf = Handle(Geom_BSplineSurface)::DownCast(m_skinningSurfProfiles->Copy());

    // creating the Gordon Surface = s_u + s_v - tps by adding the control points
    for (int cp_u_idx = 1; cp_u_idx <= m_gordonSurf->NbUPoles(); ++cp_u_idx) {
        for (int cp_v_idx = 1; cp_v_idx <= m_gordonSurf->NbVPoles(); ++cp_v_idx) {
            gp_Pnt cp_surf_u = m_skinningSurfProfiles->Pole(cp_u_idx, cp_v_idx);
            gp_Pnt cp_surf_v = m_skinningSurfGuides->Pole(cp_u_idx, cp_v_idx);
            gp_Pnt cp_tensor = m_tensorProdSurf->Pole(cp_u_idx, cp_v_idx);

            m_gordonSurf->SetPole(cp_u_idx, cp_v_idx, cp_surf_u.XYZ() + cp_surf_v.XYZ() - cp_tensor.XYZ());
        }
    }
}

void CTiglGordonSurfaceBuilder::CheckCurveNetworkCompatibility(const std::vector<Handle(Geom_BSplineCurve) >& profiles,
                                                               const std::vector<Handle(Geom_BSplineCurve) >& guides,
                                                               const std::vector<double>& intersection_params_spline_u,
                                                               const std::vector<double>& intersection_params_spline_v,
                                                               double tol)
{
    // find out the 'average' scale of the B-splines in order to being able to handle a more approximate dataset and find its intersections
    double splines_scale = 0.5 * (CTiglBSplineAlgorithms::scale(profiles)+ CTiglBSplineAlgorithms::scale(guides));

    if (std::abs(intersection_params_spline_u.front()) > splines_scale * tol || std::abs(intersection_params_spline_u.back() - 1.) > splines_scale * tol) {
        throw tigl::CTiglError("WARNING: B-splines in u-direction mustn't stick out, spline network must be 'closed'!");
    }

    if (std::abs(intersection_params_spline_v.front()) > splines_scale * tol || std::abs(intersection_params_spline_v.back() - 1.) > splines_scale * tol) {
        throw tigl::CTiglError("WARNING: B-splines in v-direction mustn't stick out, spline network must be 'closed'!");
    }

    // check compatibilty of network
    for (size_t u_param_idx = 0; u_param_idx < intersection_params_spline_u.size(); ++u_param_idx) {
        double spline_u_param = intersection_params_spline_u[u_param_idx];
        const Handle(Geom_BSplineCurve)& spline_v = guides[u_param_idx];
        for (size_t v_param_idx = 0; v_param_idx < intersection_params_spline_v.size(); ++v_param_idx) {
            const Handle(Geom_BSplineCurve)& spline_u = profiles[v_param_idx];
            double spline_v_param = intersection_params_spline_v[v_param_idx];

            gp_Pnt p_prof = spline_u->Value(spline_u_param);
            gp_Pnt p_guid = spline_v->Value(spline_v_param);
            double distance = p_prof.Distance(p_guid);

            if (distance > splines_scale * tol) {
                throw tigl::CTiglError("B-spline network is incompatible (e.g. wrong parametrization) or intersection parameters are in a wrong order!");
            }
        }
    }
}

} // namespace tigl
