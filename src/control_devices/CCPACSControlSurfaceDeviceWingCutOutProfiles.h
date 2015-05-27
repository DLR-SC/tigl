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

#ifndef CCPACSControlSurfaceDeviceWingCutOutProfiles_H
#define CCPACSControlSurfaceDeviceWingCutOutProfiles_H

#include <vector>

#include "tixi.h"
#include "CTiglError.h"
#include "CCPACSControlSurfaceDeviceWingCutOutProfile.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSControlSurfaceDeviceWingCutOutProfiles
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDeviceWingCutOutProfiles();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string & controlSurfaceDeviceWingCutOutProfilesXPath);

private:
    typedef std::vector<CCPACSControlSurfaceDeviceWingCutOutProfile*> ControlSurfaceDeviceCutOutProfileContainer;
    ControlSurfaceDeviceCutOutProfileContainer wingCutOutProfiles;
};

} // end namespace tigl

#endif // CCPACSTrailingEdgeDeviceWingCutOutProfiles_H
