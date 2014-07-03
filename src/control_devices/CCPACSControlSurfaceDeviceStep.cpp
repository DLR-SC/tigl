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

#include "CCPACSControlSurfaceDeviceStep.h"

namespace tigl
{

CCPACSControlSurfaceDeviceStep::CCPACSControlSurfaceDeviceStep()
{
    hingeLineRotation = -1;
    relDeflection = -1;
}

// Read CPACS TrailingEdgeDeviceStep element
void CCPACSControlSurfaceDeviceStep::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& controlSurfaceDeviceStepXPath)
{

    char*       elementPath;
    std::string tempString;

    tempString = controlSurfaceDeviceStepXPath + "/innerHingeTranslation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        innerHingeTranslation.ReadCPACS(tixiHandle, elementPath);
    }

    tempString = controlSurfaceDeviceStepXPath + "/outerHingeTranslation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        outerHingeTranslation.ReadCPACS(tixiHandle, elementPath);
    } else {
        outerHingeTranslation = innerHingeTranslation;
    }

    tempString = controlSurfaceDeviceStepXPath + "/relDeflection";
    elementPath = const_cast<char*>(tempString.c_str());
    if ( tixiGetDoubleElement(tixiHandle,elementPath, &relDeflection) != SUCCESS) {
        // error while trying to read relDeflection out of CPACS data
    }

    tempString = controlSurfaceDeviceStepXPath + "/hingeLineRotation";
    elementPath = const_cast<char*>(tempString.c_str());
    if ( tixiCheckElement(tixiHandle, elementPath) == SUCCESS ) {
        if ( tixiGetDoubleElement(tixiHandle,elementPath, &hingeLineRotation) != SUCCESS) {
            // error while trying to read hingeLineRotation out of CPACS data
        }
    }
}

CCPACSHingeTranslation CCPACSControlSurfaceDeviceStep::getInnerHingeTranslation()
{
    return innerHingeTranslation;
}

CCPACSHingeTranslation CCPACSControlSurfaceDeviceStep::getOuterHingeTranslation()
{
    return outerHingeTranslation;
}

double CCPACSControlSurfaceDeviceStep::getRelDeflection()
{
    return relDeflection;
}

double CCPACSControlSurfaceDeviceStep::getHingeLineRotation()
{
    return hingeLineRotation;
}

}

// end namespace tigl
