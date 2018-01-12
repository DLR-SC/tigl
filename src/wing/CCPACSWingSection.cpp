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

namespace tigl
{
CCPACSWingSection::CCPACSWingSection(CCPACSWingSections* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSection(parent, uidMgr) {}

// Get profile count for this section
int CCPACSWingSection::GetSectionElementCount() const
{
    return static_cast<int>(m_elements.GetElements().size());
}

// Get element for a given index
CCPACSWingSectionElement& CCPACSWingSection::GetSectionElement(int index)
{
    return m_elements.GetSectionElement(index);
}
const CCPACSWingSectionElement& CCPACSWingSection::GetSectionElement(int index) const
{
    return m_elements.GetSectionElement(index);
}

// Gets the section transformation
CTiglTransformation CCPACSWingSection::GetSectionTransformation() const
{
    return m_transformation.getTransformationMatrix();
}

// Gets the section translation
CTiglPoint CCPACSWingSection::GetTranslation() const
{
    return m_transformation.getTranslationVector();
}

// Gets the section rotation
CTiglPoint CCPACSWingSection::GetRotation() const
{
    return m_transformation.getRotation();
}

// Gets the section scaling
CTiglPoint CCPACSWingSection::GetScaling() const
{
    return m_transformation.getScaling();
}

// Setter for translation
void CCPACSWingSection::SetTranslation(const CTiglPoint& trans)
{
    m_transformation.setTranslation(trans, ABS_LOCAL);

}

// Setter for rotation
void CCPACSWingSection::SetRotation(const CTiglPoint& rot)
{
    m_transformation.setRotation(rot);
}

// Setter for scaling
void CCPACSWingSection::SetScaling(const CTiglPoint& scale)
{
    m_transformation.setScaling(scale);
}

} // end namespace tigl
