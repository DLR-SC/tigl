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
* @brief  Implementation of CPACS wing connection handling routines.
*/

#ifndef CCPACSWINGCONNECTION_H
#define CCPACSWINGCONNECTION_H

#include <string>

#include "tixi.h"
#include "tigl_internal.h"
#include "CCPACSWingProfile.h"
#include "CTiglTransformation.h"

namespace tigl
{
class CCPACSWingSegment;

// TODO: this class does not represent any class in CPACS, rename to e.g. CTiglWingConnection
// TODO: this class is very similar to CCPACSFuselageConnection, merge into one class
class CCPACSWingConnection
{
public:
    // Constructor
    TIGL_EXPORT CCPACSWingConnection();
    TIGL_EXPORT CCPACSWingConnection(const std::string& elementUID, CCPACSWingSegment* aSegment);

    // Returns the section uid of this connection
    TIGL_EXPORT const std::string& GetSectionUID() const;

    // Returns the section element uid of this connection
    TIGL_EXPORT const std::string& GetSectionElementUID() const;

    // Returns the section index of this connection
    TIGL_EXPORT int GetSectionIndex() const;

    // Returns the section element index of this connection
    TIGL_EXPORT int GetSectionElementIndex() const;

    // Returns the wing profile referenced by this connection
    TIGL_EXPORT CCPACSWingProfile& GetProfile() const;

    // Returns the positioning transformation (segment transformation) for the referenced section
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation() const;

    // Returns the section matrix referenced by this connection
    TIGL_EXPORT CTiglTransformation GetSectionTransformation() const;

    // Returns the section element matrix referenced by this connection
    TIGL_EXPORT CTiglTransformation GetSectionElementTransformation() const;

private:
    const std::string* elementUID;    /**< UID of the connection-section/-elements */ //

    int                   sectionIndex;  /**< Index of the connection-sections */
    int                   elementIndex;  /**< Index of the connection-section/-elements */
    std::string           sectionUID;    /**< UID of the connection-sections */
    CCPACSWingSegment*    segment;       /**< Parent segment */

};

} // end namespace tigl

#endif // CCPACSWINGCONNECTION_H
