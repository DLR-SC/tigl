/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Implementation of CPACS fuselage section handling routines.
*/

#include "CCPACSFuselageSection.h"
#include "CTiglError.h"
#include <iostream>

namespace tigl
{
CCPACSFuselageSection::CCPACSFuselageSection(CCPACSFuselageSections* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSection(parent, uidMgr) {}

// Cleanup routine
void CCPACSFuselageSection::Cleanup()
{
    m_name = "";
    m_uID = "";
    m_transformation.reset();
}

// Read CPACS section elements
void CCPACSFuselageSection::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
{
    Cleanup();
    generated::CPACSFuselageSection::ReadCPACS(tixiHandle, sectionXPath);
}

// Get profile count for this section
int CCPACSFuselageSection::GetSectionElementCount() const
{
    return m_elements.GetSectionElementCount();
}

// Get element for a given index
CCPACSFuselageSectionElement& CCPACSFuselageSection::GetSectionElement(int index) const
{
    return m_elements.GetSectionElement(index);
}

// Gets the section transformation
CTiglTransformation CCPACSFuselageSection::GetSectionTransformation() const
{
    return m_transformation.getTransformationMatrix();
}

} // end namespace tigl
