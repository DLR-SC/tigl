/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-02-24 Martin Siggel <martin.siggel@dlr.de>
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

#include "CCPACSControlSurfaceDeviceAirfoil.h"

#include "CTiglError.h"
#include "CTiglLogging.h"

#include <cmath>

namespace tigl
{

CCPACSControlSurfaceDeviceAirfoil::CCPACSControlSurfaceDeviceAirfoil(CCPACSConfiguration* cfg)
    : _config(cfg)
{
}

CCPACSControlSurfaceDeviceAirfoil::~CCPACSControlSurfaceDeviceAirfoil()
{
}

void CCPACSControlSurfaceDeviceAirfoil::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    char* tmp = NULL;

    if (tixiGetTextElement(tixiHandle, (xpath + "/airfoilUID").c_str(), &tmp) == SUCCESS) {
        _airfoilUID = tmp;
    }
    else {
        throw CTiglError("Missing airfoilUID element in path: " + xpath + "!", TIGL_OPEN_FAILED);
    }

    if (tixiGetDoubleElement(tixiHandle, (xpath + "/rotX").c_str(), &_rotX) != SUCCESS) {
        _rotX = 90.0;
    }

    // check, if yscale != 1. If yes, we show a warning
    double scalY;
    if (tixiGetDoubleElement(tixiHandle, (xpath + "/scalY").c_str(), &scalY) == SUCCESS) {
        if (fabs(scalY - 1.0) > 1e-10) {
            LOG(WARNING) << "Y scaling in \"" << xpath << "\" ignored. Only 2D profiles supported.";
        }
    }

    if (tixiGetDoubleElement(tixiHandle, (xpath + "/scalZ").c_str(), &_scalZ) != SUCCESS) {
        _scalZ = 1.0;
    }
}



} // namespace til
