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

#ifndef CTIGLCOMPOUNDSURFACE_H
#define CTIGLCOMPOUNDSURFACE_H

#include "tigl_internal.h"

#include <gp_Pnt.hxx>
#include <Geom_BoundedSurface.hxx>
#include <vector>

namespace tigl
{

class CTiglCompoundSurface
{
public:
    TIGL_EXPORT CTiglCompoundSurface() = default;
    /**
     * @brief CTiglCompoundSurface
     * @param surfaces A list of (adjacent) surfaces
     * @param uparams Ordered list of parameters. Must be one entry more than surfaces
     */
    TIGL_EXPORT CTiglCompoundSurface(const std::vector<Handle(Geom_BoundedSurface)>& surfaces,
                                     const std::vector<double>& uparams);

    TIGL_EXPORT gp_Pnt Value(double u, double v) const;

    TIGL_EXPORT CTiglCompoundSurface& operator=(const CTiglCompoundSurface& other) = default;

private:
    std::vector<Handle(Geom_BoundedSurface)> m_surfaces;
    std::vector<double> m_uparams;
};

} // namespace tigl

#endif // CTIGLCOMPOUNDSURFACE_H
