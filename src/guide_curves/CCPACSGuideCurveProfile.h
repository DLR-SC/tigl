/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of a CPACS guide curve profile
*/

#ifndef CCPACSGUIDECURVEPROFILE_H
#define CCPACSGUIDECURVEPROFILE_H

#include "generated/CPACSGuideCurveProfileGeometry.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CTiglPoint.h"

namespace tigl
{
class CCPACSGuideCurveProfile : public generated::CPACSGuideCurveProfileGeometry
{
public:
    // Constructor
    TIGL_EXPORT CCPACSGuideCurveProfile();

    // Read CPACS guide curve file
    TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

    // Returns the guide curve points as read from TIXI.
    TIGL_EXPORT const std::vector<CTiglPoint>& GetGuideCurveProfilePoints();

private:
    std::vector<CTiglPoint> coordinates; ///< Coordinates of a guide curve points
};

} // end namespace tigl

#endif // CCPACSGUIDECURVEPROFILE_H

