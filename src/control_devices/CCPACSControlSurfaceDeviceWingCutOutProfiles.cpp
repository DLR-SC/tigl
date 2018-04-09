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

#include "CCPACSControlSurfaceDeviceWingCutOutProfiles.h"

namespace tigl
{

CCPACSControlSurfaceDeviceWingCutOutProfiles::CCPACSControlSurfaceDeviceWingCutOutProfiles()
{
}

// Read CPACS TrailingEdgeDeviceWingCutOutProfiles element
void CCPACSControlSurfaceDeviceWingCutOutProfiles::ReadCPACS(
        TixiDocumentHandle tixiHandle,
        const std::string& controlSurfaceDeviceWingCutOutProfilesXPath)
{

    char* elementPath;
    std::string tempString;
    ReturnCode tixiRet;

    tempString = controlSurfaceDeviceWingCutOutProfilesXPath + "/cutOutProfiles";
    elementPath = const_cast<char*>(tempString.c_str());
    int profileCount;

    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath,
            "cutOutProfile", &profileCount);
    if (tixiRet != SUCCESS) {
        return;
    }

    // Loop over all controlSurfaceDeviceWingCoutOutProfile
    for (int i = 1; i <= profileCount; i++) {

        CCPACSControlSurfaceDeviceWingCutOutProfile* cutOutProfile =
                new CCPACSControlSurfaceDeviceWingCutOutProfile();
        wingCutOutProfiles.push_back(cutOutProfile);
        tempString = controlSurfaceDeviceWingCutOutProfilesXPath
                + "/cutOutProfiles/cutOutProfile[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";
        cutOutProfile->ReadCPACS(tixiHandle, xpath.str());
    }
}

CCPACSControlSurfaceDeviceWingCutOutProfiles::~CCPACSControlSurfaceDeviceWingCutOutProfiles()
{
    for (ControlSurfaceDeviceCutOutProfileContainer::iterator it = wingCutOutProfiles.begin(); it != wingCutOutProfiles.end(); ++it) {
        delete *it;
        *it = 0;
    }
    wingCutOutProfiles.clear();
}

}
// end namespace tigl
