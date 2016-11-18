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

#include "CCPACSWingSparWeb.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingSparWeb::CCPACSWingSparWeb()
{
    Cleanup();
}

CCPACSWingSparWeb::~CCPACSWingSparWeb()
{
    Cleanup();
}

void CCPACSWingSparWeb::Cleanup()
{
    relPos = 0;
    material.Cleanup();
}

void CCPACSWingSparWeb::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sparWebXPath)
{
    Cleanup();

    if (tixiGetDoubleElement(tixiHandle, (sparWebXPath + "/relPos").c_str(), &relPos) != SUCCESS) {
        LOG(ERROR) << "Missing relPos";
        throw CTiglError("Error: Missing relPos in CCPACSWingSparWeb::ReadCPACS!", TIGL_XML_ERROR);
    }

    if (tixiCheckElement(tixiHandle, (sparWebXPath + "/material").c_str()) != SUCCESS) {
        LOG(ERROR) << "Missing material";
        throw CTiglError("Error: Missing material in CCPACSWingSparWeb::ReadCPACS!", TIGL_XML_ERROR);
    }
    material.ReadCPACS(tixiHandle, sparWebXPath + "/material");
}

void CCPACSWingSparWeb::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& sparWebXPath) const
{
    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, sparWebXPath.c_str(), "relPos", relPos, NULL);

    TixiSaveExt::TixiSaveElement(tixiHandle, sparWebXPath.c_str(), "material");
    material.WriteCPACS(tixiHandle, sparWebXPath + "/material");
}

const CCPACSMaterial& CCPACSWingSparWeb::GetMaterial() const
{
    return material;
}

CCPACSMaterial& CCPACSWingSparWeb::GetMaterial()
{
    // forward call to const method
    return const_cast<CCPACSMaterial&>(static_cast<const CCPACSWingSparWeb&>(*this).GetMaterial());
}

double CCPACSWingSparWeb::GetRelPos() const
{
    return relPos;
}

} // end namespace tigl
