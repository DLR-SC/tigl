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

#ifndef CTIGLGORDONSURFACEBUILDER_H
#define CTIGLGORDONSURFACEBUILDER_H

#include "tigl_internal.h"

#include <vector>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BSplineCurve.hxx>

namespace tigl
{

/**
 * @brief This class is basically a helper class for the tigl::CTiglInterpolateCurveNetwork algorithm.
 * 
 * It implements the basics gordon surface algorithm.
 * 
 * Since it requires a compatible curve network, it is not very useful on its own.
 * For practical reasons, use tigl::CTiglInterpolateCurveNetwork.
 */
class CTiglGordonSurfaceBuilder
{
public:
    /**
     * @brief   Builds a Gordon Surface of a given compatible network of B-splines
     *          All parameters must be in the right order and the B-spline network must be 'closed',
     *          i.e., B-splines mustn't stick out!
     * @param profiles:
     *          vector of B-splines in u-direction
     *          compatible means: intersection parameters with v-directional B-splines are equal
     * @param guides:
     *          vector of B-splines in v-direction, orthogonal to u-direction
     *          compatible means: intersection parameters with u-directional B-splines are equal
     *                            (if not: reparametrize -> change B-spline knots)
     *                            DON'T need to have common knot vector because skinning method is creating it when
     *                            needed (for surface_v)
     * @param intersectParamsOnProfiles:
     *          Parameters on the profiles of the intersection with the guides (size = nGuides)
     * @param intersectParamsOnGuides:
     *          Parameters on the guides of the intersection with the profiles (size = nProfiles)
     * @param spatialTolerance:
     *          Maximum allowed distance between each guide and profile (in theory they must intersect)
     */
    CTiglGordonSurfaceBuilder(const std::vector<Handle(Geom_BSplineCurve)>& profiles,
                              const std::vector<Handle(Geom_BSplineCurve)>& guides,
                              const std::vector<double>& intersectParamsOnProfiles,
                              const std::vector<double>& intersectParamsOnGuides,
                              double spatialTolerance);
    
    /// Returns the interpolation surface
    TIGL_EXPORT Handle(Geom_BSplineSurface) SurfaceGordon();

    /// Returns the surface that interpolates the profiles
    TIGL_EXPORT Handle(Geom_BSplineSurface) SurfaceProfiles();
    
    /// Returns the surface that interpolates the guides
    TIGL_EXPORT Handle(Geom_BSplineSurface) SurfaceGuides();
    
    /// Returns the Surface that interpolations the intersection point of both surfaces
    TIGL_EXPORT Handle(Geom_BSplineSurface) SurfaceIntersections();
    
private:
    void Perform();

    void CheckCurveNetworkCompatibility(const std::vector<Handle(Geom_BSplineCurve) >& profiles,
                                        const std::vector<Handle(Geom_BSplineCurve) >& guides,
                                        const std::vector<double>& intersection_params_spline_u,
                                        const std::vector<double>& intersection_params_spline_v,
                                        double tol);

    void CreateGordonSurface(const std::vector<Handle(Geom_BSplineCurve)>& profiles,
                             const std::vector<Handle(Geom_BSplineCurve)>& guides,
                             const std::vector<double>& intersection_params_spline_u,
                             const std::vector<double>& intersection_params_spline_v);

    typedef std::vector<Handle(Geom_BSplineCurve)> CurveArray;
    CurveArray m_profiles;
    CurveArray m_guides;
    const std::vector<double>& m_intersection_params_spline_u, m_intersection_params_spline_v;
    Handle(Geom_BSplineSurface) m_skinningSurfProfiles, m_skinningSurfGuides, m_tensorProdSurf, m_gordonSurf;
    bool m_hasPerformed;
    double m_tol;
};

} // namespace tigl

#endif // CTIGLGORDONSURFACEBUILDER_H
