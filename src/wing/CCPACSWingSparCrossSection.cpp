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

#include "CCPACSWingSparCrossSection.h"

#include "CCPACSCapType.h"
#include "CCPACSWingCSStructure.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingSparCrossSection::CCPACSWingSparCrossSection(CCPACSWingCSStructure& structure)
: structure(structure),
  lowerCap(NULL),
  upperCap(NULL),
  web2(NULL)
{
    Cleanup();
}

CCPACSWingSparCrossSection::~CCPACSWingSparCrossSection(void)
{
    Cleanup();
}

void CCPACSWingSparCrossSection::Cleanup(void)
{
    rotation = 0;
    if (lowerCap) {
        delete lowerCap;
        lowerCap = NULL;
    }
    if (upperCap) {
        delete upperCap;
        upperCap = NULL;
    }
    web1.Cleanup();
    if (web2) {
        delete web2;
        web2 = NULL;
    }
}

void CCPACSWingSparCrossSection::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& crossSectionXPath)
{
    Cleanup();

    // Get subelement "rotation"
    ReturnCode tixiRet = tixiGetDoubleElement(tixiHandle, (crossSectionXPath + "/rotation").c_str(), &rotation);
    if (tixiRet != SUCCESS) {
        LOG(ERROR) << "missing rotation";
        throw CTiglError("Error: missing rotation in CCPACSWingSparCrossSection::ReadCPACS!", TIGL_XML_ERROR);
    }

    /* Get subelement lowerCap */
    int count;
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, crossSectionXPath.c_str(), "lowerCap", &count);
    if (tixiRet == SUCCESS && count == 1) {
        lowerCap = new CCPACSCapType();
        lowerCap->ReadCPACS(tixiHandle, crossSectionXPath + "/lowerCap");
    }

    /* Get subelement upperCap */
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, crossSectionXPath.c_str(), "upperCap", &count);
    if (tixiRet == SUCCESS && count == 1) {
        upperCap = new CCPACSCapType();
        upperCap->ReadCPACS(tixiHandle, crossSectionXPath + "/upperCap");
    }

    /* Get subelement web1 */
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, crossSectionXPath.c_str(), "web1", &count);
    if (tixiRet != SUCCESS || count < 1) {
        LOG(ERROR) << "missing web1";
        throw CTiglError("Error: Missing web1 in CCPACSWingSparCrossSection::ReadCPACS!", TIGL_XML_ERROR);
    }

    web1.ReadCPACS(tixiHandle, crossSectionXPath + "/web1");

    /* Get subelement web2 */
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, crossSectionXPath.c_str(), "web2", &count);
    if (tixiRet == SUCCESS && count == 1) {
        web2 = new CCPACSWingSparWeb();
        web2->ReadCPACS(tixiHandle, crossSectionXPath + "/web2");
    }

    //TODO: sparCells not implemented yet
}

void CCPACSWingSparCrossSection::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& crossSectionXPath) const
{
    if (upperCap != NULL) {
        const std::string elementPath = crossSectionXPath + "/upperCap";
        TixiSaveExt::TixiSaveElement(tixiHandle, crossSectionXPath.c_str(), "upperCap");
        upperCap->WriteCPACS(tixiHandle, elementPath);
    }

    if (lowerCap != NULL) {
        const std::string elementPath = crossSectionXPath + "/lowerCap";
        TixiSaveExt::TixiSaveElement(tixiHandle, crossSectionXPath.c_str(), "lowerCap");
        lowerCap->WriteCPACS(tixiHandle, elementPath);
    }

    const std::string elementPath = crossSectionXPath + "/web1";
    TixiSaveExt::TixiSaveElement(tixiHandle, crossSectionXPath.c_str(), "web1");
    web1.WriteCPACS(tixiHandle, elementPath);
        
    if (web2 != NULL) {
        const std::string elementPath = crossSectionXPath + "/web2";
        TixiSaveExt::TixiSaveElement(tixiHandle, crossSectionXPath.c_str(), "web2");
        web2->WriteCPACS(tixiHandle, elementPath);
    }

    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, crossSectionXPath.c_str(), "rotation", rotation, NULL);
}

double CCPACSWingSparCrossSection::GetRotation(void) const
{
    return rotation;
}

bool CCPACSWingSparCrossSection::HasUpperCap() const
{
    return (upperCap != NULL);
}

bool CCPACSWingSparCrossSection::HasLowerCap() const
{
    return (lowerCap != NULL);
}

bool CCPACSWingSparCrossSection::HasWeb2() const
{
    return (web2 != NULL);
}

const CCPACSCapType& CCPACSWingSparCrossSection::GetLowerCap() const
{
    if (!lowerCap) {
        throw CTiglError("LowerCap requested but not defined in CCPACSWingSparCrossSection::GetLowerCap()!");
    }
    return *lowerCap;
}

CCPACSCapType& CCPACSWingSparCrossSection::GetLowerCap()
{
    // forward call to const method
    return const_cast<CCPACSCapType&>(static_cast<const CCPACSWingSparCrossSection&>(*this).GetLowerCap());
}

const CCPACSCapType& CCPACSWingSparCrossSection::GetUpperCap() const
{
    if (!upperCap) {
        throw CTiglError("UpperCap requested but not defined in CCPACSWingSparCrossSection::GetUpperCap()!");
    }
    return *upperCap;
}

CCPACSCapType& CCPACSWingSparCrossSection::GetUpperCap()
{
    // forward call to const method
    return const_cast<CCPACSCapType&>(static_cast<const CCPACSWingSparCrossSection&>(*this).GetUpperCap());
}

const CCPACSWingSparWeb& CCPACSWingSparCrossSection::GetWeb1() const
{
    return web1;
}

CCPACSWingSparWeb& CCPACSWingSparCrossSection::GetWeb1()
{
    // forward call to const method
    return const_cast<CCPACSWingSparWeb&>(static_cast<const CCPACSWingSparCrossSection&>(*this).GetWeb1());
}

const CCPACSWingSparWeb& CCPACSWingSparCrossSection::GetWeb2() const
{
    if (!web2) {
        throw CTiglError("Web2 requested but not defined in CCPACSWingSparCrossSection::GetWeb2()!");
    }
    return *web2;
}

CCPACSWingSparWeb& CCPACSWingSparCrossSection::GetWeb2()
{
    // forward call to const method
    return const_cast<CCPACSWingSparWeb&>(static_cast<const CCPACSWingSparCrossSection&>(*this).GetWeb2());
}

} // end namespace tigl
