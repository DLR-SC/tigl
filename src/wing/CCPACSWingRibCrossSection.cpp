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

#include "CCPACSWingRibCrossSection.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"


namespace tigl
{
CCPACSWingRibCrossSection::CCPACSWingRibCrossSection(CPACSPylonRibsDefinition * parent)
    : generated::CPACSWingRibCrossSection(parent)
{
}

CCPACSWingRibCrossSection::CCPACSWingRibCrossSection(CCPACSWingRibsDefinition* parent)
    : generated::CPACSWingRibCrossSection(parent)
{
}

double CCPACSWingRibCrossSection::GetXRotation() const
{
    if (m_ribRotation)
        return m_ribRotation->GetX();
    else
        return 90;
}

void CCPACSWingRibCrossSection::SetXRotation(double rotation)
{
    if (!m_ribRotation)
        m_ribRotation = boost::in_place();
    m_ribRotation->SetX(rotation);

    // invalidate whole component segment structure, since rib could be referenced anywher
    GetParent<CCPACSWingRibsDefinition>()->GetStructure().Invalidate();
}
} // end namespace tigl
