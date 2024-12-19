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
    TIGL_EXPORT CCPACSPositionings(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSPositionings(CCPACSFuselage* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSPositionings(CCPACSDuct* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSPositionings(CCPACSWing* parent, CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT ~CCPACSPositionings() override;

    // Read CPACS positionings element
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& wingXPath) override;

    /**
     * @brief GetPositioningTransformation returns the positioning matrix for a given section-uid
     * @param sectionIndex
     * @return Returns CTiglTransformation positioning matrix by sectionIndex
     */
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(const std::string& sectionIndex);

    /**
     * @brief GetPositioningCount returns the total count of positionings in a configuration per element (e.g. wing)
     * @return int Return total count of positionings
     */
    TIGL_EXPORT int GetPositioningCount() const;

    /**
     * @brief GetPositioning
     * @param index Note that index starts at 1
     * @return Returns CCPACSPositioning& by index
     */
    TIGL_EXPORT CCPACSPositioning& GetPositioning(int index);

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
