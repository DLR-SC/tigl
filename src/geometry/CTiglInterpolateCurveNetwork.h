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

#ifndef CTIGLINTERPOLATECURVENETWORK_H
#define CTIGLINTERPOLATECURVENETWORK_H

#include "tigl_internal.h"

#include <vector>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>

class math_Matrix;

namespace tigl
{

/**
 * @brief Curve network interpolation with gordon surfaces
 * 
 * The algorithm uses the gordon method, to create the interpolation surface.
 * To do this, it does the following:
 *  - Compute intersection points between profiles and guides
 *  - Sort the profiles and guides
 *  - Reparametrize profiles and curves to make the network compatible (in most cases necessary)
 *  - Compute the gordon surface
 */
class CTiglInterpolateCurveNetwork
{
public:
    /**
     * @brief CTiglInterpolateCurveNetwork interpolated a curve network of guide curves and profiles curves
     * @param profiles The profiles to be interpolated
     * @param guides   The guides curves to be interpolated
     * @param spatialTolerance Maximum allowed distance between each guide and profile (in theory they must intersect)
     */
    TIGL_EXPORT CTiglInterpolateCurveNetwork(const std::vector<Handle(Geom_Curve)>& profiles,
                                             const std::vector<Handle(Geom_Curve)>& guides,
                                             double spatialTolerance);

    TIGL_EXPORT operator Handle(Geom_BSplineSurface) ();
    
    /// Returns the interpolation surface
    TIGL_EXPORT Handle(Geom_BSplineSurface) Surface();

    /// Returns the surface that interpolates the profiles
    TIGL_EXPORT Handle(Geom_BSplineSurface) SurfaceProfiles();
    
    /// Returns the surface that interpolates the guides
    TIGL_EXPORT Handle(Geom_BSplineSurface) SurfaceGuides();
    
    /// Returns the Surface that interpolations the intersection point of both surfaces
    TIGL_EXPORT Handle(Geom_BSplineSurface) SurfaceIntersections();

    /// Returns the v parameters of the final surface, that correspond to the profile curve locations
    TIGL_EXPORT std::vector<double> ParametersProfiles();

    /// Returns the u parameters of the final surface, that correspond to the guide curve locations
    TIGL_EXPORT std::vector<double> ParametersGuides();

    /// Returns the reparametrized profile curves
    TIGL_EXPORT std::vector<Handle(Geom_BSplineCurve)> const& ReparametrizedProfiles() const;

    /// Returns the reparametrized guide curves
    TIGL_EXPORT std::vector<Handle(Geom_BSplineCurve)> const& ReparametrizedGuides() const;

private:
    void Perform();


    void ComputeIntersections(math_Matrix& intersection_params_u,
                              math_Matrix& intersection_params_v) const;

    // Sorts the profiles and guides
    void SortCurves(math_Matrix& intersection_params_u, math_Matrix& intersection_params_v);

    void MakeCurvesCompatible();

    void EliminateInaccuraciesNetworkIntersections(const std::vector<Handle(Geom_BSplineCurve)> & sorted_splines_u,
                                                   const std::vector<Handle(Geom_BSplineCurve)> & sorted_splines_v,
                                                   math_Matrix & intersection_params_u,
                                                   math_Matrix & intersection_params_v) const;

    bool m_hasPerformed;
    double m_spatialTol;
    
    typedef std::vector<Handle(Geom_BSplineCurve)> CurveArray;
    CurveArray m_profiles;
    CurveArray m_guides;
    std::vector<double> m_intersectionParamsU, m_intersectionParamsV;
    Handle(Geom_BSplineSurface) m_skinningSurfProfiles, m_skinningSurfGuides, m_tensorProdSurf, m_gordonSurf;
};

/// Convenience function calling CTiglInterpolateCurveNetwork
TIGL_EXPORT Handle(Geom_BSplineSurface) curveNetworkToSurface(const std::vector<Handle(Geom_Curve)>& profiles,
                                                              const std::vector<Handle(Geom_Curve)>& guides,
                                                              double spatialTol = 3e-4);

} // namespace tigl

#endif // CTIGLINTERPOLATECURVENETWORK_H
