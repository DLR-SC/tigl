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

#include "CCPACSTrailingEdgeDeviceWingCutOut.h"

namespace tigl
{

CCPACSTrailingEdgeDeviceWingCutOut::CCPACSTrailingEdgeDeviceWingCutOut()
{
}

// Read CPACS TrailingEdgeDeviceWingCutOut element
void CCPACSTrailingEdgeDeviceWingCutOut::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& TrailingEdgeDeviceWingCutOutXPath)
{
    /*
    char* elementPath;
    std::string tempString;

    // getting subelements
    tempString = TrailingEdgeDeviceWingCutOutXPath + "/etaLE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &etaLE) != SUCCESS) {
        // couldnt read etaLE
    }

    tempString = TrailingEdgeDeviceWingCutOutXPath + "/etaTE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &etaTE) != SUCCESS) {
        etaTE = etaLE;
    }

    tempString = TrailingEdgeDeviceWingCutOutXPath + "/xsiLE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiLE) != SUCCESS) {
        // couldnt read xsiLE
    }
*/
}

}

// end namespace tigl
