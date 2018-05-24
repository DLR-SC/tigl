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

#include "CTiglInterpolateCurveNetwork.h"

#include "CTiglError.h"
#include "CTiglBSplineAlgorithms.h"
#include "CTiglCurveNetworkSorter.h"
#include "tiglcommonfunctions.h"

#include <algorithm>

#include <math_Matrix.hxx>
#include <TColStd_HArray1OfReal.hxx>

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

CTiglInterpolateCurveNetwork::CTiglInterpolateCurveNetwork(const std::vector<Handle (Geom_BSplineCurve)> &profiles,
                                                           const std::vector<Handle (Geom_BSplineCurve)> &guides,
                                                           double spatialTol)
    : m_hasPerformed(false)
    , m_spatialTol(spatialTol)
{
    // check whether there are any u-directional and v-directional B-splines in the vectors
    if (profiles.size() < 2) {
        throw CTiglError("There must be at least two profiles for the curve network interpolation.", TIGL_MATH_ERROR);
    }

    if (guides.size()  < 2) {
        throw CTiglError("There must be at least two guides for the curve network interpolation.", TIGL_MATH_ERROR);
    }
    
    m_profiles.reserve(profiles.size());
    m_guides.reserve(guides.size());

    // Copy the curves
    for (CurveArray::const_iterator it = profiles.begin(); it != profiles.end(); ++it) {
        m_profiles.push_back(Handle_Geom_BSplineCurve::DownCast((*it)->Copy()));
    }
    for (CurveArray::const_iterator it = guides.begin(); it != guides.end(); ++it) {
        m_guides.push_back(Handle_Geom_BSplineCurve::DownCast((*it)->Copy()));
    }
}

void CTiglInterpolateCurveNetwork::CheckCurveNetworkCompatibility(const std::vector<Handle(Geom_BSplineCurve) >& profiles,
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

void CTiglInterpolateCurveNetwork::ComputeIntersections(math_Matrix& intersection_params_u,
                                                        math_Matrix& intersection_params_v) const
{
    const std::vector<Handle(Geom_BSplineCurve)>& profiles = m_profiles;
    const std::vector<Handle(Geom_BSplineCurve)>& guides = m_guides;
    
    for (int spline_u_idx = 0; spline_u_idx < static_cast<int>(profiles.size()); ++spline_u_idx) {
        for (int spline_v_idx = 0; spline_v_idx < static_cast<int>(guides.size()); ++spline_v_idx) {
            std::vector<std::pair<double, double> > currentIntersections = CTiglBSplineAlgorithms::intersections(profiles[static_cast<size_t>(spline_u_idx)],
                                                                                                                 guides[static_cast<size_t>(spline_v_idx)],
                                                                                                                 m_spatialTol);

            if (currentIntersections.size() < 1) {
                throw tigl::CTiglError("U-directional B-spline and v-directional B-spline don't intersect each other!");
            }

            else if (currentIntersections.size() == 1) {
                intersection_params_u(spline_u_idx, spline_v_idx) = currentIntersections[0].first;
                intersection_params_v(spline_u_idx, spline_v_idx) = currentIntersections[0].second;
            }
                // for closed curves
            else if (currentIntersections.size() == 2) {

                // only the u-directional B-spline curves are closed
                if (profiles[0]->IsClosed()) {

                    if (spline_v_idx == 0) {
                        intersection_params_u(spline_u_idx, spline_v_idx) = std::min(currentIntersections[0].first, currentIntersections[1].first);
                    }
                    else if (spline_v_idx == static_cast<int>(guides.size() - 1)) {
                        intersection_params_u(spline_u_idx, spline_v_idx) = std::max(currentIntersections[0].first, currentIntersections[1].first);
                    }

                    // intersection_params_vector[0].second == intersection_params_vector[1].second
                    intersection_params_v(spline_u_idx, spline_v_idx) = currentIntersections[0].second;
                }

                // only the v-directional B-spline curves are closed
                if (guides[0]->IsClosed()) {

                    if (spline_u_idx == 0) {
                        intersection_params_v(spline_u_idx, spline_v_idx) = std::min(currentIntersections[0].second, currentIntersections[1].second);
                    }
                    else if (spline_u_idx == static_cast<int>(profiles.size() - 1)) {
                        intersection_params_v(spline_u_idx, spline_v_idx) = std::max(currentIntersections[0].second, currentIntersections[1].second);
                    }
                    // intersection_params_vector[0].first == intersection_params_vector[1].first
                    intersection_params_u(spline_u_idx, spline_v_idx) = currentIntersections[0].first;
                }

//                // TODO: both u-directional splines and v-directional splines are closed
//               else if (intersection_params_vector.size() == 4) {

//                }
            }

            else if (currentIntersections.size() > 2) {
                throw tigl::CTiglError("U-directional B-spline and v-directional B-spline have more than two intersections with each other!");
            }
        }
    }
}

void CTiglInterpolateCurveNetwork::SortCurves(math_Matrix& intersection_params_u, math_Matrix& intersection_params_v)
{
    CTiglCurveNetworkSorter sorterObj(std::vector<Handle(Geom_Curve)>(m_profiles.begin(), m_profiles.end()),
                                      std::vector<Handle(Geom_Curve)>(m_guides.begin(), m_guides.end()),
                                      intersection_params_u,
                                      intersection_params_v);
    sorterObj.Perform();

    // get the sorted matrices and vectors
    intersection_params_u = sorterObj.ProfileIntersectionParms();
    intersection_params_v = sorterObj.GuideIntersectionParms();

    // copy sorted curves back into our curve arrays
    std::transform(sorterObj.Profiles().begin(), sorterObj.Profiles().end(), m_profiles.begin(), Handle(Geom_BSplineCurve)::DownCast);
    std::transform(sorterObj.Guides().begin(), sorterObj.Guides().end(), m_guides.begin(), Handle(Geom_BSplineCurve)::DownCast);
}

void CTiglInterpolateCurveNetwork::MakeCurvesCompatible()
{

    // reparametrize into [0,1]
    for (CurveArray::iterator it = m_profiles.begin(); it != m_profiles.end(); ++it) {
       CTiglBSplineAlgorithms::reparametrizeBSpline(*(*it), 0., 1., 1e-15);
    }

    for (CurveArray::iterator it = m_guides.begin(); it != m_guides.end(); ++it) {
        CTiglBSplineAlgorithms::reparametrizeBSpline(*(*it), 0., 1., 1e-15);
    }
    // now the parameter range of all  profiles and guides is [0, 1]

    int nGuides = static_cast<int>(m_guides.size());
    int nProfiles = static_cast<int>(m_profiles.size());
    // now find all intersections of all B-splines with each other
    math_Matrix intersection_params_u(0, nProfiles - 1,
                                      0, nGuides - 1);
    math_Matrix intersection_params_v(0, nProfiles - 1,
                                      0, nGuides - 1);

    ComputeIntersections(intersection_params_u, intersection_params_v);

    // sort intersection_params_u and intersection_params_v and u-directional and v-directional B-spline curves
    SortCurves(intersection_params_u, intersection_params_v);

    // eliminate small inaccuracies of the intersection parameters:
    EliminateInaccuraciesNetworkIntersections(m_profiles, m_guides, intersection_params_u, intersection_params_v);

    std::vector<double> newParametersProfiles;
    for (int spline_v_idx = 1; spline_v_idx <= nGuides; ++spline_v_idx) {
        double sum = 0;
        for (int spline_u_idx = 1; spline_u_idx <= nProfiles; ++spline_u_idx) {
            sum += intersection_params_u(spline_u_idx - 1, spline_v_idx - 1);
        }
        newParametersProfiles.push_back(sum / nProfiles);
    }

    std::vector<double> newParametersGuides;
    for (int spline_u_idx = 1; spline_u_idx <= nProfiles; ++spline_u_idx) {
        double sum = 0;
        for (int spline_v_idx = 1; spline_v_idx <= nGuides; ++spline_v_idx) {
            sum += intersection_params_v(spline_u_idx - 1, spline_v_idx - 1);
        }
        newParametersGuides.push_back(sum / nGuides);
    }

    if (newParametersProfiles.front() > 1e-5 || newParametersGuides.front() > 1e-5) {
        throw CTiglError("At least one B-splines has no intersection at the beginning.");
    }

    // Get maximum number of control points to figure out detail of spline
    size_t max_cp_u = 0, max_cp_v = 0;
    for(CurveArray::const_iterator it = m_profiles.begin(); it != m_profiles.end(); ++it) {
        max_cp_u = std::max(max_cp_u, static_cast<size_t>((*it)->NbPoles()));
    }
    for(CurveArray::const_iterator it = m_guides.begin(); it != m_guides.end(); ++it) {
        max_cp_v = std::max(max_cp_v, static_cast<size_t>((*it)->NbPoles()));
    }

    // we want to use at least 10 and max 80 control points to be able to reparametrize the geometry properly
    size_t mincp = 10;
    size_t maxcp = 80;

    // since we interpolate the intersections, we cannot use fewer control points than curves
    // We need to add two since we want c2 continuity, which adds two equations
    size_t min_u = std::max(m_guides.size() + 2, mincp);
    size_t min_v = std::max(m_profiles.size() + 2, mincp);

    size_t max_u = std::max(min_u, maxcp);
    size_t max_v = std::max(min_v, maxcp);

    max_cp_u = Clamp(max_cp_u + 10, min_u, max_u);
    max_cp_v = Clamp(max_cp_v + 10, min_v, max_v);

    // reparametrize u-directional B-splines
    for (int spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx) {

        std::vector<double> oldParametersProfile;
        for (int spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx) {
            oldParametersProfile.push_back(intersection_params_u(spline_u_idx, spline_v_idx));
        }

        // eliminate small inaccuracies at the first knot
        if (std::abs(oldParametersProfile.front()) < 1e-5) {
            oldParametersProfile.front() = 0;
        }

        if (std::abs(newParametersProfiles.front()) < 1e-5) {
            newParametersProfiles.front() = 0;
        }

        // eliminate small inaccuracies at the last knot
        if (std::abs(oldParametersProfile.back() - 1) < 1e-5) {
            oldParametersProfile.back() = 1;
        }

        if (std::abs(newParametersProfiles.back() - 1) < 1e-5) {
            newParametersProfiles.back() = 1;
        }

        Handle(Geom_BSplineCurve)& profile = m_profiles[static_cast<size_t>(spline_u_idx)];
        profile = CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(profile, oldParametersProfile, newParametersProfiles, max_cp_u);
    }

    // reparametrize v-directional B-splines
    for (int spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx) {

        std::vector<double> oldParameterGuide;
        for (int spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx) {
            oldParameterGuide.push_back(intersection_params_v(spline_u_idx, spline_v_idx));
        }

        // eliminate small inaccuracies at the first knot
        if (std::abs(oldParameterGuide.front()) < 1e-5) {
            oldParameterGuide.front() = 0;
        }

        if (std::abs(newParametersGuides.front()) < 1e-5) {
            newParametersGuides.front() = 0;
        }

        // eliminate small inaccuracies at the last knot
        if (std::abs(oldParameterGuide.back() - 1) < 1e-5) {
            oldParameterGuide.back() = 1;
        }

        if (std::abs(newParametersGuides.back() - 1) < 1e-5) {
            newParametersGuides.back() = 1;
        }

        Handle(Geom_BSplineCurve)& guide = m_guides[static_cast<size_t>(spline_v_idx)];
        guide = CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(guide, oldParameterGuide, newParametersGuides, max_cp_v);
    }


    m_intersectionParamsU = newParametersProfiles;
    m_intersectionParamsV = newParametersGuides;
}

void CTiglInterpolateCurveNetwork::CreateGordonSurface(const std::vector<Handle(Geom_BSplineCurve) >& profiles,
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
                                   intersection_params_spline_v);

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
    Handle(Geom_BSplineSurface) surfProfiles = CTiglBSplineAlgorithms::curvesToSurface(profiles, intersection_params_spline_v, makeVClosed);
    // therefore reparametrization before this method

    // Skinning in u-direction with v directional B-Splines
    Handle(Geom_BSplineSurface) surfGuides = CTiglBSplineAlgorithms::curvesToSurface(guides, intersection_params_spline_u, makeUClosed);

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

    assert(m_skinningSurfU->NbUPoles() == m_skinningSurfV->NbUPoles() && m_skinningSurfV->NbUPoles() == m_tensorProdSurf->NbUPoles());
    assert(m_skinningSurfU->NbVPoles() == m_skinningSurfV->NbVPoles() && m_skinningSurfV->NbVPoles() == m_tensorProdSurf->NbVPoles());

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

void CTiglInterpolateCurveNetwork::EliminateInaccuraciesNetworkIntersections(const std::vector<Handle(Geom_BSplineCurve)> & sortedProfiles,
                                                                             const std::vector<Handle(Geom_BSplineCurve)> & sortedGuides,
                                                                             math_Matrix & intersection_params_u,
                                                                             math_Matrix & intersection_params_v) const
{
    Standard_Integer nProfiles = static_cast<Standard_Integer>(sortedProfiles.size());
    Standard_Integer nGuides = static_cast<Standard_Integer>(sortedGuides.size());
    // eliminate small inaccuracies of the intersection parameters:

    // first intersection
    for (Standard_Integer spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx) {
        if (std::abs(intersection_params_u(spline_u_idx, 0) - sortedProfiles[0]->Knot(1)) < 0.001) {
            if (std::abs(sortedProfiles[0]->Knot(1)) < 1e-10) {
                intersection_params_u(spline_u_idx, 0) = 0;
            }
            else {
                intersection_params_u(spline_u_idx, 0) = sortedProfiles[0]->Knot(1);
            }
        }
    }

    for (Standard_Integer spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx) {
        if (std::abs(intersection_params_v(0, spline_v_idx) - sortedGuides[0]->Knot(1)) < 0.001) {
            if (std::abs(sortedGuides[0]->Knot(1)) < 1e-10) {
                intersection_params_v(0, spline_v_idx) = 0;
            }
            else {
                intersection_params_v(0, spline_v_idx) = sortedGuides[0]->Knot(1);
            }
        }
    }

    // last intersection
    for (Standard_Integer spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx) {
        if (std::abs(intersection_params_u(spline_u_idx, nGuides - 1) - sortedProfiles[0]->Knot(sortedProfiles[0]->NbKnots())) < 0.001) {
            intersection_params_u(spline_u_idx, nGuides - 1) = sortedProfiles[0]->Knot(sortedProfiles[0]->NbKnots());
        }
    }

    for (Standard_Integer spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx) {
        if (std::abs(intersection_params_v(nProfiles - 1, spline_v_idx) - sortedGuides[0]->Knot(sortedGuides[0]->NbKnots())) < 0.001) {
            intersection_params_v(nProfiles - 1, spline_v_idx) = sortedGuides[0]->Knot(sortedGuides[0]->NbKnots());
        }
    }
}


Handle(Geom_BSplineSurface) CTiglInterpolateCurveNetwork::Surface()
{
    Perform();

    return m_gordonSurf;
}

CTiglInterpolateCurveNetwork::operator Handle(Geom_BSplineSurface) ()
{
    return Surface();
}

Handle(Geom_BSplineSurface) CTiglInterpolateCurveNetwork::SurfaceProfiles()
{
    Perform();

    return m_skinningSurfProfiles;
}

Handle(Geom_BSplineSurface) CTiglInterpolateCurveNetwork::SurfaceGuides()
{
    Perform();

    return m_skinningSurfGuides;
}

Handle(Geom_BSplineSurface) CTiglInterpolateCurveNetwork::SurfaceIntersections()
{
    Perform();

    return m_tensorProdSurf;
}

std::vector<double> CTiglInterpolateCurveNetwork::ParametersProfiles()
{
    Perform();

    return m_intersectionParamsV;
}

std::vector<double> CTiglInterpolateCurveNetwork::ParametersGuides()
{
    Perform();

    return m_intersectionParamsU;
}

void CTiglInterpolateCurveNetwork::Perform()
{
    if (m_hasPerformed) {
        return;
    }

    // Gordon surfaces are only defined on a compatible curve network
    // We first have to reparametrize the network
    MakeCurvesCompatible();

    CreateGordonSurface(m_profiles, m_guides, m_intersectionParamsU, m_intersectionParamsV);

    m_hasPerformed = true;
}

Handle(Geom_BSplineSurface) curveNetworkToSurface(const std::vector<Handle (Geom_BSplineCurve)> &profiles, const std::vector<Handle (Geom_BSplineCurve)> &guides, double tol)
{
    return CTiglInterpolateCurveNetwork(profiles, guides, tol).Surface();
}

} // namespace tigl
