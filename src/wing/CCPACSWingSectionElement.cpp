/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#include "CCPACSWingSection.h"
#include "CCPACSWing.h"

namespace tigl
{
CCPACSWingSectionElement::CCPACSWingSectionElement(CCPACSWingSectionElements* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingElement(parent, uidMgr) {}

void CCPACSWingSectionElement::SetAirfoilUID(const std::string& value) {
    generated::CPACSWingElement::SetAirfoilUID(value);
    // invalidate wing as we affect wing segments and component segments
    m_parent->GetParent()->GetParent()->GetParent<CCPACSWing>()->Invalidate();
}

// Gets the section element transformation
CTiglTransformation CCPACSWingSectionElement::GetSectionElementTransformation() const
{
    return m_transformation.getTransformationMatrix();
}

CTiglPoint CCPACSWingSectionElement::GetTranslation() const
{
    return m_transformation.getTranslationVector();
}

CTiglPoint CCPACSWingSectionElement::GetRotation() const
{
    return m_transformation.getRotation();
}

CTiglPoint CCPACSWingSectionElement::GetScaling() const
{
    return m_transformation.getScaling();
}

void CCPACSWingSectionElement::SetTranslation(const CTiglPoint &trans)
{
    m_transformation.setTranslation(trans, ABS_LOCAL);
}

void CCPACSWingSectionElement::SetRotation(const CTiglPoint &rot)
{
    m_transformation.setRotation(rot);
}

void CCPACSWingSectionElement::SetScaling(const CTiglPoint &scaling)
{
    m_transformation.setScaling(scaling);
}

} // end namespace tigl
