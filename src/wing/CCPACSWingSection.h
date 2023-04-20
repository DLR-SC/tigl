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
* @brief  Implementation of CPACS wing section handling routines.
*/

#ifndef CCPACSWINGSECTION_H
#define CCPACSWINGSECTION_H

#include "generated/CPACSWingSection.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSWingSectionElements.h"
#include "CCPACSTransformation.h"
#include <string>

namespace tigl
{

class CCPACSWingSection : public generated::CPACSWingSection
{
public:
    TIGL_EXPORT CCPACSWingSection(CCPACSWingSections* parent, CTiglUIDManager* uidMgr);

    /**
    * @brief Get element count for this section
    * @return int
    */
    TIGL_EXPORT int GetSectionElementCount() const;

    /**
    * @brief Get element for a given index
    * @return CCPACSWingSectionElement
    */
    TIGL_EXPORT CCPACSWingSectionElement& GetSectionElement(int index);
    TIGL_EXPORT const CCPACSWingSectionElement& GetSectionElement(int index) const;

    /**
    * @brief Gets the section transformation
    * @return CTiglTransformation
    */
    TIGL_EXPORT CTiglTransformation GetSectionTransformation() const;

    /**
    * @brief Returns the translation of a wing section in CPACS-coordinates
    * @return CTiglPoint
    */
    TIGL_EXPORT CTiglPoint GetTranslation() const;

    /**
    * @brief Returns the rotation of a wing section in degree
    * @return CTiglPoint
    */
    TIGL_EXPORT CTiglPoint GetRotation() const;

    /**
    * @brief Returns the scaling of a wing section
    * @return CTiglpoint 
    */
    TIGL_EXPORT CTiglPoint GetScaling() const;

    /** 
    * @brief Sets the translation vector for a wing section.
    * @param trans Translation vector
    *              Default data value: (0,0,0)
    */
    TIGL_EXPORT void SetTranslation(const CTiglPoint& trans);

    /**
    * @brief Sets the rotation angles for a wing section
    * @param rot Euler angles (x,y,z) -> (x,y', z'') in degree, which means, that the object will be rotaded in following order:
    * 1st Rotation around x-axis
    * 2nd Rotation around the rotaded y-axis y'
    * 3rd Rotation around the twice rotaded z axis x''.
    * Default data value:(0,0,0).
    */
    TIGL_EXPORT void SetRotation(const CTiglPoint& rot);

    /**
    * @brief Sets the scaling for a wing section
    * @param scaling Scaling in three dimensions, in the order xyz.
    * Default data value: (1,1,1)
    */
    TIGL_EXPORT void SetScaling(const CTiglPoint& scaling);

private:
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

};

} // end namespace tigl

#endif // CCPACSWINGSECTION_H
