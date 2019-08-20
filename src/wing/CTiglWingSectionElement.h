/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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


#ifndef TIGL_CTIGLWINGSECTIONELEMENT_H
#define TIGL_CTIGLWINGSECTIONELEMENT_H

#include "CTiglSectionElement.h"
#include "CCPACSWingProfile.h"

namespace tigl
{

class CCPACSWing;
class CCPACSWingSectionElement;
class CCPACSWingSection;

class CTiglWingSectionElement: public CTiglSectionElement
{

public:
    CTiglWingSectionElement();

    CTiglWingSectionElement(CCPACSWingSectionElement* element);

    TIGL_EXPORT bool IsValid() const override;

    TIGL_EXPORT void SetAssociateElement(CCPACSWingSectionElement* element);

    TIGL_EXPORT std::string GetSectionUID() const override;

    TIGL_EXPORT std::string GetSectionElementUID() const override;

    TIGL_EXPORT virtual std::string GetProfileUID() const override;

    TIGL_EXPORT CTiglTransformation GetPositioningTransformation() const override;

    TIGL_EXPORT CCPACSPositionings& GetPositionings() override;

    TIGL_EXPORT CTiglTransformation GetSectionTransformation() const override;

    TIGL_EXPORT CTiglTransformation GetElementTransformation() const override;

    TIGL_EXPORT CTiglTransformation GetParentTransformation() const override;

    TIGL_EXPORT TopoDS_Wire GetWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const override;

    TIGL_EXPORT CTiglPoint GetNormal(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const override;

    // Returns the fuselage transformation
    TIGL_EXPORT CTiglTransformation GetWingTransformation() const;

    // Returns the fuselage profile referenced by this element
    TIGL_EXPORT CCPACSWingProfile& GetProfile();
    TIGL_EXPORT const CCPACSWingProfile& GetProfile() const;

    TIGL_EXPORT CTiglPoint GetChordPoint(double xsi, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

protected:

    CCPACSTransformation& GetElementCCPACSTransformation() override;

    CCPACSTransformation& GetSectionCCPACSTransformation() override;

    /**
     * Return the conventional direction for the unit vector z of the profile.
     *
     * This direction is used to determine the rotation around the normal and is defined as:
     * the the vector that line on the profile plane end on the intersection of the line l, define by (0,y,1).
     * If there is no intersection with the line l,we set the end of the vector by the intersection of the line l2,
     * defined by (0,1,z)
     *
     * @param referenceCS
     * @return
     */
    CTiglPoint GetStdDirForProfileUnitZ(TiglCoordinateSystem referenceCS) const override;

    void InvalidateParent() override;

private:
    CCPACSWingSectionElement* element;
    CCPACSWingSection* section;
    CCPACSWing* wing;

};


}

#endif //TIGL_CTIGLWINGSECTIONELEMENT_H
