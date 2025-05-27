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

#include "CCPACSCuboid.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

namespace tigl
{

CCPACSCuboid::CCPACSCuboid(CCPACSElementGeometry* parent)
    : generated::CPACSCuboid(parent)
{
}

CCPACSCuboid::CCPACSCuboid(CCPACSElementGeometryAddtionalPart* parent)
    : generated::CPACSCuboid(parent)
{
}

// ToDo: Should this be based on the given elements only or should we also evaluate its values, e.g. false if all angles are 90degs?
// Note: Exceptions for incomplete data sets (only alpha, but no beta or gamma would be invalid according to XSD) should be caught 
// by the generated classes, I would say.
bool CCPACSCuboid::isRectangularCuboid() const
{
    return !isParallelepiped() && !isWedge();
}

bool CCPACSCuboid::isParallelepiped() const
{
    return m_alpha_choice1 && m_beta_choice1 && m_gamma_choice1;
}

bool CCPACSCuboid::isWedge() const
{
    return m_upperFaceXmin_choice2 && m_upperFaceXmax_choice2 && m_upperFaceYmin_choice2 && m_upperFaceYmax_choice2;
}

} // namespace tigl
