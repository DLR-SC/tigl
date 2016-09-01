/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file
* @brief  Implementation of CPACS wing section handling routines.
*/

#include "CCPACSWingSection.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>

namespace tigl
{

// Constructor
CCPACSWingSection::CCPACSWingSection()
{
    Cleanup();
}

// Destructor
CCPACSWingSection::~CCPACSWingSection(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingSection::Cleanup(void)
{
    name = "";
    uID  = "";
    description = "";
    transformation.SetIdentity();
    translation = CTiglPoint(0.0, 0.0, 0.0);
    scaling     = CTiglPoint(1.0, 1.0, 1.0);
    rotation    = CTiglPoint(0.0, 0.0, 0.0);
}

// Build transformation matrix for the section
void CCPACSWingSection::BuildMatrix(void)
{
    transformation.SetIdentity();

    // scale normalized coordinates relative to (0,0,0)
    transformation.AddScaling(scaling.x, scaling.y, scaling.z);

    // rotate wing profile elements around their local reference points
    transformation.AddRotationZ(rotation.z);
    // rotate section by angle of incidence
    transformation.AddRotationY(rotation.y);
    // rotate section according to wing profile roll
    transformation.AddRotationX(rotation.x);

    // move local reference point to (0,0,0)
    transformation.AddTranslation(translation.x, translation.y, translation.z);
}

// Update internal section data
void CCPACSWingSection::Update(void)
{
    BuildMatrix();
}

// Read CPACS section elements
void CCPACSWingSection::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
{
    Cleanup();

    // Get subelement "name"
    char* ptrName = NULL;
    if (tixiGetTextElement(tixiHandle, (sectionXPath + "/name").c_str(), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get subelement "description"
    char* ptrDescription = NULL;
    if (tixiGetTextElement(tixiHandle, (sectionXPath + "/description").c_str(), &ptrDescription) == SUCCESS) {
        description = ptrDescription;
    }

    // Get attribute "uID"
    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, sectionXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        uID = ptrUID;
    }

    // Get subelement "/transformation/translation"
    std::string elementPath = sectionXPath + "/transformation/translation";
    if (tixiCheckElement(tixiHandle, elementPath.c_str()) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath.c_str(), &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <translation/> in CCPACSWingSection::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/scaling"
    elementPath = sectionXPath + "/transformation/scaling";
    if (tixiCheckElement(tixiHandle, elementPath.c_str()) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath.c_str(), &(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <scaling/> in CCPACSWingSection::ReadCPACS", TIGL_XML_ERROR);
        }
    }
    // Fix scaling of y-component
    if (scaling.x == scaling.z && scaling.x != scaling.y) {
        scaling.y = scaling.x;
    }

    // Get subelement "/transformation/rotation"
    elementPath = sectionXPath + "/transformation/rotation";
    if (tixiCheckElement(tixiHandle, elementPath.c_str()) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath.c_str(), &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <rotation/> in CCPACSWingSection::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "elements", which means the section elements
    elements.ReadCPACS(tixiHandle, sectionXPath);

    Update();
}

// Write CPACS section elements
void CCPACSWingSection::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
{
    TixiSaveExt::TixiSaveTextElement(tixiHandle, sectionXPath.c_str(), "name", GetName().c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, sectionXPath.c_str(), "description", description.c_str());
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, sectionXPath.c_str(), "uID", GetUID().c_str());
    TixiSaveExt::TixiSaveElement(tixiHandle, sectionXPath.c_str(), "transformation");

    const std::string elementPath = sectionXPath + "/transformation";

    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "scaling");
    TixiSaveExt::TixiSavePoint(tixiHandle, (elementPath + "/scaling").c_str(), GetScaling().x, GetScaling().y, GetScaling().z, NULL);

    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "rotation");
    TixiSaveExt::TixiSavePoint(tixiHandle, (elementPath + "/rotation").c_str(), GetRotation().x, GetRotation().y, GetRotation().z, NULL);

    const std::string subelementPath = elementPath + "/translation";
    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "translation");
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, subelementPath.c_str(), "refType", "absLocal");
    TixiSaveExt::TixiSavePoint(tixiHandle, subelementPath.c_str(), GetTranslation().x, GetTranslation().y, GetTranslation().z, NULL);

    elements.WriteCPACS(tixiHandle, sectionXPath);
}

// Get profile count for this section
int CCPACSWingSection::GetSectionElementCount(void) const
{
    return elements.GetSectionElementCount();
}

// Get the UID of this WingSection
const std::string& CCPACSWingSection::GetUID(void) const
{
    return uID;
}

// Get the name of this WingSection
const std::string& CCPACSWingSection::GetName() const
{
    return name;
}

// Getter for the member description
const std::string& CCPACSWingSection::GetDescription(void) const
{
    return description;
}

// Get element for a given index
CCPACSWingSectionElement& CCPACSWingSection::GetSectionElement(int index) const
{
    return elements.GetSectionElement(index);
}

// Gets the section transformation
CTiglTransformation CCPACSWingSection::GetSectionTransformation(void) const
{
    return transformation;
}

// Gets the section translation
const CTiglPoint& CCPACSWingSection::GetTranslation() const
{
    return translation;
}

// Gets the section rotation
const CTiglPoint& CCPACSWingSection::GetRotation() const
{
    return rotation;
}

// Gets the section scaling
const CTiglPoint& CCPACSWingSection::GetScaling() const
{
    return scaling;
}

// Setter for translation
void CCPACSWingSection::SetTranslation(const CTiglPoint& trans)
{
    translation = trans;
    Update();
}

// Setter for rotation
void CCPACSWingSection::SetRotation(const CTiglPoint& rot)
{
    rotation = rot;
    Update();
}

// Setter for scaling
void CCPACSWingSection::SetScaling(const CTiglPoint& scale)
{
    scaling = scale;
    Update();
}

} // end namespace tigl
