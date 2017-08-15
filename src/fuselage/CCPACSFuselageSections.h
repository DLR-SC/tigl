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
* @brief  Implementation of CPACS fuselage sections handling routines.
*/

#ifndef CCPACSFUSELAGESECTIONS_H
#define CCPACSFUSELAGESECTIONS_H

#include "tigl_internal.h"
#include "tixi.h"
#include <string>
#include <vector>

namespace tigl
{

class CCPACSFuselageSection;

class CCPACSFuselageSections
{

private:
    // Typedef for a CCPACSFuselageSection container to store the sections of a fuselage.
    typedef std::vector<CCPACSFuselageSection*> CCPACSFuselageSectionContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSFuselageSections(void);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSFuselageSections(void);

    // Read CPACS fuselage sections element
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath);

    // Write CPACS fuselage sections element
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& fuselageXPath);

    // Get section count
    TIGL_EXPORT int GetSectionCount(void) const;

    // Returns the section for a given index
    TIGL_EXPORT CCPACSFuselageSection& GetSection(int index) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSFuselageSections(const CCPACSFuselageSections& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CCPACSFuselageSections& ) { /* Do nothing */ }

private:
    CCPACSFuselageSectionContainer sections;       /**< Section elements */

};

} // end namespace tigl

#endif // CCPACSFUSELAGESECTIONS_H
