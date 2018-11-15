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
* @brief  Implementation of CPACS wing sections handling routines.
*/

#include "CCPACSWingSections.h"
#include "CTiglError.h"

namespace tigl
{
CCPACSWingSections::CCPACSWingSections(CCPACSWing* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSections(parent, uidMgr) {}

CCPACSWingSections::CCPACSWingSections(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSections(parent, uidMgr) {}

// Get section count
int CCPACSWingSections::GetSectionCount() const
{
    return static_cast<int>(m_sections.size());
}

// Returns the section for a given index
CCPACSWingSection& CCPACSWingSections::GetSection(int index) const
{
    index--;
    if (index < 0 || index >= GetSectionCount()) {
        throw CTiglError("Invalid index in CCPACSWingSections::GetSection", TIGL_INDEX_ERROR);
    }
    return *m_sections[index];
}
} // end namespace tigl
