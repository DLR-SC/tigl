/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-17 Marko Alder <marko.alder@dlr.de>
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

#include "CCPACSDeck.h"
#include "generated/CPACSDecks.h"
#include "CCPACSFuselage.h"

namespace tigl
{

// Constructor
CCPACSDeck::CCPACSDeck(CCPACSDecks* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSDeck(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(
          &m_parentUID,
          &m_transformation) // ToDo: if m_parentUID is not given, the parent should be the parent fuselage
{
}

// Destructor
CCPACSDeck::~CCPACSDeck()
{
}

void CCPACSDeck::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    Reset();
    generated::CPACSDeck::ReadCPACS(tixiHandle, xpath);

    // Default parent: if no explicit parentUID is given, place the deck
    // relative to the surrounding fuselage.
    if ((!m_parentUID || m_parentUID->empty()) && m_parent && m_parent->GetParent()) {
        generated::CPACSDeck::SetParentUID(boost::optional<std::string>(m_parent->GetParent()->GetUID()));
    }
}

std::string CCPACSDeck::GetDefaultedUID() const
{
    return GetUID();
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSDeck::GetConfiguration() const
{
    return m_parent->GetParent()->GetConfiguration();
}

// build loft
PNamedShape CCPACSDeck::BuildLoft() const
{
    // ToDo: implement get shapes
    PNamedShape groupedShape;

    return groupedShape;
}

} // end namespace tigl
