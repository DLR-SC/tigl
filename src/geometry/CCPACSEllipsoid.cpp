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

#include "CCPACSEllipsoid.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSEllipsoid::CCPACSEllipsoid(CCPACSElementGeometry* parent)
    : generated::CPACSEllipsoid(parent)
{
}

CCPACSEllipsoid::CCPACSEllipsoid(CCPACSElementGeometryAddtionalPart* parent)
    : generated::CPACSEllipsoid(parent)
{
}

double CCPACSEllipsoid::getRadiusY() const
{
    return m_radiusY.get_value_or(GetRadiusX());
}

double CCPACSEllipsoid::getRadiusZ() const
{
    return m_radiusZ.get_value_or(GetRadiusX());
}

} // namespace tigl
