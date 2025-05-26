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

#ifndef CCPACSFRUSTUM_H
#define CCPACSFRUSTUM_H

#include "generated/CPACSFrustum.h"

namespace tigl
{

class CCPACSFrustum : public generated::CPACSFrustum
{
public:
    TIGL_EXPORT CCPACSFrustum(CCPACSElementGeometry* parent);
    TIGL_EXPORT CCPACSFrustum(CCPACSElementGeometryAddtionalPart* parent);

    /**
		 * @brief According to the CPACS documentation, this method either returns the
		 * value from CPACS or the default value, which equals the lower radius.
		 */
    TIGL_EXPORT double getUpperRadius() const;

private:
};

} // namespace tigl
#endif // CCPACSFRUSTUM_H
