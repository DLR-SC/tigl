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

#include "CTiglConcatSurfaces.h"

#include <CTiglBSplineAlgorithms.h>
#include <CTiglError.h>

#include <algorithm>

namespace tigl
{

CTiglConcatSurfaces::CTiglConcatSurfaces(const std::vector<Handle(Geom_BSplineSurface)>& surfaces,
                                         const std::vector<double>& surfaceParams, ConcatDir dir)
    : m_surfaces(surfaces)
    , m_params(surfaceParams)
    , m_dir(dir)
{

    if (surfaces.size() + 1 != surfaceParams.size()) {
        throw CTiglError("Surfaces dont match parameters: Number of parameters != number of surfaces + 1");
    }

    if (dir ==ConcatDir::v) {
        throw CTiglError("CTiglConcatSurfaces in v-direction not yet implemented");
    }

    std::sort(m_params.begin(), m_params.end());
}

void CTiglConcatSurfaces::SetApproxInputSurfacesEnabled(unsigned int nUSeg, unsigned int nVSeg)
{
    if (nUSeg > 0 && nVSeg > 0) {
        m_approxSegments.reset({nUSeg, nVSeg});
    }
    else {
        m_approxSegments.reset();
    }
}

Handle(Geom_BSplineSurface) CTiglConcatSurfaces::Surface() const
{
    // create a copy of all surfaces
    std::vector<Handle(Geom_BSplineSurface)> surfaces;
    std::transform(std::begin(m_surfaces), std::end(m_surfaces), std::back_inserter(surfaces),
                   [](const Handle(Geom_BSplineSurface) & surf) {
                       return Handle(Geom_BSplineSurface)::DownCast(surf->Copy());
                   });

    if (m_approxSegments) {
        std::transform(
            std::begin(surfaces), std::end(surfaces), std::begin(surfaces), [this](Handle(Geom_BSplineSurface) & surf) {
                return CTiglBSplineAlgorithms::approxSurface(surf, m_approxSegments->nu, m_approxSegments->nv);
            });
    }

    for (size_t surfIdx = 0; surfIdx < surfaces.size(); ++surfIdx) {
        CTiglBSplineAlgorithms::reparametrizeBSpline(*surfaces[surfIdx], m_params[surfIdx], m_params[surfIdx + 1], 0.,
                                                     1., 1e-10);
    }

    for (size_t surfIdx = 1; surfIdx < surfaces.size(); ++surfIdx) {
        surfaces[0] = CTiglBSplineAlgorithms::concatSurfacesUDir(surfaces[0], surfaces[surfIdx]);
    }

    return surfaces[0];
}

} // namespace tigl
