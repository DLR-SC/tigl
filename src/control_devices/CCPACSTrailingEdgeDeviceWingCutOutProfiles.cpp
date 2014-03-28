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
 * @brief  Implementation of CPACS ...  handling routines.
 */

#include <iostream>
#include <sstream>
#include <exception>

#include "CCPACSTrailingEdgeDeviceWingCutOutProfiles.h"

namespace tigl
{

CCPACSTrailingEdgeDeviceWingCutOutProfiles::CCPACSTrailingEdgeDeviceWingCutOutProfiles()
{
}

// Read CPACS TrailingEdgeDeviceWingCutOutProfiles element
void CCPACSTrailingEdgeDeviceWingCutOutProfiles::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& TrailingEdgeDeviceWingCutOutProfilesXPath)
{
    /*
     *
    tempString = TrailingEdgeDeviceStepsXPath;
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath,
            "profile", &trailingEdgeDeviceStepCount);
    if (tixiRet != SUCCESS) {
        return;
    }

    // Loop over all trailingEdgeDevices
    for (int i = 1; i <= trailingEdgeDeviceStepCount; i++) {

        CCPACSTrailingEdgeDeviceStep* trailingEdgeDeviceStep =
                new CCPACSTrailingEdgeDeviceStep();
        trailingEdgeDeviceSteps.push_back(trailingEdgeDeviceStep);
        tempString = TrailingEdgeDeviceStepsXPath
                + "/step[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";

        trailingEdgeDeviceStep->ReadCPACS(tixiHandle, xpath.str());
    }
*/
}

}
// end namespace tigl
