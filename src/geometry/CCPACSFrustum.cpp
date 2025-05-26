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

#include "CCPACSFrustum.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

#include "generated/CPACSElementGeometry.h"

namespace tigl
{

CCPACSFrustum::CCPACSFrustum(CCPACSElementGeometry* parent)
    : generated::CPACSFrustum(parent)
{
}

CCPACSFrustum::CCPACSFrustum(CCPACSElementGeometryAddtionalPart* parent)
    : generated::CPACSFrustum(parent)
{
}

double CCPACSFrustum::getUpperRadius() const
{
    return m_upperRadius.get_value_or(GetLowerRadius());
}

} // namespace tigl
