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
#include "CCPACSTransformation.h"
#include "CCPACSPositionings.h"

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
    // Returns true iff the element was set with a CCPACSSectionElement
    TIGL_EXPORT virtual bool IsValid() const = 0;

    // Returns the section UID
    TIGL_EXPORT virtual std::string GetSectionUID() const = 0;

    // Returns the section element UID
    TIGL_EXPORT virtual std::string GetSectionElementUID() const = 0;

    // Returns the profile UID
    TIGL_EXPORT virtual std::string GetProfileUID() const = 0;

    // Returns the positioning transformation
    // If there are no positioning will return a trivial transformation
    TIGL_EXPORT virtual CTiglTransformation GetPositioningTransformation() const = 0;

    TIGL_EXPORT virtual CCPACSPositionings& GetPositionings() = 0;

    // Returns the section transformation
    TIGL_EXPORT virtual CTiglTransformation GetSectionTransformation() const = 0;

    // Returns the element transformation
    TIGL_EXPORT virtual CTiglTransformation GetElementTransformation() const = 0;

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

    // Set the element and section transformation such that the total transformation is equal to the given transformation
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


    /**
     *
     * The normal angle is defined as the angle that bring the unit vector z of the profile form
     * its conventional direction to its current direction. The angle is always counter-clockwise around the normal
     * direction and the normal is taken at the center of the section. 
     * The conventional direction depends of of the profile type.
     * If the profile type is fuselage, the standard direction is the the vector that line on the profile plane
     * and end on the intersection of the line l, define by (x,0,1). If there is no intersection with the line l,
     * we set the end of the vector by the intersection of the line l2, defined by (1,0,z)
     * If the profile type is wing, the standard direction is the the vector that line on the profile plane
     * and end on the intersection of the line l, define by (0,y,1). If there is no intersection with the line l,
     * we set the end of the vector by the intersection of the line l2, defined by (0,1,z)
     *
     * @param referenceCS
     * @return
     */
    TIGL_EXPORT double GetRotationAroundNormal(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;


    /**
     * Set the normal of the profile.
     * @param referenceCS
     * @return
     */
    TIGL_EXPORT void SetNormal(CTiglPoint newNormal, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);


    /**
     * Set the rotation around the normal.
     * @remark The normal is always computed on the center of the section.
     * @see GetRotationAroundNormal to have a more precise definition of the angle.
     * @param angle
     * @param referenceCS
     * @return
     */
    TIGL_EXPORT void SetRotationAroundNormal(double angle, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);



    // Set the underlying CPACSTransformation of the positioning, section  and element
    // such that the multiplication of the three transformations give the input transformation!
    // Remark, the strength of this method is that we are sure that the decomposition in CCPACSTransformations are exact!!
    // We use the "DecomposeTRSRS" of the CTiglTransformation class and we set a part of the decomposition in the
    // positioning, another part in the section CCPACSTransformation and another part in element CCPACSTransformation,
    // thus the limitation of decomposing the matrix in translation, rotation and scaling is hacked.
    // Remark, to apply this function we need to be sure that a section exist for each element,
    // for the moment we assume it. But, in a near future, we can force this by creating a new section when is needed.
    void SetPositioningSectionElementTransformation(const CTiglTransformation &newTransformation);


protected:

    // Retrieve the stored "CCPACSTransformation" of the element transformation;
    virtual CCPACSTransformation& GetElementCCPACSTransformation() = 0;

    // Retrieve the stored "CCPACSTransformation" of the section transformation;
    virtual CCPACSTransformation& GetSectionCCPACSTransformation() = 0;

    // Invalidate the fuselage or the wing
    virtual void InvalidateParent() = 0;

    /**
     * If element or section transformation contains near zero scaling,
     * we set this scaling to 1.
     * Used to set the width or set the height when there is a zero scaling.
     */
    void SetElementAndSectionScalingToNoneZero();

    /**
     * Get the rotation that move the profile to the XZ plane and the unit vector Z of the profile to (0,0,1).
     *
     * @param referenceCS
     * @return a CTiglTransformation that hold the rotation
     */
    TIGL_EXPORT CTiglTransformation GetRotationToXZPlane(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    /**
     * Return conventional direction for the unit vector Z of the profile.
     * The conventional direction depends of of the profile type.
     * If the profile type is fuselage, the standard direction is the the vector that line on the profile plane
     * and end on the intersection of the line l, define by (x,0,1). If there is no intersection with the line l,
     * we set the end of the vector by the intersection of the line l2, defined by (1,0,z)
     * If the profile type is wing, the standard direction is the the vector that line on the profile plane
     * and end on the intersection of the line l, define by (0,y,1). If there is no intersection with the line l,
     * we set the end of the vector by the intersection of the line l2, defined by (0,1,z)
     *
     * @param referenceCS
     * @return
     */
    virtual CTiglPoint GetStdDirForProfileUnitZ(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const = 0;

    /**
     * Return the direction of the profile z unit vector in the referenceCS coordinate;
     * @param referenceCS
     * @return
     */
    CTiglPoint GetCurrentUnitZDirectionOfProfile(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

};
}

#endif //TIGL_CTIGLSECTIONELEMENT_H
