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

#include "CCPACSCapType.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSCapType::CCPACSCapType()
{
    Cleanup();
}

CCPACSCapType::~CCPACSCapType()
{
    Cleanup();
}

void CCPACSCapType::Cleanup()
{
    area = 0;
    material.Cleanup();
}

void CCPACSCapType::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sparCapXPath)
{
    Cleanup();

    if (tixiGetDoubleElement(tixiHandle, (sparCapXPath + "/area").c_str(), &area) != SUCCESS) {
        LOG(ERROR) << "Missing area";
        throw CTiglError("Error: Missing area in CCPACSCapType::ReadCPACS!", TIGL_XML_ERROR);
    }

    if (tixiCheckElement(tixiHandle, (sparCapXPath + "/material").c_str()) != SUCCESS) {
        LOG(ERROR) << "Missing material";
        throw CTiglError("Error: Missing material in CCPACSCapType::ReadCPACS!", TIGL_XML_ERROR);
    }
    material.ReadCPACS(tixiHandle, sparCapXPath + "/material");
}

void CCPACSCapType::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& sparCapXPath) const
{
    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, sparCapXPath.c_str(), "area", area, NULL);

    TixiSaveExt::TixiSaveElement(tixiHandle, sparCapXPath.c_str(), "material");
    material.WriteCPACS(tixiHandle, sparCapXPath + "/material");
}

double CCPACSCapType::GetCapArea() const
{
    return area;
}

void CCPACSCapType::SetCapArea(double value)
{
    area = value;
}

const CCPACSMaterial& CCPACSCapType::GetMaterial() const
{
    return material;
}

CCPACSMaterial& CCPACSCapType::GetMaterial()
{
    return material;
}

} // end namespace tigl
