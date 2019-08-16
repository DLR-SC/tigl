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
* @brief  Implementation of CPACS wing positionings handling routines.
*/

#ifndef CCPACSPositioningS_H
#define CCPACSPositioningS_H

#include "generated/CPACSPositionings.h"
#include "tigl_internal.h"
#include "CCPACSPositioning.h"
#include "CTiglError.h"
#include <string>
#include <vector>
#include <map>

namespace tigl
{

class CCPACSPositionings : public generated::CPACSPositionings
{

public:
    // Constructor
    TIGL_EXPORT CCPACSPositionings(CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT ~CCPACSPositionings() OVERRIDE;

    // Read CPACS positionings element
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& wingXPath) OVERRIDE;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Returns the positioning matrix for a given section-uid
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(const std::string& sectionUID);

    /**
     * Set positioning transformation.
     * This function will take a section UID and will set the positioning of this section such that
     * the positioning obtained by this section is equal to the given position.
     * @Remark, If the section has no postioning associated to it, the function will create a new positioning that has no
     * from element.
     * @param sectionUID: The section that will be modified
     * @param newPosition: The new final position given by positionings for this section.
     * @param moveDependentPositionings: If true the sections depending of the positioning of sectionUID will also move.
     * If false, the sections depending of this will stay at the same place, this mean that the function will update
     * the parameters of the depending positonings.
     * @remark No verification is performed on the section UID. So, if the section is not present, a dirty useless
     * positioning will be created.
     */
    TIGL_EXPORT void SetPositioningTransformation(const std::string& sectionUID, CTiglPoint newPosition, bool moveDependentPositionings = false);

    // Cleanup routine
    TIGL_EXPORT void Cleanup();

protected:
    // Update internal positioning structure
    void Update();

private:
    bool                           invalidated;       /**< Internal state flag  */

};

} // end namespace tigl

#endif // CCPACSPositioningS_H
