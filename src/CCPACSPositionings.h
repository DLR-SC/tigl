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
    TIGL_EXPORT virtual ~CCPACSPositionings();

    // Read CPACS positionings element
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Returns the positioning matrix for a given section-uid
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(std::string sectionIndex);

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
