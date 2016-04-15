/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSWINGCSSTRUCTURE_H
#define CCPACSWINGCSSTRUCTURE_H

#include "tigl_internal.h"
#include "CCPACSWingShell.h"

// [[CAS_AES]] added include for reference to parent
#include "TiglWingStructureReference.h"

namespace tigl
{

// [[CAS_AES]] added forward declarations
class CCPACSWingSpars;
class CCPACSWingRibsDefinitions;

class CCPACSWingCSStructure
{
public:
    // [[CAS_AES]] added reference to parent component segment
    TIGL_EXPORT CCPACSWingCSStructure(const TiglWingStructureReference& parent);
    
    // [[CAS_AES]] added destructor
    TIGL_EXPORT virtual ~CCPACSWingCSStructure(void);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& structureXPath);
    
    // Write CPACS structure elements
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & structureXPath);

    // [[CAS_AES]] Returns the component segment this structure belongs to
    const TiglWingStructureReference& GetWingStructureReference(void) const;

    TIGL_EXPORT CCPACSWingShell& GetLowerShell();
    TIGL_EXPORT CCPACSWingShell& GetUpperShell();
    
    // [[CAS_AES]] Check if subnode spars is present
    TIGL_EXPORT bool HasSpars() const;

    // [[CAS_AES]] Returns a reference to the spars sub-element
    TIGL_EXPORT CCPACSWingSpars& GetSpars() const;

    // [[CAS_AES]] Check if subnode ribsDefinitions is present
    TIGL_EXPORT bool HasRibsDefinitions() const;

    // [[CAS_AES]] Returns a reference to the ribsDefinitions sub-element
    TIGL_EXPORT CCPACSWingRibsDefinitions& GetRibsDefinitions() const;

    TIGL_EXPORT void Cleanup();
    TIGL_EXPORT void Invalidate();
    TIGL_EXPORT bool IsValid() const;

private:
    CCPACSWingShell upperShell, lowerShell;
    // [[CAS_AES]] added spar and ribs
    CCPACSWingSpars*            spars;             /**< subnode spars*/
    CCPACSWingRibsDefinitions*  ribsDefinitions;   /**< subnode ribsDefinitions*/
    // [[CAS_AES]] added reference to parent element
    TiglWingStructureReference wingStructureReference;

    bool isvalid;
};

} // namespace tigl

#endif // CCPACSWINGCSSTRUCTURE_H
