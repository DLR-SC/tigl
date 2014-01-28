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

#include "CCPACSTrailingEdgeDeviceStep.h"

namespace tigl
{

CCPACSTrailingEdgeDeviceStep::CCPACSTrailingEdgeDeviceStep()
{
    hingeLineRotation = -1;
    relDeflection = -1;
}

// Read CPACS TrailingEdgeDeviceStep element
void CCPACSTrailingEdgeDeviceStep::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& TrailingEdgeDeviceStepXPath)
{

    char*       elementPath;
    std::string tempString;

    tempString = TrailingEdgeDeviceStepXPath + "/innerHingeTranslation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        innerHingeTranslation.ReadCPACS(tixiHandle, elementPath);
    }

    tempString = TrailingEdgeDeviceStepXPath + "/outerHingeTranslation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        outerHingeTranslation.ReadCPACS(tixiHandle, elementPath);
    }

    tempString = TrailingEdgeDeviceStepXPath + "/relDeflection";
    elementPath = const_cast<char*>(tempString.c_str());
    if ( tixiGetDoubleElement(tixiHandle,elementPath, &relDeflection) != SUCCESS) {
        // error while trying to read relDeflection out of CPACS data
    }

    tempString = TrailingEdgeDeviceStepXPath + "/hingeLineRotation";
    elementPath = const_cast<char*>(tempString.c_str());
    if ( tixiCheckElement(tixiHandle, elementPath) == SUCCESS ) {
        if ( tixiGetDoubleElement(tixiHandle,elementPath, &hingeLineRotation) != SUCCESS) {
            // error while trying to read hingeLineRotation out of CPACS data
        }
    }
}

CCPACSHingeTranslation CCPACSTrailingEdgeDeviceStep::getInnerHingeTranslation()
{
    return innerHingeTranslation;
}

CCPACSHingeTranslation CCPACSTrailingEdgeDeviceStep::getOuterHingeTranslation()
{
    return outerHingeTranslation;
}

double CCPACSTrailingEdgeDeviceStep::getRelDeflection()
{
    return relDeflection;
}

double CCPACSTrailingEdgeDeviceStep::getHingeLineRotation()
{
    return hingeLineRotation;
}

}

// end namespace tigl
