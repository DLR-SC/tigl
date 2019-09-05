/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef TIGL_CTIGLFUSELAGESECTIONELEMENT_H
#define TIGL_CTIGLFUSELAGESECTIONELEMENT_H

#include "CTiglSectionElement.h"
#include "CCPACSFuselageProfile.h"

namespace tigl
{

class CCPACSFuselageSectionElement;
class CCPACSFuselageSection;
class CCPACSFuselage;

class CTiglFuselageSectionElement : public CTiglSectionElement
{

public:
    CTiglFuselageSectionElement();

    CTiglFuselageSectionElement(CCPACSFuselageSectionElement* element);

    TIGL_EXPORT bool IsValid() const override;

    TIGL_EXPORT std::string GetSectionUID() const override;

    TIGL_EXPORT std::string GetSectionElementUID() const override;

    TIGL_EXPORT virtual std::string GetProfileUID() const override;
    
    TIGL_EXPORT void SetProfileUID(const std::string& newProfileUID) override; 

    TIGL_EXPORT CTiglTransformation GetPositioningTransformation() const override;

    TIGL_EXPORT CCPACSPositionings& GetPositionings() override;

    TIGL_EXPORT CTiglTransformation GetSectionTransformation() const override;

    TIGL_EXPORT CTiglTransformation GetElementTransformation() const override;

    TIGL_EXPORT CTiglTransformation GetParentTransformation() const override;

    TIGL_EXPORT TopoDS_Wire GetWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const override;

    TIGL_EXPORT CTiglPoint GetNormal(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const override;

    // Returns the fuselage transformation
    TIGL_EXPORT CTiglTransformation GetFuselageTransformation() const;

    // Returns the fuselage profile referenced by this element
    TIGL_EXPORT CCPACSFuselageProfile& GetProfile();
    TIGL_EXPORT const CCPACSFuselageProfile& GetProfile() const;

protected:

    CCPACSTransformation& GetElementCCPACSTransformation() override;

    CCPACSTransformation& GetSectionCCPACSTransformation() override;

    void InvalidateParent() override;

    /**
     * Return the conventional direction for the unit vector z of the profile.
     *
     * This direction is used to determine the rotation around the normal and is defined as:
     * the vector that line on the profile plane and end on the intersection of the line l, define by (x,0,1).
     * If there is no intersection with the line l, we set the end of the vector by the intersection of the line l2,
     * defined by (1,0,z)
     *
     * @param referenceCS
     * @return
     */
    CTiglPoint GetStdDirForProfileUnitZ(TiglCoordinateSystem referenceCS) const override;




private:
    CCPACSFuselageSectionElement* element;
    CCPACSFuselageSection* section;
    CCPACSFuselage* fuselage;
};
}
#endif //TIGL_CTIGLFUSELAGESECTIONELEMENT_H
