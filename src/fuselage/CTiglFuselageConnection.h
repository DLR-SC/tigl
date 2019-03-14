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
* @brief  Implementation of CPACS fuselage connection handling routines.
*/

#ifndef CCPACSFUSELAGECONNECTION_H
#define CCPACSFUSELAGECONNECTION_H

#include "tixi.h"
#include "tigl_internal.h"
#include "CTiglTransformation.h"
#include <string>
#include <boost/optional.hpp>

namespace tigl
{

class CCPACSFuselageSegment;
class CCPACSFuselageProfile;

// TODO: this class is very similar to CCPACSWingConnection, merge into one class
class CTiglFuselageConnection
{
public:
    // Constructor
    TIGL_EXPORT CTiglFuselageConnection();
    TIGL_EXPORT CTiglFuselageConnection(const std::string& elementUID, CCPACSFuselageSegment* aSegment);

    // Returns the section UID of this connection
    TIGL_EXPORT const std::string& GetSectionUID() const;

    // Returns the section element UID of this connection
    TIGL_EXPORT const std::string& GetSectionElementUID() const;

    // Returns the section index of this connection
    TIGL_EXPORT int GetSectionIndex() const;

    // Returns the section element index of this connection
    TIGL_EXPORT int GetSectionElementIndex() const;

    // Returns the fuselage profile referenced by this connection
    TIGL_EXPORT CCPACSFuselageProfile& GetProfile();
    TIGL_EXPORT const CCPACSFuselageProfile& GetProfile() const;

    // Returns the positioning transformation (segment transformation) for the referenced section
    // If there are no positioning will return a trivial transformation
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation() const;

    // Returns the section matrix referenced by this connection
    TIGL_EXPORT CTiglTransformation GetSectionTransformation() const;

    // Returns the section element matrix referenced by this connection
    TIGL_EXPORT CTiglTransformation GetSectionElementTransformation() const;

    // Returns the fuselage matrix referenced by this connection
    TIGL_EXPORT CTiglTransformation GetFuselageTransformation() const;

    // Return the matrix transformation composed by every transformations apply to this connection.
    // If referenceCS==GLOBAL_COORDINATE_SYSTEM: -> fuselageMatrix*positoningMatrix*sectionMatrix*elementMatrix
    // If referenceCS==FUSELAGE_COORDINATE_SYSTEM: -> positoningMatrix*sectionMatrix*elementMatrix
    TIGL_EXPORT CTiglTransformation GetTotalTransformation(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM ) const;

    // Get the wire associate with the profile of this connection in global coordinate or in fuselage coordinate.
    TIGL_EXPORT TopoDS_Wire GetWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM ) const;

    // Get the coordinate of the origin of the coordinate system element in global coordinate or in fuselage coordinate.
    // Remark if the profile is not center on the coordinate system of the element, the origin do not correspond to
    // the center of the profile
    TIGL_EXPORT CTiglPoint GetOrigin(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Get the center of the profile of this connection in global coordinate or in fuselage coordinate.
    TIGL_EXPORT CTiglPoint GetCenterOfProfile( TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    TIGL_EXPORT double GetCircumferenceOfProfile(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

private:
    const std::string*             elementUID;   /**< UID in section/elements */

    int                            sectionIndex; /**< Index in sections */
    int                            elementIndex; /**< Index in section/elements */
    std::string                    sectionUID;   /**< UID in sections */
    CCPACSFuselageSegment*         segment;      /**< Parent segment */

};

} // end namespace tigl

#endif // CCPACSFUSELAGECONNECTION_H
