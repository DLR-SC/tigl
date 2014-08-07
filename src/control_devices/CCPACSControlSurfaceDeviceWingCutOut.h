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
/**
 * @file
 * @brief not implemented yet...
 */

#ifndef CCPACSControlSurfaceDeviceWingCutOut_H
#define CCPACSControlSurfaceDeviceWingCutOut_H

#include "tixi.h"
#include "CTiglError.h"
#include "CCPACSControlSurfaceDeviceWingCutOutProfiles.h"
#include "tigl_internal.h"


namespace tigl
{

class CCPACSControlSurfaceDeviceWingCutOut
{

private:

    // upper Skin
    // lower Skin
    // cutOutProfiles
    // ControlPoint

    CCPACSControlSurfaceDeviceWingCutOutProfiles wingCutOutProfiles;

public:
    TIGL_EXPORT CCPACSControlSurfaceDeviceWingCutOut();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
            const std::string & controlSurfaceDeviceWingCutOutXPath);

};

} // end namespace tigl

#endif // CCPACSControlSurfaceDeviceWingCutOut_H
