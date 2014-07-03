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

#include "CCPACSControlSurfaceDevicePathHingePoint.h"

namespace tigl
{

CCPACSControlSurfaceDevicePathHingePoint::CCPACSControlSurfaceDevicePathHingePoint()
{
    xsi = -1;
    relHeight = -1;
}

// Read CPACS TrailingEdgeDevicePathHingePoint element
void CCPACSControlSurfaceDevicePathHingePoint::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& controlSurfaceDevicePathHingePointXPath)
{
    char* elementPath;
    std::string tempString;

    // getting subelements
    tempString = controlSurfaceDevicePathHingePointXPath + "/hingeXsi";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &xsi) != SUCCESS) {
        // couldnt read xsi valu
    }

    tempString = controlSurfaceDevicePathHingePointXPath + "/hingeRelHeight";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &relHeight) != SUCCESS) {
        // couldnt read relHeight
    }
}

double CCPACSControlSurfaceDevicePathHingePoint::getRelHeight()
{
    return relHeight;
}
double CCPACSControlSurfaceDevicePathHingePoint::getXsi()
{
    return xsi;
}

}

// end namespace tigl
