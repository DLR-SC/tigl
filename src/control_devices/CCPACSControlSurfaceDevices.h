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
 * @brief  Implementation of ..
 */

#ifndef CCPACSControlSurfaceDevices_H
#define CCPACSControlSurfaceDevices_H

#include <string>
#include <vector>

#include "tixi.h"
#include "CTiglError.h"
#include "CCPACSControlSurfaceDevice.h"
#include "CSharedPtr.h"
#include "tigl_internal.h"

namespace tigl {

class CCPACSWingComponentSegment;

class CCPACSControlSurfaceDevices
{

private:

    typedef std::vector<CCPACSControlSurfaceDevice*> controlSurfaceDeviceContainer;
    controlSurfaceDeviceContainer controlSurfaceDevices;
    CCPACSWingComponentSegment* _componentSegment;

    void Cleanup();

public:

    TIGL_EXPORT CCPACSControlSurfaceDevices(CCPACSWingComponentSegment*);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& controlSurfaceDevicesXPath, TiglControlSurfaceType type);

    TIGL_EXPORT CCPACSControlSurfaceDevice& getControlSurfaceDeviceByID( int id );
    TIGL_EXPORT int getControlSurfaceDeviceCount();

    TIGL_EXPORT ~CCPACSControlSurfaceDevices();
};

typedef CSharedPtr<CCPACSControlSurfaceDevices> CCPACSControlSurfaceDevicesPtr;

} // end namespace tigl

#endif // CCPACSControlSurfaceDevices_H
