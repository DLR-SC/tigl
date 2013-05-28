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

#include "CCPACSWingCSStructure.h"

#include "CTiglError.h"
#include "CTiglLogger.h"

namespace tigl {

CCPACSWingCSStructure::CCPACSWingCSStructure()
{
    Cleanup();
}

void CCPACSWingCSStructure::Cleanup(){
    upperShell.Reset();
    lowerShell.Reset();
    isvalid = false;
}

void CCPACSWingCSStructure::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &structureXPath){
    Cleanup();
    
    // check path
    if( tixiCheckElement(tixiHandle, structureXPath.c_str()) != SUCCESS){
        LOG(ERROR) << "Wing structure " << structureXPath << " not found in CPACS file!" << std::endl;
        return;
    }
    
    // lower shell
    std::string shellPath;
    shellPath = structureXPath + "/upperShell";
    if ( tixiCheckElement(tixiHandle, shellPath.c_str()) == SUCCESS){
        upperShell.ReadCPACS(tixiHandle, shellPath.c_str());
    }
    
    shellPath = structureXPath + "/lowerShell";
    if ( tixiCheckElement(tixiHandle, shellPath.c_str()) == SUCCESS){
        lowerShell.ReadCPACS(tixiHandle, shellPath.c_str());
    }
    
    isvalid = true;
}

CCPACSWingShell& CCPACSWingCSStructure::GetLowerShell() {
    return lowerShell;
}

CCPACSWingShell& CCPACSWingCSStructure::GetUpperShell() {
    return upperShell;
}

void CCPACSWingCSStructure::Invalidate()
{
    isvalid = false;
}

bool CCPACSWingCSStructure::IsValid() const
{
    return isvalid;
}

} // namespace tigl
