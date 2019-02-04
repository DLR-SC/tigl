/* 
* Copyright (C) 2016 Airbus Defence and Space
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
#include "CCPACSWingRibRotation.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"


namespace tigl
{

CCPACSWingRibRotation::CCPACSWingRibRotation(CCPACSWingRibsPositioning* parent)
    : generated::CPACSRibRotation(parent)
{
    m_z = 90;
}

void CCPACSWingRibRotation::SetRibRotationReference(const boost::optional<std::string>& value)
{
    generated::CPACSRibRotation::SetRibRotationReference(value);
    m_parent->invalidateStructure();
}

void CCPACSWingRibRotation::SetZ(const double & value)
{
    generated::CPACSRibRotation::SetZ(value);
    // invalidate whole component segment structure since rib may be referenced anywhere
    m_parent->invalidateStructure();
}

} // end namespace tigl
