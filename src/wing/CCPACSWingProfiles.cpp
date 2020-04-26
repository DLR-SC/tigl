/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
/**
* @file
* @brief  Implementation of CPACS wing profiles handling routines.
*/

#include <iostream>
#include <algorithm>

#include "generated/CPACSWingAirfoils.h"
#include "CCPACSWingProfile.h"
#include "tigl_internal.h"
#include "CCPACSWingProfiles.h"
#include "CTiglError.h"
#include "generated/TixiHelper.h"
#include "generated/UniquePtr.h"
#include "CTiglLogging.h"

namespace tigl
{
CCPACSWingProfiles::CCPACSWingProfiles(CCPACSProfiles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingAirfoils(parent, uidMgr) {}

// Invalidates internal state
void CCPACSWingProfiles::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 0; i < m_wingAirfoils.size(); i++) {
        static_cast<const CCPACSWingProfile&>(*m_wingAirfoils[i]).Invalidate(source);
    }
}

// Read CPACS wing profiles
void CCPACSWingProfiles::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // call separate import method for reading (allows for importing
    // additional profiles from other files)
    m_wingAirfoils.clear();
    ImportCPACS(tixiHandle, xpath);
}

void CCPACSWingProfiles::ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    // we replace generated::CPACSWingAirfoils::ReadCPACS and not call it to allow instantiation of CCPACSWingProfile instead of generated::CPACSProfileGeometry
    // read element wingAirfoil
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/wingAirfoil")) {
        tixi::TixiReadElementsInternal(tixiHandle, xpath + "/wingAirfoil", m_wingAirfoils, 1, tixi::xsdUnbounded, [&](const std::string& childXPath) {
            auto child = tigl::make_unique<CCPACSWingProfile>(this, m_uidMgr);
            child->ReadCPACS(tixiHandle, childXPath);
            return child;
        });
    }
}

CCPACSWingProfile& CCPACSWingProfiles::AddWingAirfoil() {
    m_wingAirfoils.push_back(make_unique<CCPACSWingProfile>(this, m_uidMgr));
    return static_cast<CCPACSWingProfile&>(*m_wingAirfoils.back());
}

bool CCPACSWingProfiles::HasProfile(std::string uid) const
{
    for (const auto& p : m_wingAirfoils)
        if (p->GetUID() == uid)
            return true;

    return false;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSWingProfiles::GetProfile(std::string uid) const
{
    for (auto& p : m_wingAirfoils)
        if (p->GetUID() == uid)
            return static_cast<CCPACSWingProfile&>(*p);
    throw CTiglError("Fuselage profile \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

} // end namespace tigl
