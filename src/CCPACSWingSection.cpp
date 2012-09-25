/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
#include <iostream>

namespace tigl {

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

        char*       elementPath;
        std::string tempString;

        // Get subelement "name"
        char* ptrName = "";
        tempString    = sectionXPath + "/name";
        elementPath   = const_cast<char*>(tempString.c_str());
        tixiGetTextElement(tixiHandle, elementPath, &ptrName);
        name          = ptrName;

		// Get attribute "uID"
		char* ptrUID = "";
        tempString    = sectionXPath;
        elementPath   = const_cast<char*>(tempString.c_str());
        tixiGetTextAttribute(tixiHandle, elementPath, "uID", &ptrUID);
		uID          = ptrUID;

        // Get subelement "/transformation/translation"
        tempString  = sectionXPath + "/transformation/translation";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <translation/> in CCPACSWingSection::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "/transformation/scaling"
        tempString  = sectionXPath + "/transformation/scaling";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <scaling/> in CCPACSWingSection::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "/transformation/rotation"
        tempString  = sectionXPath + "/transformation/rotation";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
            if (tixiGetPoint(tixiHandle, elementPath, &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
                throw CTiglError("Error: XML error while reading <rotation/> in CCPACSWingSection::ReadCPACS", TIGL_XML_ERROR);
            }
        }

        // Get subelement "elements", which means the section elements
        elements.ReadCPACS(tixiHandle, sectionXPath);

        Update();
    }

    // Get profile count for this section
    int CCPACSWingSection::GetSectionElementCount(void) const
    {
        return elements.GetSectionElementCount();
    }

    // Get the UID of this WingSection
    std::string CCPACSWingSection::GetUID(void) const
    {
        return uID;
    }

    // Get the UID of this WingSection
    const char* CCPACSWingSection::GetUIDPtr(void) const
    {
        return uID.c_str();
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

} // end namespace tigl
