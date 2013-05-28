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

#include "CCPACSWingShell.h"

namespace tigl {

class CCPACSWingCSStructure
{
public:
    CCPACSWingCSStructure();
    
    void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& structureXPath);
    
    CCPACSWingShell& GetLowerShell();
    CCPACSWingShell& GetUpperShell();
    
    void Cleanup();
    void Invalidate();
    bool IsValid() const;
private:
    CCPACSWingShell upperShell, lowerShell;
    
    bool isvalid;
};

} // namespace tigl

#endif // CCPACSWINGCSSTRUCTURE_H
