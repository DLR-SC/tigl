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
// Returns the UID of the referenced wing profile
std::string CCPACSWingSectionElement::GetProfileIndex() const
{
    return m_airfoilUID;
}

// Returns the uid of the profile of this element
std::string CCPACSWingSectionElement::GetProfileUID() const
{
    return m_airfoilUID;
}


// Gets the section element transformation
CTiglTransformation CCPACSWingSectionElement::GetSectionElementTransformation(void) const
{
    return transformation.getTransformationMatrix();
}

const CTiglPoint &CCPACSWingSectionElement::GetTranslation() const
{
    return transformation.getTranslationVector();
}

const CTiglPoint &CCPACSWingSectionElement::GetRotation() const
{
    return transformation.getRotation();
}

const CTiglPoint &CCPACSWingSectionElement::GetScaling() const
{
    return transformation.getScaling();
}

void CCPACSWingSectionElement::SetTranslation(const CTiglPoint &trans)
{
    transformation.setTranslation(trans, ABS_LOCAL);
    transformation.updateMatrix();
}

void CCPACSWingSectionElement::SetRotation(const CTiglPoint &rot)
{
    transformation.setRotation(rot);
    transformation.updateMatrix();
}

void CCPACSWingSectionElement::SetScaling(const CTiglPoint &scaling)
{
    transformation.setScaling(scaling);
    transformation.updateMatrix();
}

} // end namespace tigl
