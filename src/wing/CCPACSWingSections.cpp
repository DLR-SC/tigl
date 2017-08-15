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
* @brief  Implementation of CPACS wing sections handling routines.
*/

#include "CCPACSWingSections.h"
#include "TixiSaveExt.h"
#include "IOHelper.h"
#include <iostream>
#include <sstream>

namespace tigl
{

// Constructor
CCPACSWingSections::CCPACSWingSections(void)
{
    Cleanup();
}

// Destructor
CCPACSWingSections::~CCPACSWingSections(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingSections::Cleanup(void)
{
    for (CCPACSWingSectionContainer::size_type i = 0; i < sections.size(); i++) {
        delete sections[i];
    }
    sections.clear();
}

// Read CPACS wing sections element
void CCPACSWingSections::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    Cleanup();
    ReadContainerElement(tixiHandle, xpath, "section", 2, sections);
}

// Write CPACS wing sections element
void CCPACSWingSections::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    WriteContainerElement(tixiHandle, xpath, "section", sections);
}

// Get section count
int CCPACSWingSections::GetSectionCount(void) const
{
    return static_cast<int>(sections.size());
}

// Returns the section for a given index
CCPACSWingSection& CCPACSWingSections::GetSection(int index) const
{
    index--;
    if (index < 0 || index >= GetSectionCount()) {
        throw CTiglError("Error: Invalid index in CCPACSWingSections::GetSection", TIGL_INDEX_ERROR);
    }
    return (*sections[index]);
}

} // end namespace tigl
