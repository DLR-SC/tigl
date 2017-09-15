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

#ifndef CCPACSWINGSECTIONS_H
#define CCPACSWINGSECTIONS_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSWingSection.h"
#include "CTiglError.h"
#include <string>
#include <vector>

namespace tigl
{

class CCPACSWingSections
{

private:
    // Typedef for a CCPACSWingSection container to store the sections of a wing.
    typedef std::vector<CCPACSWingSection*> CCPACSWingSectionContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSWingSections(void);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingSections(void);

    // Read CPACS wing sections element
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath);

    // Write CPACS wing sections element
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath) const;

    // Get section count
    TIGL_EXPORT int GetSectionCount(void) const;

    // Returns the section for a given index
    TIGL_EXPORT CCPACSWingSection& GetSection(int index) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSWingSections(const CCPACSWingSections& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CCPACSWingSections& ) { /* Do nothing */ }

private:
    CCPACSWingSectionContainer sections;       /**< Section elements */

};

} // end namespace tigl

#endif // CCPACSWINGSECTIONS_H
