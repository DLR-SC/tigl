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

#include "CCPACSControlSurfaces.h"

namespace tigl
{

CCPACSControlSurfaces::CCPACSControlSurfaces(CCPACSConfiguration* cfg, CCPACSWingComponentSegment* cs)
{
    controlSurfaceDevices = CCPACSControlSurfaceDevicesPtr(new CCPACSControlSurfaceDevices(cfg, cs));
}

// Read CPACS segment elements
void CCPACSControlSurfaces::ReadCPACS(TixiDocumentHandle tixiHandle,
                                      const std::string& segmentXPath)
{
    char* elementPath;
    std::string tempString;

    // read TrailingEdgeDevices
    tempString = segmentXPath + "/trailingEdgeDevices";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        controlSurfaceDevices->ReadCPACS(tixiHandle, elementPath, TRAILING_EDGE_DEVICE);
    }

    // read LeadingEdgeDevices
    tempString = segmentXPath + "/leadingEdgeDevices";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        controlSurfaceDevices->ReadCPACS(tixiHandle, elementPath, LEADING_EDGE_DEVICE);
    }

    tempString = segmentXPath + "/spoilers";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle,elementPath) == SUCCESS) {
        /*
         * Spoilers have been disabled, because there are still some problems with the implementation.
         * To Solve these problems, following thing should be done:
         *  -> Find a Method to determine Spoiler Thickness
         *  -> How to Build The Cut Out Shape
         */
        // controlSurfaceDevices->ReadCPACS(tixiHandle,elementPath, SPOILER);
    }
}

CCPACSControlSurfaceDevices* CCPACSControlSurfaces::getControlSurfaceDevices()
{
    return controlSurfaceDevices.get();
}

CCPACSControlSurfaceDevices const * CCPACSControlSurfaces::getControlSurfaceDevices() const
{
    return controlSurfaceDevices.get();
}

} // end namespace tigl
