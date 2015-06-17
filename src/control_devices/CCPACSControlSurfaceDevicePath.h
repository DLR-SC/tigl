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

#ifndef CCPACSControlSurfaceDevicePath_H
#define CCPACSControlSurfaceDevicePath_H

#include <vector>

#include "tixi.h"
#include "CTiglError.h"
#include "CCPACSControlSurfaceDeviceSteps.h"
#include "CCPACSControlSurfaceDevicePathHingePoint.h"
#include "tigl_internal.h"

namespace tigl
{

class CCPACSControlSurfaceDevicePath
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDevicePath();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string & controlSurfaceDevicePathXPath);

    TIGL_EXPORT CCPACSControlSurfaceDeviceSteps getSteps() const;

    TIGL_EXPORT CCPACSControlSurfaceDevicePathHingePoint getInnerHingePoint() const;
    TIGL_EXPORT CCPACSControlSurfaceDevicePathHingePoint getOuterHingePoint() const;

    // TODO: this is ugly and should be changed. Instead, provide GetInnerHingeTranslation(iStep)
    TIGL_EXPORT std::vector<double> getInnerHingeTranslationsX() const;
    TIGL_EXPORT std::vector<double> getInnerHingeTranslationsY() const;
    TIGL_EXPORT std::vector<double> getInnerHingeTranslationsZ() const;
    TIGL_EXPORT std::vector<double> getOuterHingeTranslationsX() const;
    TIGL_EXPORT std::vector<double> getOuterHingeTranslationsZ() const;
    
    TIGL_EXPORT std::vector<double> getRelDeflections() const;
    TIGL_EXPORT std::vector<double> getHingeLineRotations() const;

private:
    CCPACSControlSurfaceDeviceSteps steps;

    CCPACSControlSurfaceDevicePathHingePoint innerPoint;
    CCPACSControlSurfaceDevicePathHingePoint outerPoint;

};

} // end namespace tigl

#endif // CCPACSTrailingEdgeDevicePath_H
