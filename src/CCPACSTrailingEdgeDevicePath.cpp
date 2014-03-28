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

#include "CCPACSTrailingEdgeDevicePath.h"

namespace tigl
{

CCPACSTrailingEdgeDevicePath::CCPACSTrailingEdgeDevicePath()
{
}

// Read CPACS TrailingEdgeDevicePath element
void CCPACSTrailingEdgeDevicePath::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& trailingEdgeDevicePathXPath)
{

    char*       elementPath;
    std::string tempString;

    tempString = trailingEdgeDevicePathXPath + "/steps";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        steps.ReadCPACS(tixiHandle, elementPath);
    }

    tempString = trailingEdgeDevicePathXPath + "/innerHingePoint";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        innerPoint.ReadCPACS(tixiHandle, elementPath);
    }

    tempString = trailingEdgeDevicePathXPath + "/outerHingePoint";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        outerPoint.ReadCPACS(tixiHandle, elementPath);
    }

}

CCPACSTrailingEdgeDeviceSteps CCPACSTrailingEdgeDevicePath::getSteps()
{
    return steps;
}

CCPACSTrailingEdgeDevicePathHingePoint CCPACSTrailingEdgeDevicePath::getInnerHingePoint()
{
    return innerPoint;
}

CCPACSTrailingEdgeDevicePathHingePoint CCPACSTrailingEdgeDevicePath::getOuterHingePoint()
{
    return outerPoint;
}

std::vector<double> CCPACSTrailingEdgeDevicePath::getInnerHingeTranslationsX()
{
    std::vector<double> tmp;
    for ( int i = 1; i <= steps.getTrailingEdgeDeviceStepCount(); i++ ) {
        tmp.push_back( steps.getTrailingEdgeDeviceStepByID(i).getInnerHingeTranslation().getX() );
    }
    return tmp;
}

std::vector<double> CCPACSTrailingEdgeDevicePath::getInnerHingeTranslationsY()
{
    std::vector<double> tmp;
    for ( int i = 1; i <= steps.getTrailingEdgeDeviceStepCount(); i++ ) {
        tmp.push_back( steps.getTrailingEdgeDeviceStepByID(i).getInnerHingeTranslation().getY() );
    }
    return tmp;
}

std::vector<double> CCPACSTrailingEdgeDevicePath::getInnerHingeTranslationsZ()
{
    std::vector<double> tmp;
    for ( int i = 1; i <= steps.getTrailingEdgeDeviceStepCount(); i++ ) {
        tmp.push_back( steps.getTrailingEdgeDeviceStepByID(i).getInnerHingeTranslation().getZ() );
    }
    return tmp;
}

std::vector<double> CCPACSTrailingEdgeDevicePath::getOuterHingeTranslationsX()
{
    std::vector<double> tmp;
    for ( int i = 1; i <= steps.getTrailingEdgeDeviceStepCount(); i++ ) {
        tmp.push_back( steps.getTrailingEdgeDeviceStepByID(i).getOuterHingeTranslation().getX() );
    }
    return tmp;
}

std::vector<double> CCPACSTrailingEdgeDevicePath::getOuterHingeTranslationsZ()
{
    std::vector<double> tmp;
    for ( int i = 1; i <= steps.getTrailingEdgeDeviceStepCount(); i++ ) {
        tmp.push_back( steps.getTrailingEdgeDeviceStepByID(i).getOuterHingeTranslation().getZ() );
    }
    return tmp;
}

std::vector<double> CCPACSTrailingEdgeDevicePath::getRelDeflections()
{
    std::vector<double> tmp;
    for ( int i = 1; i <= steps.getTrailingEdgeDeviceStepCount(); i++ ) {
        tmp.push_back( steps.getTrailingEdgeDeviceStepByID(i).getRelDeflection() );
    }
    return tmp;
}

std::vector<double> CCPACSTrailingEdgeDevicePath::getHingeLineRotations()
{
    std::vector<double> tmp;
    for ( int i = 1; i <= steps.getTrailingEdgeDeviceStepCount(); i++ ) {
        tmp.push_back( steps.getTrailingEdgeDeviceStepByID(i).getHingeLineRotation() );
    }
    return tmp;
}



}
// end namespace tigl
