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
#include "CTiglLogging.h"
#include "TixiSaveExt.h"

#include "CCPACSWingCell.h"

namespace tigl
{


CCPACSWingCSStructure::CCPACSWingCSStructure(CTiglWingStructureReference parent)
: wingStructureReference(parent),
  lowerShell(*this, LOWER_SIDE),
  upperShell(*this, UPPER_SIDE)
{
    Cleanup();
}

CCPACSWingCSStructure::~CCPACSWingCSStructure(void)
{
    Cleanup();
}

void CCPACSWingCSStructure::Cleanup()
{
    upperShell.Reset();
    lowerShell.Reset();
    isvalid = false;
}

void CCPACSWingCSStructure::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &structureXPath)
{
    Cleanup();
    
    // check path
    if (tixiCheckElement(tixiHandle, structureXPath.c_str()) != SUCCESS) {
        LOG(WARNING) << "Wing structure " << structureXPath << " not found in CPACS file!" << std::endl;
        return;
    }

    // lower shell
    std::string upperShellPath = structureXPath + "/upperShell";
    if ( tixiCheckElement(tixiHandle, upperShellPath.c_str()) == SUCCESS){
        upperShell.ReadCPACS(tixiHandle, upperShellPath.c_str());
    }
    
    std::string lowerShellPath = structureXPath + "/lowerShell";
    if ( tixiCheckElement(tixiHandle, lowerShellPath.c_str()) == SUCCESS){
        lowerShell.ReadCPACS(tixiHandle, lowerShellPath.c_str());
    }
    
    isvalid = true;
}

// Write CPACS structure elements
void CCPACSWingCSStructure::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& structureXPath) const
{
    // for each element, if it exist in the model, the function call the saving subfunction
    // if not existing in the model, it try to remove an eventuel pre-existing subsection (no error if not pre-existing)
    // create the subelement Spars
    TixiSaveExt::TixiSaveElement(tixiHandle,structureXPath.c_str(), "upperShell");
    upperShell.WriteCPACS(tixiHandle, structureXPath + "/upperShell");

    // create the subelement Spars
    TixiSaveExt::TixiSaveElement(tixiHandle,structureXPath.c_str(), "lowerShell");
    lowerShell.WriteCPACS(tixiHandle, structureXPath + "/lowerShell");
}

CTiglWingStructureReference& CCPACSWingCSStructure::GetWingStructureReference()
{
    return wingStructureReference;
}

const CTiglWingStructureReference& CCPACSWingCSStructure::GetWingStructureReference() const
{
    return wingStructureReference;
}

CCPACSWingShell& CCPACSWingCSStructure::GetLowerShell()
{
    return lowerShell;
}

CCPACSWingShell& CCPACSWingCSStructure::GetUpperShell()
{
    return upperShell;
}

void CCPACSWingCSStructure::Invalidate()
{
    // forward invalidation
    upperShell.Invalidate();
    lowerShell.Invalidate();
}

bool CCPACSWingCSStructure::IsValid() const
{
    return isvalid;
}

} // namespace tigl
