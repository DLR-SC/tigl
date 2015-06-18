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
* @brief  Implementation of CPACS fuselage section element handling routines.
*/

#include "CCPACSFuselageSectionElement.h"
#include "CTiglError.h"
#include "TixiSaveExt.h"
#include <iostream>

namespace tigl
{

// Constructor
CCPACSFuselageSectionElement::CCPACSFuselageSectionElement()
{
    Cleanup();
}

// Destructor
CCPACSFuselageSectionElement::~CCPACSFuselageSectionElement(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSFuselageSectionElement::Cleanup(void)
{
    name         = "";
    uid          = "";
    profileUID   = "";
    profileIndex = -1;
    transformation.SetIdentity();
    translation = CTiglPoint(0.0, 0.0, 0.0);
    scaling     = CTiglPoint(1.0, 1.0, 1.0);
    rotation    = CTiglPoint(0.0, 0.0, 0.0);
}

// Build transformation matrix for the section element
void CCPACSFuselageSectionElement::BuildMatrix(void)
{
    transformation.SetIdentity();

    // scale normalized coordinates relative to (0,0,0)
    transformation.AddScaling(scaling.x, scaling.y, scaling.z);

    // rotate section element around z
    transformation.AddRotationZ(rotation.z);
    // rotate section element by angle of incidence
    transformation.AddRotationY(rotation.y);
    // rotate section element according to fuselage profile roll
    transformation.AddRotationX(rotation.x);

    // Translate section element to final position relative to section plane
    transformation.AddTranslation(translation.x, translation.y, translation.z);
}

// Update internal section element data
void CCPACSFuselageSectionElement::Update(void)
{
    BuildMatrix();
}

// Read CPACS section elements
void CCPACSFuselageSectionElement::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = elementXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get subelement "description"
    char* ptrDescription = "";
    tempString    = elementXPath + "/description";
    elementPath   = const_cast<char*>(tempString.c_str());
    tixiGetTextElement(tixiHandle, elementPath, &ptrDescription);
    description   = ptrDescription;

    // Get subelement "profileUID"
    char* ptrUID  = NULL;
    tempString    = elementXPath + "/profileUID";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrUID) == SUCCESS) {
        profileUID = ptrUID;
    }

    // Get attribute "uID"
    char* ptrMyUID  = NULL;
    tempString    = elementXPath;
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, elementPath, "uID", &ptrMyUID) == SUCCESS) {
        uid = ptrMyUID;
    }

    // Get subelement "/transformation/translation"
    tempString  = elementXPath + "/transformation/translation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <translation/> in CCPACSFuselageSectionElement::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/scaling"
    tempString  = elementXPath + "/transformation/scaling";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath,&(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <scaling/> in CCPACSFuselageSectionElement::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/rotation"
    tempString  = elementXPath + "/transformation/rotation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <rotation/> in CCPACSFuselageSectionElement::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    Update();
}

// Write CPACS section elements
void CCPACSFuselageSectionElement::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    std::string elementPath;
    std::string subelementPath;

    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, elementXPath.c_str(), "uID", uid.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "name", name.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "description", description.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, elementXPath.c_str(), "profileUID", profileUID.c_str());
    
    TixiSaveExt::TixiSaveElement(tixiHandle, elementXPath.c_str(), "transformation");
    elementPath = elementXPath + "/transformation";
    
    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "scaling");
    subelementPath = elementPath + "/scaling";
    TixiSaveExt::TixiSavePoint(tixiHandle, subelementPath.c_str(), scaling.x, scaling.y, scaling.z, NULL);
    
    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "rotation");
    subelementPath = elementPath + "/rotation";
    TixiSaveExt::TixiSavePoint(tixiHandle, subelementPath.c_str(), rotation.x, rotation.y, rotation.z, NULL);
    
    TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "translation");
    subelementPath = elementPath + "/translation";
    TixiSaveExt::TixiSaveTextAttribute(tixiHandle, subelementPath.c_str(), "refType", "absLocal");
    TixiSaveExt::TixiSavePoint(tixiHandle, subelementPath.c_str(), translation.x, translation.y, translation.z, NULL);
}

// Returns the UID of the referenced fuselage profile
std::string CCPACSFuselageSectionElement::GetProfileIndex(void) const
{
    return profileUID;
}

// Returns the UID of this FuselageSectionElement
std::string CCPACSFuselageSectionElement::GetUID(void) const
{
    return uid;
}

// Returns the uid of the profile of this element
std::string CCPACSFuselageSectionElement::GetProfileUID(void) const
{
    return profileUID;
}

// Gets the section element transformation
CTiglTransformation CCPACSFuselageSectionElement::GetSectionElementTransformation(void) const
{
    return transformation;
}

} // end namespace tigl
