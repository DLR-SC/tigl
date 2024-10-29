/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
*
* Created: 2020-09-10 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLCONCATSURFACES_H
#define CTIGLCONCATSURFACES_H

#include "tigl_internal.h"
#include <vector>
#include <Geom_BSplineSurface.hxx>
#include <boost/optional.hpp>

namespace tigl
{

enum class ConcatDir
{
    u,
    v
};

class CTiglConcatSurfaces
{
public:
    /**
     * @brief Concatenes multiple surfaces along the given directions
     *
     * Note: The surfaces must follow in the correct order in the specified direction
     *
     * @param surfaces      The surfaces to concatenate
     * @param surfaceParams A vector of parameters that define the parameters at the final surface, at which
     *                      the input surfaces are stitched.
     * @param dir           Direction to concatenate
     */
    TIGL_EXPORT CTiglConcatSurfaces(const std::vector<Handle(Geom_BSplineSurface)>& surfaces,
                                    const std::vector<double>& surfaceParams, ConcatDir dir);

    /**
     * @brief This allows the algorithm to make the knot sequence of each input
     * surface uniform.
     *
     * This might be useful, if the input surfaces have very different knot distributions
     * leaving a resulting in a bad balanced knot vector of the final surfaces.
     * This approximation comes however with an error.
     *
     * @param nUSeg Number of knot segments in u dir to approximate
     * @param nVSeg Number of knot segments in v dir to approximate
     */
    TIGL_EXPORT void SetMakeKnotsUniformEnabled(unsigned int nUSeg, unsigned int nVSeg);

    /**
     + @brief Returns the final surface
     */
    TIGL_EXPORT Handle(Geom_BSplineSurface) Surface() const;

private:
    std::vector<Handle(Geom_BSplineSurface)> m_surfaces;
    std::vector<double> m_params;
    ConcatDir m_dir;

    struct SegmentsSize {
        unsigned int nu, nv;
    };
    boost::optional<SegmentsSize> m_approxSegments;
};

} // namespace tigl

#endif // CTIGLCONCATSURFACES_H
