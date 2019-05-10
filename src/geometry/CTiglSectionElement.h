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

#ifndef TIGL_CTIGLSECTIONELEMENT_H
#define TIGL_CTIGLSECTIONELEMENT_H

#include "tigl_internal.h"
#include "CTiglTransformation.h"

#include <string>
#include <boost/optional.hpp>

namespace tigl
{

/**
 * This class is wrapper class for SectionElement (FuselageSectionElement and WingSectionElement).
 * It's goal is to simplify the access of all the transformations that are performed on a SectionElement.
 * Furthermore, it contain functions to compute and set useful information about the element,
 * such as: get/set the center of the profile, get/set the circumference of the profile
 */
class CTiglSectionElement
{

public:
    // Returns the section UID
    TIGL_EXPORT virtual const std::string& GetSectionUID() const = 0;

    // Returns the section element UID
    TIGL_EXPORT virtual const std::string& GetSectionElementUID() const = 0;

    // Returns the profile UID
    TIGL_EXPORT virtual const std::string& GetProfileUID() const = 0;

    // Returns the positioning transformation
    // If there are no positioning will return a trivial transformation
    TIGL_EXPORT virtual CTiglTransformation GetPositioningTransformation() const = 0;

    // Returns the section transformation
    TIGL_EXPORT virtual CTiglTransformation GetSectionTransformation() const = 0;

    // Returns the element transformation
    TIGL_EXPORT virtual CTiglTransformation GetSectionElementTransformation() const = 0;

    // Returns the fuselage or wing transformation
    TIGL_EXPORT virtual CTiglTransformation GetParentTransformation() const = 0;


    // Return the transformation composed by every transformations apply to this connection.
    TIGL_EXPORT virtual CTiglTransformation
    GetTotalTransformation(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;


    // Get the wire associate with the profile of this element
    TIGL_EXPORT virtual TopoDS_Wire GetWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const = 0;


    TIGL_EXPORT virtual CTiglPoint GetNormal(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const = 0;

    // Return the coordinate of the origin of the coordinate system element in global coordinate or in fuselage coordinate.
    // Remark if the profile is not center on the coordinate system of the element, the origin do not correspond to
    // the center of the profile
    TIGL_EXPORT CTiglPoint GetOrigin(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Return  the center of the profile of this connection in global coordinate or in fuselage coordinate.
    TIGL_EXPORT CTiglPoint GetCenter(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Return the circumference of the profile in the wanted coordinate system
    TIGL_EXPORT double GetCircumference(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Return the area of the profile in the wanted coordinate system
    TIGL_EXPORT double GetArea(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;


    // Return the height of the wire
    TIGL_EXPORT virtual double GetHeight(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Return the width of the wire
    TIGL_EXPORT virtual double GetWidth(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Set the element transformation such that the total transformation is equal to the given transformation
    // TODO set element and section transfomation simultaneously to cover all the case !) even the shearing case
    TIGL_EXPORT void SetTotalTransformation(const CTiglTransformation& newTotalTransformation,
                                            TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Scale the wire uniformly in world or fuselage/wing coordinates.
    // Remark that the effect is not always the same as to add a scaling transformation to the element transformation.
    TIGL_EXPORT virtual void ScaleUniformly(double scaleFactor, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);


    TIGL_EXPORT virtual void ScaleCircumference(double scaleFactor, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Set the origin of the element in the wanted coordinate system
    // Remark this function will modify the value contain in the tigl -> the geometry will change
    TIGL_EXPORT virtual void SetOrigin(const CTiglPoint& newOrigin,
                                       TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Set the center of mass of the element in the wanted coordinate system
    // Remark this function will modify the value contain in the tigl -> the geometry will change
    TIGL_EXPORT virtual void SetCenter(const CTiglPoint& newOrigin,
                                       TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);


    // Set the height of the wire.
    // Remark, will also set the width in the same time because a uniform scaling will be performed.
    TIGL_EXPORT virtual void SetHeight(double newHeight, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Set the width of the wire.
    // Remark, will also set the height in the same time because a uniform scaling will be performed.
    TIGL_EXPORT virtual void SetWidth(double newWidth, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Set the area of the wire to the wanted value 
    TIGL_EXPORT virtual void SetArea(double newArea, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM); 


protected:

    // Set the underlying CPACSTransformation (fuselage or wing ) with the given CTiglTransformation.
    // Calling this function will change the geometry of the aircraft.
    virtual void SetElementTransformation(const CTiglTransformation& newTransformation) = 0;


    // Set the underlying section CPACSTransformation with the given CTiglTransformation.
    // Calling this function will change the geometry of the aircraft.
    virtual void SetSectionTransformation(const CTiglTransformation& newTransformation) = 0;


    // Return the element transformation needed to move a point A to the position B in referenceCS.
    CTiglTransformation
    GetElementTrasformationToTranslatePoint(const CTiglPoint& newP, const CTiglPoint& oldP,
                                            TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Return the element transformation that will scale uniformly the the wire in the world or fuselage coordinate system.
    // Remark that the effect is not always the same as to add a scaling to the element transformation.
    CTiglTransformation
    GetElementTransformationForScaling(double scaleFactor, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);


};
}

#endif //TIGL_CTIGLSECTIONELEMENT_H
