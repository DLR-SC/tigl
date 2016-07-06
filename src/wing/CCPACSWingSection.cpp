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
    m_name = "";
    m_uID  = "";
    m_description = "";
    transformationMatrix.SetIdentity();
}

// Build transformation matrix for the section
void CCPACSWingSection::BuildMatrix(void)
{
    const CTiglPoint& scaling = GetScaling();
    const CTiglPoint& rotation = GetRotation();
    const CTiglPoint& translation = GetTranslation();

    transformationMatrix.SetIdentity();

    // scale normalized coordinates relative to (0,0,0)
    transformationMatrix.AddScaling(scaling.x, scaling.y, scaling.z);

    // rotate wing profile elements around their local reference points
    transformationMatrix.AddRotationZ(rotation.z);
    // rotate section by angle of incidence
    transformationMatrix.AddRotationY(rotation.y);
    // rotate section according to wing profile roll
    transformationMatrix.AddRotationX(rotation.x);

    // move local reference point to (0,0,0)
    transformationMatrix.AddTranslation(translation.x, translation.y, translation.z);
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
    generated::CPACSWingSection::ReadCPACS(tixiHandle, sectionXPath);

    Update();
}

// Write CPACS section elements
void CCPACSWingSection::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& sectionXPath)
{
    generated::CPACSWingSection::WriteCPACS(tixiHandle, sectionXPath);
}

// Get profile count for this section
int CCPACSWingSection::GetSectionElementCount(void) const
{
    return static_cast<int>(m_elements.GetElement().size());
}

// Get element for a given index
const CCPACSWingSectionElement& CCPACSWingSection::GetSectionElement(int index) const
{
    return *m_elements.GetElement()[index];
}

// Gets the section transformation
CTiglTransformation CCPACSWingSection::GetSectionTransformation(void) const
{
    return transformationMatrix;
}

// Gets the section translation
CTiglPoint CCPACSWingSection::GetTranslation() const
{
    return m_transformation.GetTranslation();
}

// Gets the section rotation
CTiglPoint CCPACSWingSection::GetRotation() const
{
	return m_transformation.GetRotation();
}

// Gets the section scaling
CTiglPoint CCPACSWingSection::GetScaling() const
{
	return m_transformation.GetScaling();
}

// Setter for translation
void CCPACSWingSection::SetTranslation(const CTiglPoint& trans)
{
    m_transformation.SetTranslation(trans);
    Update();
}

// Setter for rotation
void CCPACSWingSection::SetRotation(const CTiglPoint& rot)
{
    m_transformation.SetRotation(rot);
    Update();
}

// Setter for scaling
void CCPACSWingSection::SetScaling(const CTiglPoint& scale)
{
    m_transformation.SetScaling(scale);
    Update();
}

} // end namespace tigl
