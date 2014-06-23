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
 * @brief  Implementation of ..
 */

#ifndef CCPACSTrailingEdgeDevicePath_H
#define CCPACSTrailingEdgeDevicePath_H

#include <vector>

#include "tixi.h"
#include "CTiglError.h"
#include "CCPACSTrailingEdgeDeviceSteps.h"
#include "CCPACSTrailingEdgeDevicePathHingePoint.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSTrailingEdgeDevicePath
{

private:

    CCPACSTrailingEdgeDeviceSteps steps;

    CCPACSTrailingEdgeDevicePathHingePoint innerPoint;
    CCPACSTrailingEdgeDevicePathHingePoint outerPoint;

public:
    TIGL_EXPORT CCPACSTrailingEdgeDevicePath();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
            const std::string & trailingEdgeDevicePathXPath);

    TIGL_EXPORT CCPACSTrailingEdgeDeviceSteps getSteps();

    TIGL_EXPORT CCPACSTrailingEdgeDevicePathHingePoint getInnerHingePoint();
    TIGL_EXPORT CCPACSTrailingEdgeDevicePathHingePoint getOuterHingePoint();

    TIGL_EXPORT std::vector<double> getInnerHingeTranslationsX();
    TIGL_EXPORT std::vector<double> getInnerHingeTranslationsY();
    TIGL_EXPORT std::vector<double> getInnerHingeTranslationsZ();
    TIGL_EXPORT std::vector<double> getOuterHingeTranslationsX();
    TIGL_EXPORT std::vector<double> getOuterHingeTranslationsZ();
    TIGL_EXPORT std::vector<double> getRelDeflections();
    TIGL_EXPORT std::vector<double> getHingeLineRotations();

};

} // end namespace tigl

#endif // CCPACSTrailingEdgeDevicePath_H
