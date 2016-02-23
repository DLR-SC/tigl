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

#include "CTiglError.h"

#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "CCPACSControlSurfaceDeviceWingCutOutProfiles.h"

#include <string>

namespace tigl
{

CCPACSControlSurfaceDeviceWingCutOut::CCPACSControlSurfaceDeviceWingCutOut()
{
}

// Read CPACS ControlSurface DeviceWingCutOut element
void CCPACSControlSurfaceDeviceWingCutOut::ReadCPACS(
        TixiDocumentHandle tixiHandle,
        const std::string& xpath)
{
    char* elementPath;
    std::string tempString;


    tempString = xpath + "/cutOutProfiles";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        wingCutOutProfiles.ReadCPACS(tixiHandle,elementPath);
    }

    if (tixiCheckElement(tixiHandle, (xpath + "/upperSkin").c_str()) == SUCCESS) {
        _upperSkin.ReadCPACS(tixiHandle, xpath + "/upperSkin");
    }
    else {
        throw CTiglError("Missing upperSkin element in path: " + xpath + "!", TIGL_OPEN_FAILED);
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/upperSkin").c_str()) == SUCCESS) {
        _lowerSkin.ReadCPACS(tixiHandle, xpath + "/lowerSkin");
    }
    else {
        throw CTiglError("Missing lowerSkin element in path: " + xpath + "!", TIGL_OPEN_FAILED);
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/innerBorder").c_str()) == SUCCESS) {
        _innerBorder = CCPACSControlSurfaceSkinCutoutBorderPtr(new CCPACSControlSurfaceSkinCutoutBorder);
        _innerBorder->ReadCPACS(tixiHandle, xpath + "/innerBorder");
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/outerBorder").c_str()) == SUCCESS) {
        _outerBorder = CCPACSControlSurfaceSkinCutoutBorderPtr(new CCPACSControlSurfaceSkinCutoutBorder);
        _outerBorder->ReadCPACS(tixiHandle, xpath + "/outerBorder");
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/cutOutProfileControlPoint").c_str()) == SUCCESS) {
        _cutOutProfileControlPoints = CCPACSCutOutControlPointsPtr(new CCPACSCutOutControlPoints);
        _cutOutProfileControlPoints->ReadCPACS(tixiHandle, xpath + "/cutOutProfileControlPoint");
    }
}

const CCPACSCutOutControlPointsPtr CCPACSControlSurfaceDeviceWingCutOut::cutOutProfileControlPoints() const
{
    return _cutOutProfileControlPoints;
}

const CCPACSControlSurfaceSkinCutOut& CCPACSControlSurfaceDeviceWingCutOut::upperSkin() const
{
    return _upperSkin;
}

const CCPACSControlSurfaceSkinCutOut& CCPACSControlSurfaceDeviceWingCutOut::lowerSkin() const
{
    return _lowerSkin;
}

const CCPACSControlSurfaceSkinCutoutBorderPtr CCPACSControlSurfaceDeviceWingCutOut::innerBorder() const
{
    return _innerBorder;
}

const CCPACSControlSurfaceSkinCutoutBorderPtr CCPACSControlSurfaceDeviceWingCutOut::outerBorder() const
{
    return _outerBorder;
}

}

// end namespace tigl
