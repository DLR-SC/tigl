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

#include "CCPACSWingRibCrossSection.h"

#include "CCPACSCapType.h"
#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


namespace tigl
{

CCPACSWingRibCrossSection::CCPACSWingRibCrossSection(CCPACSWingRibsDefinition& parent)
: parent(parent),
  lowerCap(NULL),
  upperCap(NULL)
{
    Cleanup();
}

CCPACSWingRibCrossSection::~CCPACSWingRibCrossSection()
{
    Cleanup();
}

void CCPACSWingRibCrossSection::Cleanup()
{
    material.Cleanup();
    xRotation = 90.;
    if (lowerCap) {
        delete lowerCap;
        lowerCap = NULL;
    }
    if (upperCap) {
        delete upperCap;
        upperCap = NULL;
    }
}

void CCPACSWingRibCrossSection::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& ribCrossSectionXPath)
{
    Cleanup();

    // Get subelement "material"
    std::string path = ribCrossSectionXPath + "/material";
    if (tixiCheckElement(tixiHandle, path.c_str()) != SUCCESS) {
        LOG(ERROR) << "Missing material";
        throw CTiglError("Error: Missing material in CCPACSWingRibCrossSection::ReadCPACS!", TIGL_XML_ERROR);
    }
    material.ReadCPACS(tixiHandle, path);

    // Get subelement "ribRotation/x"
    path = ribCrossSectionXPath + "/ribRotation";
    if (tixiCheckElement(tixiHandle, path.c_str()) == SUCCESS) {
        path += "/x";
        if (tixiGetDoubleElement(tixiHandle, path.c_str(), &xRotation) != SUCCESS) {
            LOG(ERROR) << "Missing x rotation";
            throw CTiglError("Error: Missing x rotation in CCPACSWingRibCrossSection::ReadCPACS!", TIGL_XML_ERROR);
        }
    }

    // Get subelement lowerCap
    path = ribCrossSectionXPath + "/lowerCap";
    if (tixiCheckElement(tixiHandle, path.c_str()) == SUCCESS) {
        lowerCap = new CCPACSCapType();
        lowerCap->ReadCPACS(tixiHandle, path);
    }

    // Get subelement upperCap
    path = ribCrossSectionXPath + "/upperCap";
    if (tixiCheckElement(tixiHandle, path.c_str()) == SUCCESS) {
        upperCap = new CCPACSCapType();
        upperCap->ReadCPACS(tixiHandle, path);
    }

    // TODO: add support for ribCell
}

void CCPACSWingRibCrossSection::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& ribCrossSectionXPath) const
{
    TixiSaveExt::TixiSaveElement(tixiHandle, ribCrossSectionXPath.c_str(), "material");
    material.WriteCPACS(tixiHandle, (ribCrossSectionXPath + "/material").c_str());

    // NOTE: xRotation is always written, even if it was only default initialized
    TixiSaveExt::TixiSaveElement(tixiHandle, ribCrossSectionXPath.c_str(), "ribRotation");
    TixiSaveExt::TixiSaveDoubleElement(tixiHandle, (ribCrossSectionXPath + "/ribRotation").c_str(), "x", xRotation, NULL);

    if (upperCap != NULL) {
        TixiSaveExt::TixiSaveElement(tixiHandle, ribCrossSectionXPath.c_str(), "upperCap");
        upperCap->WriteCPACS(tixiHandle, (ribCrossSectionXPath + "/upperCap").c_str());
    }

    if (lowerCap != NULL) {
        TixiSaveExt::TixiSaveElement(tixiHandle, ribCrossSectionXPath.c_str(), "lowerCap");
        lowerCap->WriteCPACS(tixiHandle, (ribCrossSectionXPath + "/lowerCap").c_str());
    }

    // TODO: add support for ribCell
}

const CCPACSMaterial& CCPACSWingRibCrossSection::GetMaterial() const
{
    return material;
}

CCPACSMaterial& CCPACSWingRibCrossSection::GetMaterial()
{
    return material;
}

double CCPACSWingRibCrossSection::GetXRotation() const
{
    return xRotation;
}

void CCPACSWingRibCrossSection::SetXRotation(double rotation)
{
    xRotation = rotation;
    // invalidate whole component segment structure, since rib could be referenced anywher
    parent.GetStructure().Invalidate();
}

bool CCPACSWingRibCrossSection::HasUpperCap() const
{
    return (upperCap != NULL);
}

const CCPACSCapType& CCPACSWingRibCrossSection::GetUpperCap() const
{
    if (!upperCap) {
        throw CTiglError("UpperCap is not defined in CCPACSWingRibCrossSection::GetUpperCap(): Please check using HasUpperCap() method first!");
    }
    return *upperCap;
}

CCPACSCapType& CCPACSWingRibCrossSection::GetUpperCap()
{
    // forward call to const method
    return const_cast<CCPACSCapType&>(static_cast<const CCPACSWingRibCrossSection&>(*this).GetUpperCap());
}

bool CCPACSWingRibCrossSection::HasLowerCap() const
{
    return (lowerCap != NULL);
}

const CCPACSCapType& CCPACSWingRibCrossSection::GetLowerCap() const
{
    if (!lowerCap) {
        throw CTiglError("LpperCap is not defined in CCPACSWingRibCrossSection::GetLowerCap(): Please check using HasLowerCap() method first!");
    }
    return *lowerCap;
}

CCPACSCapType& CCPACSWingRibCrossSection::GetLowerCap()
{
    // forward call to const method
    return const_cast<CCPACSCapType&>(static_cast<const CCPACSWingRibCrossSection&>(*this).GetLowerCap());
}

} // end namespace tigl
