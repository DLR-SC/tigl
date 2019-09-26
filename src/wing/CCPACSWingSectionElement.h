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

#ifndef CCPACSWINGSECTIONELEMENT_H
#define CCPACSWINGSECTIONELEMENT_H

#include "generated/CPACSWingElement.h"
#include "CCPACSTransformation.h"
#include "CTiglPoint.h"
#include "CTiglWingSectionElement.h"

namespace tigl
{

class CCPACSWingSectionElement : public generated::CPACSWingElement
{
public:
    TIGL_EXPORT CCPACSWingSectionElement(CCPACSWingSectionElements* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void SetAirfoilUID(const std::string& value) override;

    // Gets the section element transformation
    TIGL_EXPORT CTiglTransformation GetSectionElementTransformation() const;

    // Getter for translation
    TIGL_EXPORT CTiglPoint GetTranslation() const;

    // Getter for rotation
    TIGL_EXPORT CTiglPoint GetRotation() const;

    // Getter for scaling
    TIGL_EXPORT CTiglPoint GetScaling() const;

    // Setter for translation
    TIGL_EXPORT void SetTranslation(const CTiglPoint& trans);

    // Setter for rotation
    TIGL_EXPORT void SetRotation(const CTiglPoint& rot);

    // Setter for scaling
    TIGL_EXPORT void SetScaling(const CTiglPoint& scaling);

    // Getter for the CTiglSectionElement
    TIGL_EXPORT CTiglWingSectionElement* GetCTiglSectionElement() { return &(cTiglElement); }
    TIGL_EXPORT CTiglWingSectionElement const* GetCTiglSectionElement() const { return &(cTiglElement); }

private:

    CTiglWingSectionElement cTiglElement;

};

} // end namespace tigl

#endif // CCPACSWINGSECTIONELEMENT_H
