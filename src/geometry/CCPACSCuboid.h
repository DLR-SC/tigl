/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2025-04-22 Marko Alder <Marko.Alder@dlr.de>
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

#ifndef CCPACSCuboid_H
#define CCPACSCuboid_H

#include "generated/CPACSCuboid.h"

namespace tigl
{

class CCPACSCuboid : public generated::CPACSCuboid
{
public:
    TIGL_EXPORT CCPACSCuboid(CCPACSElementGeometry* parent);
    TIGL_EXPORT CCPACSCuboid(CCPACSElementGeometryAddtionalPart* parent);

    /**
     * @brief Returns either value from CPACS or default.
     */
    TIGL_EXPORT double getAlpha() const;
    TIGL_EXPORT double getBeta() const;
    TIGL_EXPORT double getGamma() const;

private:
    double _default_alpha;
    double _default_beta;
    double _default_gamma;
};

} // namespace tigl
#endif // CCPACSCuboid_H
