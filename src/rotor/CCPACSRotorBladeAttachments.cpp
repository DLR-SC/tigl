/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-11 Philipp Kunze <Philipp.Kunze@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
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
/**
* @file
* @brief  Implementation of CPACS rotor blade attachments handling routines.
*/

#include "CCPACSRotorBladeAttachments.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CCPACSRotorHub.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSRotorBladeAttachments::CCPACSRotorBladeAttachments(CCPACSRotorHub* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSRotorBladeAttachments(parent, uidMgr) {}

// Invalidates internal state
void CCPACSRotorBladeAttachments::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 1; i <= GetRotorBladeAttachmentCount(); i++) {
        GetRotorBladeAttachment(i).Invalidate(source);
    }
}

// Returns the parent rotor
const CCPACSRotor& CCPACSRotorBladeAttachments::GetRotor() const
{
    return *m_parent->GetParent();
}

CCPACSRotor& CCPACSRotorBladeAttachments::GetRotor()
{
    return *m_parent->GetParent();
}

// Returns the parent configuration
const CCPACSConfiguration& CCPACSRotorBladeAttachments::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

CCPACSConfiguration& CCPACSRotorBladeAttachments::GetConfiguration()
{
    return m_parent->GetConfiguration();
}

} // end namespace tigl
