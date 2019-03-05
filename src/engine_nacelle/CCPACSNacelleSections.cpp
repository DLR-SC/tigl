/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleSections.h"
#include "CCPACSNacelleSection.h"
#include "CTiglUIDManager.h"
#include "cassert"

namespace tigl {

CCPACSNacelleSections::CCPACSNacelleSections(CCPACSNacelleCowl* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSNacelleSections(parent, uidMgr)
{}

TIGL_EXPORT size_t CCPACSNacelleSections::GetSectionCount() const
{
    return GetSections().size();
}

TIGL_EXPORT CCPACSNacelleSection& CCPACSNacelleSections::GetSection(size_t index) const
{
    index--;
    if (index < 0 || index >= GetSectionCount()) {
        throw CTiglError("Invalid index in CCPACSNacelleSections::GetSection", TIGL_INDEX_ERROR);
    }
    return *m_sections[index];
}

TIGL_EXPORT CCPACSNacelleSection& CCPACSNacelleSections::GetSection(std::string const uid) const
{
    for (std::size_t i = 0; i < m_sections.size(); ++i) {
        if (m_sections[i]->GetUID() == uid) {
            return *m_sections[i];
        }
    }
    throw CTiglError("CCPACSNacelleSection::GetSection: Section \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

size_t CCPACSNacelleSections::GetSectionIndex(std::string const uid) const
{
    for (std::size_t i = 0; i < m_sections.size(); ++i) {
        if (m_sections[i]->GetUID() == uid) {
            return i+1;
        }
    }
    throw CTiglError("CCPACSNacelleSection::GetSectionIndex: Section \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

} //namepsace tigl
