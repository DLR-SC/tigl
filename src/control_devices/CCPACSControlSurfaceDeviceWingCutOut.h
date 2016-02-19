/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#ifndef CCPACSControlSurfaceDeviceWingCutOut_H
#define CCPACSControlSurfaceDeviceWingCutOut_H

#include "tixi.h"
#include "CCPACSControlSurfaceDeviceWingCutOutProfiles.h"
#include "CCPACSControlSurfaceSkinCutOut.h"
#include "CCPACSCutOutControlPoints.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSControlSurfaceDeviceWingCutOut
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDeviceWingCutOut();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string & controlSurfaceDeviceWingCutOutXPath);
    
    TIGL_EXPORT const CCPACSCutOutControlPointsPtr cutOutProfileControlPoint() const;

private:
    CCPACSControlSurfaceDeviceWingCutOutProfiles wingCutOutProfiles;
    CCPACSControlSurfaceSkinCutOut upperSkin, lowerSkin;
    CCPACSCutOutControlPointsPtr _cutOutProfileControlPoint;
};

} // end namespace tigl

#endif // CCPACSControlSurfaceDeviceWingCutOut_H
