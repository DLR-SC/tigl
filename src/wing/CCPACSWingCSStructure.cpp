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

// [[CAS_AES]] BEGIN
#include "CCPACSWingSpars.h"
#include "CCPACSWingRibsDefinitions.h"
// [[CAS_AES]] END

namespace tigl
{

// [[CAS_AES]] added reference to parent component segment
// [[CAS_AES]] added parent reference to shells
// [[CAS_AES]] added initialization of spar and rib pointers
CCPACSWingCSStructure::CCPACSWingCSStructure(const TiglWingStructureReference& parent)
: wingStructureReference(parent),
  lowerShell(parent, LOWER_SIDE),
  upperShell(parent, UPPER_SIDE),
  spars(NULL),
  ribsDefinitions(NULL)
{
    Cleanup();
}

// [[CAS_AES]] added destructor
CCPACSWingCSStructure::~CCPACSWingCSStructure(void)
{
    Cleanup();
}

void CCPACSWingCSStructure::Cleanup()
{
    upperShell.Reset();
    lowerShell.Reset();
    // [[CAS_AES]] added spars
    if (spars != NULL) {
        delete spars;
        spars = NULL;
    }
    // [[CAS_AES]] added ribs
    if (ribsDefinitions != NULL) {
        delete ribsDefinitions;
        ribsDefinitions = NULL;
    }
    isvalid = false;
}

void CCPACSWingCSStructure::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &structureXPath)
{
    Cleanup();
    
    // check path
    if ( tixiCheckElement(tixiHandle, structureXPath.c_str()) != SUCCESS) {
        LOG(WARNING) << "Wing structure " << structureXPath << " not found in CPACS file!" << std::endl;
        return;
    }

    // [[CAS_AES]] added spars
    std::string sparsPath = structureXPath + "/spars";
    if ( tixiCheckElement(tixiHandle, sparsPath.c_str()) == SUCCESS){
        spars = new CCPACSWingSpars(wingStructureReference);
        spars->ReadCPACS(tixiHandle, sparsPath);
    }

    // [[CAS_AES]] added ribs
    std::string ribsPath = structureXPath + "/ribsDefinitions";
    if ( tixiCheckElement(tixiHandle, ribsPath.c_str()) == SUCCESS){
        ribsDefinitions = new CCPACSWingRibsDefinitions(wingStructureReference);
        ribsDefinitions->ReadCPACS(tixiHandle, ribsPath);
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

// Write CPACS structure elements
void CCPACSWingCSStructure::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& structureXPath)
{
    std::string elementPath;

    // for each element, if it exist in the model, the function call the saving subfunction
    // if not existing in the model, it try to remove an eventuel pre-existing subsection (no error if not pre-existing)
    elementPath = structureXPath + "/upperShell";
    // create the subelement Spars
    TixiSaveExt::TixiSaveElement(tixiHandle,structureXPath.c_str(), "upperShell");
    upperShell.WriteCPACS(tixiHandle, elementPath);

    elementPath = structureXPath + "/lowerShell";
    // create the subelement Spars
    TixiSaveExt::TixiSaveElement(tixiHandle,structureXPath.c_str(), "lowerShell");
    lowerShell.WriteCPACS(tixiHandle, elementPath);

    elementPath = structureXPath + "/spars";
    if(HasSpars() == true) {
        // create the subelement Spars
        TixiSaveExt::TixiSaveElement(tixiHandle,structureXPath.c_str(), "spars");
        spars->WriteCPACS(tixiHandle, elementPath);
    }
    else {
        tixiRemoveElement(tixiHandle, elementPath.c_str());
    }

    elementPath = structureXPath + "/ribsDefinitions";
    if (HasRibsDefinitions() == true) {
        // create the subelement RibsDefinitions
        TixiSaveExt::TixiSaveElement(tixiHandle, structureXPath.c_str(), "ribsDefinitions");
        ribsDefinitions->WriteCPACS(tixiHandle, elementPath);
    }
    else {
        tixiRemoveElement(tixiHandle, elementPath.c_str());
    }
}

// [[CAS_AES]] Returns the component segment this structure belongs to
const TiglWingStructureReference& CCPACSWingCSStructure::GetWingStructureReference(void) const
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

// [[CAS_AES]] Check if subnode spars is present
bool CCPACSWingCSStructure::HasSpars() const
{
    return (spars != NULL);
}

// [[CAS_AES]] Returns a reference to the spars sub-element
CCPACSWingSpars& CCPACSWingCSStructure::GetSpars() const
{
    if (!spars) {
        throw CTiglError("Error: spars not available but requested in CCPACSWingCSStructure::GetSpars!");
    }
    return *spars;
}

// [[CAS_AES]] Check if subnode ribsDefinitions is present
bool CCPACSWingCSStructure::HasRibsDefinitions() const
{
    return (ribsDefinitions != NULL);
}

// [[CAS_AES]] Returns a reference to the ribsDefinitions sub-element
CCPACSWingRibsDefinitions& CCPACSWingCSStructure::GetRibsDefinitions() const
{
    if (!ribsDefinitions) {
        throw CTiglError("Error: ribsDefinitions not available but requested in CCPACSWingCSStructure::GetRibsDefinitions!");
    }
    return *ribsDefinitions;
}

void CCPACSWingCSStructure::Invalidate()
{
    isvalid = false;
    // [[CAS_AES]] invalidate also spars and ribs
    if (spars != NULL) {
        spars->Invalidate();
    }
    if (ribsDefinitions != NULL) {
        ribsDefinitions->Invalidate();
    }

    upperShell.Invalidate();

    lowerShell.Invalidate();
}

bool CCPACSWingCSStructure::IsValid() const
{
    return isvalid;
}

} // namespace tigl
