/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: CCPACSWingConnection.h 2641 2017-03-30 21:08:46Z bgruber $ 
*
* Version: $Revision: 2641 $
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
#include "Cache.h"

namespace tigl
{
class CCPACSWingSegment;

// TODO(bgruber): this class is very similar to CCPACSFuselageConnection, merge into one class
class CTiglWingConnection
{
public:
    // Constructor
    TIGL_EXPORT CTiglWingConnection(CCPACSWingSegment* aSegment);
    TIGL_EXPORT CTiglWingConnection(const std::string& elementUID, CCPACSWingSegment* aSegment);

    // Returns the section uid of this connection
    TIGL_EXPORT const std::string& GetSectionUID() const;

    // Returns the section element uid of this connection
    TIGL_EXPORT const std::string& GetSectionElementUID() const;

    // Returns the section index of this connection
    TIGL_EXPORT int GetSectionIndex() const;

    // Returns the section element index of this connection
    TIGL_EXPORT int GetSectionElementIndex() const;

    // Returns the wing profile referenced by this connection
    TIGL_EXPORT CCPACSWingProfile& GetProfile();
    TIGL_EXPORT const CCPACSWingProfile& GetProfile() const;

    // Returns the positioning transformation (segment transformation) for the referenced section
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation() const;

    // Returns the section matrix referenced by this connection
    TIGL_EXPORT CTiglTransformation GetSectionTransformation() const;

    // Returns the section element matrix referenced by this connection
    TIGL_EXPORT CTiglTransformation GetSectionElementTransformation() const;

    TIGL_EXPORT void SetElementUID(const std::string& uid);

private:
    struct ResolvedIndices {
        int sectionIndex;
        int elementIndex;
        const std::string* sectionUidPtr;
        const std::string* profileUIDPtr;
    };

    void resolve(ResolvedIndices& cache) const;

private:
    Cache<ResolvedIndices, CTiglWingConnection> m_resolved;
    std::string           elementUID;    /**< UID of the connection-section/-elements */
    CCPACSWingSegment*    segment;       /**< Parent segment */

};

} // end namespace tigl

#endif // CCPACSWINGCONNECTION_H
