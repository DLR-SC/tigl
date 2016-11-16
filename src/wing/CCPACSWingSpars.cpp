/* 
* Copyright (C) 2016 Airbus Defence and Space
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

#include "CCPACSWingSpars.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingSpars::CCPACSWingSpars(CCPACSWingCSStructure& structure)
: structure(structure),
  sparSegments(*this),
  sparPositions(*this)
{
    Cleanup();
}

CCPACSWingSpars::~CCPACSWingSpars()
{
    Cleanup();
}

void CCPACSWingSpars::Invalidate()
{
    sparSegments.Invalidate();
}

void CCPACSWingSpars::Cleanup()
{
    sparPositions.Cleanup();
    sparSegments.Cleanup();
}

void CCPACSWingSpars::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sparsXPath)
{
    Cleanup();

    if (tixiCheckElement(tixiHandle, (sparsXPath + "/sparPositions").c_str()) != SUCCESS) {
        LOG(ERROR) << "Missing sparPositions";
        throw CTiglError("Error: Missing sparPositions in CCPACSWingSpars::ReadCPACS!", TIGL_XML_ERROR);
    }
    sparPositions.ReadCPACS(tixiHandle, sparsXPath + "/sparPositions");

    if (tixiCheckElement(tixiHandle, (sparsXPath + "/sparSegments").c_str()) != SUCCESS) {
        LOG(ERROR) << "Missing sparSegments";
        throw CTiglError("Error: Missing sparSegments in CCPACSWingSpars::ReadCPACS!", TIGL_XML_ERROR);
    }
    sparSegments.ReadCPACS(tixiHandle, sparsXPath + "/sparSegments");
}

void CCPACSWingSpars::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& sparsXPath) const
{
    TixiSaveExt::TixiSaveElement(tixiHandle, sparsXPath.c_str(), "sparPositions");
    sparPositions.WriteCPACS(tixiHandle, (sparsXPath + "/sparPositions").c_str());

    TixiSaveExt::TixiSaveElement(tixiHandle, sparsXPath.c_str(), "sparSegments");
    sparSegments.WriteCPACS(tixiHandle, (sparsXPath + "/sparSegments").c_str());
}

const CCPACSWingCSStructure& CCPACSWingSpars::GetStructure() const
{
    return structure;
}

CCPACSWingCSStructure& CCPACSWingSpars::GetStructure()
{
    return structure;
}

const CCPACSWingSparSegments& CCPACSWingSpars::GetSparSegments() const
{
    return sparSegments;
}

CCPACSWingSparSegments& CCPACSWingSpars::GetSparSegments()
{
    return sparSegments;
}

const CCPACSWingSparPositions& CCPACSWingSpars::GetSparPositions() const
{
    return sparPositions;
}

CCPACSWingSparPositions& CCPACSWingSpars::GetSparPositions()
{
    return sparPositions;
}

} // end namespace tigl
