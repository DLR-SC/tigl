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
 * @brief  not implemented yet...
 */

#include <iostream>
#include <sstream>
#include <exception>

#include "CCPACSControlSurfaceDeviceWingCutOutProfile.h"

namespace tigl
{

CCPACSControlSurfaceDeviceWingCutOutProfile::CCPACSControlSurfaceDeviceWingCutOutProfile()
{
}

// Read CPACS controlSurfaceDeviceWingCutOutProfile element
void CCPACSControlSurfaceDeviceWingCutOutProfile::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& controlSurfaceDeviceWingCutOutProfileXPath)
{
    char* elementPath;
    std::string tempString;
    eta = -1;
    rotZ = -1;

    // getting subelements
    tempString = controlSurfaceDeviceWingCutOutProfileXPath + "/eta";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &eta) != SUCCESS) {
        // couldnt read eta
    }

    tempString = controlSurfaceDeviceWingCutOutProfileXPath + "/rotZ";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &rotZ) != SUCCESS) {
        // couldnt read rotZ
    }

    // read profilUID
    char* ptrControlSurfaceDeviceProfileUID = NULL;
    tempString = controlSurfaceDeviceWingCutOutProfileXPath + "/profileUID";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrControlSurfaceDeviceProfileUID) != SUCCESS) {
        throw CTiglError("Error: XML error while reading controlSurfaceProfileUID in CCPACSControlSurfaceDeviceWingCutOutProfile::ReadCPACS", TIGL_XML_ERROR);
    }
    profileUID = ptrControlSurfaceDeviceProfileUID;
    printf(profileUID.c_str());
}

double CCPACSControlSurfaceDeviceWingCutOutProfile::getEta()
{
    return eta;
}
double CCPACSControlSurfaceDeviceWingCutOutProfile::getRotZ()
{
    return rotZ;
}

}

// end namespace tigl
