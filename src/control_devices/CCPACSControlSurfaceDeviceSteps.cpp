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
#include <exception>
#include <sstream>
#include "CCPACSControlSurfaceDeviceSteps.h"


namespace tigl
{

CCPACSControlSurfaceDeviceSteps::CCPACSControlSurfaceDeviceSteps()
{
}

CCPACSControlSurfaceDeviceSteps::~CCPACSControlSurfaceDeviceSteps()
{
    Cleanup();
}

void CCPACSControlSurfaceDeviceSteps::Cleanup()
{
    for (std::size_t i = 0; i < controlSurfaceDeviceSteps.size(); i++) {
        controlSurfaceDeviceSteps.pop_back();
    }

}

// Read CPACS TrailingEdgeDeviceSteps element
void CCPACSControlSurfaceDeviceSteps::ReadCPACS(
        TixiDocumentHandle tixiHandle,
        const std::string& controlSurfaceDeviceStepsXPath)
{
    Cleanup();
    ReturnCode tixiRet;
    int controlSurfaceDeviceStepCount;
    std::string tempString;
    char* elementPath;

    /* Get controlSurfaceDevice element count */
    tempString = controlSurfaceDeviceStepsXPath;
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath,
            "step", &controlSurfaceDeviceStepCount);
    if (tixiRet != SUCCESS) {
        return;
    }

    // Loop over all controlSurfaceDevices
    for (int i = 1; i <= controlSurfaceDeviceStepCount; i++) {

        CCPACSControlSurfaceDeviceStep* controlSurfaceDeviceStep =
                new CCPACSControlSurfaceDeviceStep();
        controlSurfaceDeviceSteps.push_back(controlSurfaceDeviceStep);
        tempString = controlSurfaceDeviceStepsXPath
                + "/step[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";
        controlSurfaceDeviceStep->ReadCPACS(tixiHandle, xpath.str());
    }
}




CCPACSControlSurfaceDeviceStep& CCPACSControlSurfaceDeviceSteps::GetStep( int id ) const
{
    return *controlSurfaceDeviceSteps[id-1];
}

int CCPACSControlSurfaceDeviceSteps::GetStepCount() const
{
    return controlSurfaceDeviceSteps.size();
}

}


// end namespace tigl
