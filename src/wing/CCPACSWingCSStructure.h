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
#include "CTiglWingStructureReference.h"


namespace tigl
{

class CCPACSWingCSStructure // shouldn't his be CCPACSWingComponentSegmentStructure
{
public:
    TIGL_EXPORT CCPACSWingCSStructure(CTiglWingStructureReference parent);
    
    TIGL_EXPORT virtual ~CCPACSWingCSStructure(void);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& structureXPath);
    
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & structureXPath) const;

    TIGL_EXPORT CTiglWingStructureReference& GetWingStructureReference();
    TIGL_EXPORT const CTiglWingStructureReference& GetWingStructureReference() const;

    TIGL_EXPORT CCPACSWingShell& GetLowerShell();
    TIGL_EXPORT CCPACSWingShell& GetUpperShell();
    
    TIGL_EXPORT void Cleanup();
    TIGL_EXPORT void Invalidate();
    TIGL_EXPORT bool IsValid() const;

private:
    CCPACSWingCSStructure(const CCPACSWingCSStructure&);
    void operator=(const CCPACSWingCSStructure&);

private:
    CCPACSWingShell upperShell, lowerShell;
    CTiglWingStructureReference wingStructureReference;

    bool isvalid;
};

} // namespace tigl

#endif // CCPACSWINGCSSTRUCTURE_H
