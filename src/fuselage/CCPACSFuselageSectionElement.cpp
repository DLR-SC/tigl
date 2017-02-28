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
#include <iostream>

namespace tigl
{

// Constructor
CCPACSFuselageSectionElement::CCPACSFuselageSectionElement()
{
}

// Read CPACS section elements
void CCPACSFuselageSectionElement::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath)
{
    generated::CPACSFuselageElement::ReadCPACS(tixiHandle, elementXPath);
}

// Returns the UID of the referenced fuselage profile
std::string CCPACSFuselageSectionElement::GetProfileIndex() const
{
    return m_profileUID;
}

// Gets the section element transformation
CTiglTransformation CCPACSFuselageSectionElement::GetSectionElementTransformation() const
{
    return m_transformation.getTransformationMatrix();
}

CTiglPoint CCPACSFuselageSectionElement::GetTranslation() const
{
    return m_transformation.getTranslationVector();
}

CTiglPoint CCPACSFuselageSectionElement::GetRotation() const
{
    return m_transformation.getRotation();
}

CTiglPoint CCPACSFuselageSectionElement::GetScaling() const
{
    return m_transformation.getScaling();
}

void CCPACSFuselageSectionElement::SetTranslation(const CTiglPoint &translation)
{
    m_transformation.setTranslation(translation, ABS_LOCAL);
    m_transformation.updateMatrix();
}

void CCPACSFuselageSectionElement::SetRotation(const CTiglPoint &rotation)
{
    m_transformation.setRotation(rotation);
    m_transformation.updateMatrix();
}

void CCPACSFuselageSectionElement::SetScaling(const CTiglPoint &scaling)
{
    m_transformation.setScaling(scaling);
    m_transformation.updateMatrix();
}

} // end namespace tigl
