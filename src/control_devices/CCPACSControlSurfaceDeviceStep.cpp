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

    std::string tempString;

    tempString = controlSurfaceDeviceStepXPath + "/innerHingeTranslation";
    if (tixiCheckElement(tixiHandle, tempString.c_str()) == SUCCESS) {
        
        if (tixiGetDoubleElement(tixiHandle, (tempString + "/x").c_str(), &(innerHingeTranslation.x)) != SUCCESS) {
            // error
        }
        
        if (tixiGetDoubleElement(tixiHandle, (tempString + "/y").c_str(), &(innerHingeTranslation.y)) != SUCCESS) {
            // error
        }
        
        if (tixiGetDoubleElement(tixiHandle, (tempString + "/z").c_str(), &(innerHingeTranslation.z)) != SUCCESS) {
            // error
        }
        
    }

    tempString = controlSurfaceDeviceStepXPath + "/outerHingeTranslation";
    if (tixiCheckElement(tixiHandle, tempString.c_str()) == SUCCESS) {
        
        if (tixiGetDoubleElement(tixiHandle, (tempString + "/x").c_str(), &(outerHingeTranslation.x)) != SUCCESS) {
            // error
        }
        
        
        if (tixiGetDoubleElement(tixiHandle, (tempString + "/z").c_str(), &(outerHingeTranslation.z)) != SUCCESS) {
            // error
        }
        
        // the y value of the outer hinge translation equals the inner y value
         outerHingeTranslation.y = innerHingeTranslation.y;
    }

    tempString = controlSurfaceDeviceStepXPath + "/relDeflection";
    if ( tixiGetDoubleElement(tixiHandle,tempString.c_str(), &relDeflection) != SUCCESS) {
        // error while trying to read relDeflection out of CPACS data
    }

    tempString = controlSurfaceDeviceStepXPath + "/hingeLineRotation";
    if ( tixiCheckElement(tixiHandle, tempString.c_str()) == SUCCESS ) {
        if ( tixiGetDoubleElement(tixiHandle,tempString.c_str(), &hingeLineRotation) != SUCCESS) {
            // error while trying to read hingeLineRotation out of CPACS data
        }
    }
}

CTiglPoint CCPACSControlSurfaceDeviceStep::getInnerHingeTranslation() const
{
    return innerHingeTranslation;
}

CTiglPoint CCPACSControlSurfaceDeviceStep::getOuterHingeTranslation() const
{
    return outerHingeTranslation;
}

double CCPACSControlSurfaceDeviceStep::getRelDeflection() const
{
    return relDeflection;
}

double CCPACSControlSurfaceDeviceStep::getHingeLineRotation() const
{
    return hingeLineRotation;
}

}

// end namespace tigl
