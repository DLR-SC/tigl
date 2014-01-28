/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
 * Changed: $Id$
 *
 * Version: $Revision$
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

#include "CCPACSTrailingEdgeDeviceSteps.h"

namespace tigl
{

CCPACSTrailingEdgeDeviceSteps::CCPACSTrailingEdgeDeviceSteps()
{
}

CCPACSTrailingEdgeDeviceSteps::~CCPACSTrailingEdgeDeviceSteps()
{
    Cleanup();
}

void CCPACSTrailingEdgeDeviceSteps::Cleanup()
{
    for (std::size_t i = 0; i < trailingEdgeDeviceSteps.size(); i++) {
        trailingEdgeDeviceSteps.pop_back();
    }

}

// Read CPACS TrailingEdgeDeviceSteps element
void CCPACSTrailingEdgeDeviceSteps::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& TrailingEdgeDeviceStepsXPath)
{
    Cleanup();
    ReturnCode tixiRet;
    int trailingEdgeDeviceStepCount;
    std::string tempString;
    char* elementPath;

    /* Get trailingEdgeDevice element count */
    tempString = TrailingEdgeDeviceStepsXPath;
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath,
            "step", &trailingEdgeDeviceStepCount);
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

}



CCPACSTrailingEdgeDeviceStep& CCPACSTrailingEdgeDeviceSteps::getTrailingEdgeDeviceStepByID( int id )
{
    return *trailingEdgeDeviceSteps[id-1];
}

int CCPACSTrailingEdgeDeviceSteps::getTrailingEdgeDeviceStepCount()
{
    return trailingEdgeDeviceSteps.size();
}

}

// end namespace tigl
