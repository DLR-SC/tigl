/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
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

#include "CTiglCompoundSurface.h"

#include "CTiglError.h"
#include <algorithm>

namespace tigl
{

CTiglCompoundSurface::CTiglCompoundSurface(const std::vector<Handle(Geom_BoundedSurface)>& surfaces, const std::vector<double>& uparams)
    : m_surfaces(surfaces)
    , m_uparams(uparams)
{
    if (m_surfaces.size() + 1 != m_uparams.size()) {
        throw CTiglError("Wrong number of parameters in CTiglCompoundSurface");
    }

    if (!std::is_sorted(std::begin(m_uparams), std::end(m_uparams))) {
        throw CTiglError("Parameters not sorted in CTiglCompoundSurface");
    }
}

gp_Pnt CTiglCompoundSurface::Value(double u, double v) const
{
    double tol = 1e-12;
    for (size_t idx = 0; idx < m_surfaces.size(); ++idx) {
        if (m_uparams[idx] - tol <= u && u <= m_uparams[idx+1] + tol) {
            const auto& surface = m_surfaces[idx];

            double u1, u2, v1, v2;
            surface->Bounds(u1, u2, v1, v2);

            double us = u1 + (u - m_uparams[idx])/(m_uparams[idx+1]-m_uparams[idx])*(u2 - u1);
            double vs = (1.-v) * v1 + v*v2;
            return surface->Value(us, vs);
        }
    }

    throw CTiglError("U pararmeter out of bounds in CTiglCompoundSurface::Value");
}


} // namespace tigl
