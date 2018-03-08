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

#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

#include "CCPACSControlSurfaceDevices.h"
#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"

namespace tigl
{

CCPACSControlSurfaceDevices::CCPACSControlSurfaceDevices(CCPACSConfiguration* cfg, CCPACSWingComponentSegment* cs)
    : _componentSegment(cs), _config(cfg)
{
}

CCPACSControlSurfaceDevices::~CCPACSControlSurfaceDevices()
{
    Cleanup();
}

void CCPACSControlSurfaceDevices::Cleanup()
{
    for (std::size_t i = 0; i < controlSurfaceDevices.size(); i++) {
        CCPACSControlSurfaceDevice* dev = controlSurfaceDevices[i];
        delete dev;
    }
    controlSurfaceDevices.clear();
}

// Read CPACS TrailingEdgeDevices elements
void CCPACSControlSurfaceDevices::ReadCPACS(
        TixiDocumentHandle tixiHandle,
        const std::string& controlSurfaceDevicesXPath, TiglControlSurfaceType type)
{
    std::string typestring = "";

    if (type == TRAILING_EDGE_DEVICE) {
        typestring = "trailingEdgeDevice";
    }
    else if (type == LEADING_EDGE_DEVICE) {
        typestring = "leadingEdgeDevice";
    }
    else if (type == SPOILER) {
        typestring = "spoiler";
    }
    else {
        return;
    }

    // Get ControlSurfaceDevive element count
    int controlSurfaceDeviceCount;
    if (tixiGetNamedChildrenCount(tixiHandle, controlSurfaceDevicesXPath.c_str(), typestring.c_str(), &controlSurfaceDeviceCount) != SUCCESS) {
        return;
    }

    // Loop over all controlSurfaces
    for (int i = 1; i <= controlSurfaceDeviceCount; i++) {
        CCPACSControlSurfaceDevice* controlSurfaceDevice =
                new CCPACSControlSurfaceDevice(_config, _componentSegment);
        controlSurfaceDevices.push_back(controlSurfaceDevice);

        std::ostringstream xpath;
        xpath << controlSurfaceDevicesXPath << "/" << typestring + "[" << i << "]";
        controlSurfaceDevice->ReadCPACS(tixiHandle, xpath.str(), type);
    }
}

CCPACSControlSurfaceDevice& CCPACSControlSurfaceDevices::getControlSurfaceDeviceByID(
        int id)
{
    return *controlSurfaceDevices[id-1];
}

CCPACSControlSurfaceDevice& CCPACSControlSurfaceDevices::getControlSurfaceDevice(
        const std::string& uid)
{
    for ( int id = 0; id < controlSurfaceDevices.size(); id ++ ) {
        if ( controlSurfaceDevices[id]->GetUID() == uid ) {
            return (CCPACSControlSurfaceDevice &)(*(controlSurfaceDevices[id]));
        }
    }
    throw CTiglError("Error: Invalid uid in CCPACSControlSurfaceDevices::getControlSurfaceDevice", TIGL_UID_ERROR);
}

int CCPACSControlSurfaceDevices::getControlSurfaceDeviceCount()
{
    return (int)controlSurfaceDevices.size();
}

} // end namespace tigl
