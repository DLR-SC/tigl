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
#include "CCPACSWingSpars.h"
#include "CCPACSWingSparSegment.h"
#include "CCPACSWingSparSegments.h"
#include "CCPACSWingRibsDefinitions.h"

namespace tigl
{


CCPACSWingCSStructure::CCPACSWingCSStructure(CTiglWingStructureReference parent)
: wingStructureReference(parent),
  spars(NULL),
  ribsDefinitions(NULL),
  lowerShell(*this, LOWER_SIDE),
  upperShell(*this, UPPER_SIDE)
{
    Cleanup();
}

}

void CCPACSWingCSStructure::Cleanup()
{
    upperShell.Reset();
    lowerShell.Reset();
    if (spars != NULL) {
        delete spars;
        spars = NULL;
    }
    if (ribsDefinitions != NULL) {
        delete ribsDefinitions;
        ribsDefinitions = NULL;
    }
    isvalid = false;
}
void CCPACSWingCSStructure::ReadCPACS(const TixiDocumentHandle& tixiDocument, const std::string& xpath) {
    generated::CPACSWingComponentSegmentStructure::ReadCPACS(tixiDocument, xpath);
    isvalid = true;
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

bool CCPACSWingCSStructure::HasSpars() const
{
    return (spars != NULL);
}

CCPACSWingSpars& CCPACSWingCSStructure::GetSpars() const
{
    if (!spars) {
        throw CTiglError("Error: spars not available but requested in CCPACSWingCSStructure::GetSpars!");
    }
    return *spars;
}

int CCPACSWingCSStructure::GetSparSegmentCount() const
{
    int sparSegmentCount = 0;

    if (spars) {
        sparSegmentCount = spars->GetSparSegments().GetSparSegmentCount();
    }

    return sparSegmentCount;
}

CCPACSWingSparSegment& CCPACSWingCSStructure::GetSparSegment(int index) const
{
    if (!spars) {
        throw CTiglError("Error: no spars existing in CCPACSWingCSStructure::GetSparSegment!");
    }
    return spars->GetSparSegments().GetSparSegment(index);
}

CCPACSWingSparSegment& CCPACSWingCSStructure::GetSparSegment(const std::string& uid) const
{
    if (!spars) {
        throw CTiglError("Error: no spars existing in CCPACSWingCSStructure::GetSparSegment!");
    }
    return spars->GetSparSegments().GetSparSegment(uid);
}

bool CCPACSWingCSStructure::HasRibsDefinitions() const
{
    return (ribsDefinitions != NULL);
}

int CCPACSWingCSStructure::GetRibsDefinitionCount() const
{
    int ribsDefinitionCount = 0;

    if (ribsDefinitions) {
        ribsDefinitionCount = ribsDefinitions->GetRibsDefinitionCount();
    }

    return ribsDefinitionCount;
}

CCPACSWingRibsDefinition& CCPACSWingCSStructure::GetRibsDefinition(int index) const
{
    if (!ribsDefinitions) {
        throw CTiglError("Error: no ribsDefinitions existing in CCPACSWingCSStructure::GetRibsDefinition!");
    }
    return ribsDefinitions->GetRibsDefinition(index);
}

CCPACSWingRibsDefinition& CCPACSWingCSStructure::GetRibsDefinition(const std::string& uid) const
{
    if (!ribsDefinitions) {
        throw CTiglError("Error: no ribsDefinitions existing in CCPACSWingCSStructure::GetRibsDefinition!");
    }
    return ribsDefinitions->GetRibsDefinition(uid);
}

void CCPACSWingCSStructure::Invalidate()
{
    // forward invalidation
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
