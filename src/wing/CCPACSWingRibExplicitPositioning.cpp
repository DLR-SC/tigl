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
#include "CCPACSWingRibExplicitPositioning.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"

namespace tigl
{
CCPACSWingRibExplicitPositioning::CCPACSWingRibExplicitPositioning(CCPACSWingRibsDefinition* parent)
    : generated::CPACSWingRibExplicitPositioning(parent)
{
}

void CCPACSWingRibExplicitPositioning::SetStartReference(const std::string& ref)
{
    generated::CPACSWingRibExplicitPositioning::SetStartReference(ref);
    // invalidate whole component segment structure since rib could be referenced anywhere
    m_parent->GetParent()->Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetEndReference(const std::string& ref)
{
    generated::CPACSWingRibExplicitPositioning::SetEndReference(ref);
    // invalidate whole component segment structure since rib could be referenced anywhere
    m_parent->GetParent()->Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetStartEta(double eta)
{
    generated::CPACSWingRibExplicitPositioning::SetEtaStart(eta);
    // invalidate whole component segment structure since rib could be referenced anywhere
    m_parent->GetParent()->Invalidate();
}

void CCPACSWingRibExplicitPositioning::SetEndEta(double eta)
{
    generated::CPACSWingRibExplicitPositioning::SetEtaEnd(eta);
    // invalidate whole component segment structure since rib could be referenced anywhere
    m_parent->GetParent()->Invalidate();
}

void CCPACSWingRibExplicitPositioning::Invalidate()
{
    m_parent->GetParent()->Invalidate();
}
}
