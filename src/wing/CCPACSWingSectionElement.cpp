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
* @brief  Implementation of CPACS wing section element handling routines.
*/

#include "CCPACSWingSectionElement.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>

namespace tigl
{

// Constructor
CCPACSWingSectionElement::CCPACSWingSectionElement()
{
    Cleanup();
}

// Destructor
CCPACSWingSectionElement::~CCPACSWingSectionElement(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingSectionElement::Cleanup(void)
{
    name         = "";
    profileUID     = "";
    uID          = "";
    transformation.SetIdentity();
    translation = CTiglPoint(0.0, 0.0, 0.0);
    scaling     = CTiglPoint(1.0, 1.0, 1.0);
    rotation    = CTiglPoint(0.0, 0.0, 0.0);
}

// Build transformation matrix for the wing section element
void CCPACSWingSectionElement::BuildMatrix(void)
{
    transformation.SetIdentity();

    // scale normalized coordinates relative to (0,0,0)
    transformation.AddScaling(scaling.x, scaling.y, scaling.z);

    // rotate section element around z
    transformation.AddRotationZ(rotation.z);
    // rotate section element by angle of incidence
    transformation.AddRotationY(rotation.y);
    // rotate section element according to wing profile roll
    transformation.AddRotationX(rotation.x);

    // Translate section element to final position relative to section plane
    transformation.AddTranslation(translation.x, translation.y, translation.z);
}

// Update internal section element data
void CCPACSWingSectionElement::Update(void)
{
    BuildMatrix();
}

// Read CPACS wing section elements
void CCPACSWingSectionElement::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = elementXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name          = ptrName;
    }

    // Get subelement "description"
    char* ptrdescription = "";
    tempString    = elementXPath + "/description";
    tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrdescription);
    description          = ptrdescription;

    // Get subelement "profileUID"
    char* ptrUID  = NULL;
    tempString    = elementXPath + "/airfoilUID";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrUID) == SUCCESS) {
        profileUID      = ptrUID;
    }

    // Get attribute "uID"
    char* ptrMyUID  = NULL;
    tempString    = elementXPath;
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, elementPath, "uID", &ptrMyUID) == SUCCESS) {
        uID      = ptrMyUID;
    }

    // Get subelement "/transformation/translation"
    tempString  = elementXPath + "/transformation/translation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <translation/> in CCPACSWingSectionElement::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/scaling"
    tempString  = elementXPath + "/transformation/scaling";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <scaling/> in CCPACSWingSectionElement::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/rotation"
    tempString  = elementXPath + "/transformation/rotation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <rotation/> in CCPACSWingSectionElement::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    Update();
}

// Write CPACS wing section elements
void CCPACSWingSectionElement::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    std::string path;
    std::string subPath;

    // Set attribute "uID"
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, elementXPath.c_str(), "uID", uID.c_str());

    // Set subelement "name"
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "name", name.c_str());

    // Set subelement "name"
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "description", description.c_str());

    // Set subelement "profileUID"
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "airfoilUID", profileUID.c_str());

    // Set the subelement "transformation"
    TixiSaveExt::TixiSaveElement(tixiHandle, elementXPath.c_str(), "transformation");

    path = elementXPath + "/transformation";
    // Set subelement "/transformation/scaling"
    subPath = path + "/scaling";
    TixiSaveExt::TixiSaveElement(tixiHandle, path.c_str(), "scaling");
    TixiSaveExt::TixiSavePoint(tixiHandle, subPath.c_str(), scaling.x, scaling.y, scaling.z, NULL);

    // Set subelement "/transformation/rotation"
    subPath = path + "/rotation";
    TixiSaveExt::TixiSaveElement(tixiHandle, path.c_str(), "rotation");
    TixiSaveExt::TixiSavePoint(tixiHandle, subPath.c_str(), rotation.x, rotation.y, rotation.z, NULL);

    // Set subelement "/transformation/translation"
    subPath = path + "/translation";
    TixiSaveExt::TixiSaveElement(tixiHandle, path.c_str(), "translation");
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, subPath.c_str(), "refType", "absLocal");
    TixiSaveExt::TixiSavePoint(tixiHandle, subPath.c_str(), translation.x, translation.y, translation.z, NULL);
}

// Returns the UID of the referenced wing profile
std::string CCPACSWingSectionElement::GetProfileIndex(void) const
{
    return profileUID;
}

// Returns the UID of this WingSectionElement
std::string CCPACSWingSectionElement::GetUID(void) const
{
    return uID;
}

// Returns the uid of the profile of this element
std::string CCPACSWingSectionElement::GetProfileUID(void) const
{
    return profileUID;
}

// Gets the section element transformation
CTiglTransformation CCPACSWingSectionElement::GetSectionElementTransformation(void) const
{
    return transformation;
}

} // end namespace tigl
