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

#ifndef CCPACSFUSELAGESECTIONELEMENT_H
#define CCPACSFUSELAGESECTIONELEMENT_H

#include "generated/CPACSFuselageElement.h"
#include "CCPACSTransformation.h"
#include "CTiglPoint.h"

namespace tigl
{

class CCPACSFuselageSectionElement : public generated::CPACSFuselageElement
{
public:
    // Constructor
    TIGL_EXPORT CCPACSFuselageSectionElement(CTiglUIDManager* uidMgr);

    // Read CPACS section element
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath);

    // Returns the UID of the referenced wing profile
    TIGL_EXPORT std::string GetProfileIndex() const;

    // Gets the section element transformation
    TIGL_EXPORT CTiglTransformation GetSectionElementTransformation() const;

    TIGL_EXPORT CTiglPoint GetTranslation() const;

    TIGL_EXPORT CTiglPoint GetRotation() const;

    TIGL_EXPORT CTiglPoint GetScaling() const;

    TIGL_EXPORT void SetTranslation(const CTiglPoint& translation);

    TIGL_EXPORT void SetRotation(const CTiglPoint& rotation);

    TIGL_EXPORT void SetScaling(const CTiglPoint& scaling);
};

} // end namespace tigl

#endif // CCPACSFUSELAGESECTIONELEMENT_H
