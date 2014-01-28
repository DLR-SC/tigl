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
#include <vector>

#include "CCPACSTrailingEdgeDevices.h"
#include "CCPACSTrailingEdgeDevice.h"

namespace tigl
{

CCPACSTrailingEdgeDevices::CCPACSTrailingEdgeDevices()
{
}

CCPACSTrailingEdgeDevices::~CCPACSTrailingEdgeDevices()
{
    Cleanup();
}

void CCPACSTrailingEdgeDevices::Cleanup()
{
    for (std::size_t i = 0; i < trailingEdgeDevices.size(); i++) {
        trailingEdgeDevices.pop_back();
    }
}

// Read CPACS TrailingEdgeDevices elements
void CCPACSTrailingEdgeDevices::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& trailingEdgeDevicesXPath)
{
    Cleanup();
    ReturnCode tixiRet;
    int trailingEdgeDeviceCount;
    std::string tempString;
    char* elementPath;

    // Get trailingEdgeDevice element count
    tempString = trailingEdgeDevicesXPath + "/trailingEdgeDevices";
    elementPath = const_cast<char*>(tempString.c_str());
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath,
            "trailingEdgeDevice", &trailingEdgeDeviceCount);
    if (tixiRet != SUCCESS) {
        return;
    }

    // Loop over all trailingEdgeDevices
    for (int i = 1; i <= trailingEdgeDeviceCount; i++) {

        CCPACSTrailingEdgeDevice* trailingEdgeDevice =
                new CCPACSTrailingEdgeDevice();
        trailingEdgeDevices.push_back(trailingEdgeDevice);
        tempString = trailingEdgeDevicesXPath
                + "/trailingEdgeDevices/trailingEdgeDevice[";
        std::ostringstream xpath;
        xpath << tempString << i << "]";

        trailingEdgeDevice->ReadCPACS(tixiHandle, xpath.str());
    }
}

CCPACSTrailingEdgeDevice& CCPACSTrailingEdgeDevices::getTrailingEdgeDeviceByID(
        int id)
{
    return *trailingEdgeDevices[id-1];
}

int CCPACSTrailingEdgeDevices::getTrailingEdgeDeviceCount()
{
    return trailingEdgeDevices.size();
}

} // end namespace tigl
