/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSWingCSStructure.h"

#include "CTiglError.h"
#include "CTiglLogging.h"

#include "CCPACSWingCell.h"
#include "CCPACSWingSpars.h"
#include "CCPACSWingSparSegment.h"
#include "CCPACSWingSparSegments.h"
#include "CCPACSWingRibsDefinitions.h"
#include "CCPACSWingComponentSegment.h"
//#include "CCPACSTrailingEdgeDevice.h"

namespace tigl
{
CCPACSWingCSStructure::CCPACSWingCSStructure(CCPACSWingComponentSegment* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingComponentSegmentStructure(parent, uidMgr)
{
}

CCPACSWingCSStructure::CCPACSWingCSStructure(CCPACSTrailingEdgeDevice* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingComponentSegmentStructure(parent, uidMgr)
{
}

int CCPACSWingCSStructure::GetSparSegmentCount() const
{
    if (m_spars) {
        return m_spars->GetSparSegments().GetSparSegmentCount();
    }
    return 0;
}

CCPACSWingSparSegment& CCPACSWingCSStructure::GetSparSegment(int index)
{
    if (!m_spars) {
        throw CTiglError("no spars existing in CCPACSWingCSStructure::GetSparSegment!");
    }
    return m_spars->GetSparSegments().GetSparSegment(index);
}

CCPACSWingSparSegment& CCPACSWingCSStructure::GetSparSegment(const std::string& uid)
{
    if (!m_spars) {
        throw CTiglError("no spars existing in CCPACSWingCSStructure::GetSparSegment!");
    }
    return m_spars->GetSparSegments().GetSparSegment(uid);
}

const CCPACSWingSparSegment& CCPACSWingCSStructure::GetSparSegment(int index) const
{
    if (!m_spars) {
        throw CTiglError("no spars existing in CCPACSWingCSStructure::GetSparSegment!");
    }
    return m_spars->GetSparSegments().GetSparSegment(index);
}

const CCPACSWingSparSegment& CCPACSWingCSStructure::GetSparSegment(const std::string& uid) const
{
    if (!m_spars) {
        throw CTiglError("no spars existing in CCPACSWingCSStructure::GetSparSegment!");
    }
    return m_spars->GetSparSegments().GetSparSegment(uid);
}

int CCPACSWingCSStructure::GetRibsDefinitionCount() const
{
    if (m_ribsDefinitions) {
        return m_ribsDefinitions->GetRibsDefinitionCount();
    }
    return 0;
}

CCPACSWingRibsDefinition& CCPACSWingCSStructure::GetRibsDefinition(int index)
{
    if (!m_ribsDefinitions) {
        throw CTiglError("no ribsDefinitions existing in CCPACSWingCSStructure::GetRibsDefinition!");
    }
    return m_ribsDefinitions->GetRibsDefinition(index);
}

CCPACSWingRibsDefinition& CCPACSWingCSStructure::GetRibsDefinition(const std::string& uid)
{
    if (!m_ribsDefinitions) {
        throw CTiglError("no ribsDefinitions existing in CCPACSWingCSStructure::GetRibsDefinition!");
    }
    return m_ribsDefinitions->GetRibsDefinition(uid);
}

const CCPACSWingRibsDefinition& CCPACSWingCSStructure::GetRibsDefinition(int index) const
{
    if (!m_ribsDefinitions) {
        throw CTiglError("no ribsDefinitions existing in CCPACSWingCSStructure::GetRibsDefinition!");
    }
    return m_ribsDefinitions->GetRibsDefinition(index);
}

const CCPACSWingRibsDefinition& CCPACSWingCSStructure::GetRibsDefinition(const std::string& uid) const
{
    if (!m_ribsDefinitions) {
        throw CTiglError("no ribsDefinitions existing in CCPACSWingCSStructure::GetRibsDefinition!");
    }
    return m_ribsDefinitions->GetRibsDefinition(uid);
}


void CCPACSWingCSStructure::Invalidate()
{
    // forward invalidation
    if (m_spars) {
        m_spars->Invalidate();
    }
    if (m_ribsDefinitions) {
        m_ribsDefinitions->Invalidate();
    }
    m_upperShell.Invalidate();
    m_lowerShell.Invalidate();
}

} // namespace tigl
