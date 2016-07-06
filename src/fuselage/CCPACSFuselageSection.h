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

#ifndef CCPACSFUSELAGESECTION_H
#define CCPACSFUSELAGESECTION_H

#include "generated/CPACSFuselageSection.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSFuselageSectionElements.h"
#include "CTiglTransformation.h"
#include <string>

namespace tigl
{

class CCPACSFuselageSection : public generated::CPACSFuselageSection
{
public:
    // Read CPACS section elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath);

    // Get element count for this section
    TIGL_EXPORT int GetSectionElementCount() const;

    // Get element for a given index
    TIGL_EXPORT CCPACSFuselageSectionElement& GetSectionElement(int index) const;

    // Gets the section transformation
    TIGL_EXPORT CTiglTransformation GetSectionTransformation() const;

protected:
    // Cleanup routine
    void Cleanup();
};

} // end namespace tigl

#endif // CCPACSFUSELAGESECTION_H
