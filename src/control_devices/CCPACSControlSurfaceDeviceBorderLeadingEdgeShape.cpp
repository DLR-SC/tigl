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

#include "CCPACSControlSurfaceDeviceBorderLeadingEdgeShape.h"

namespace tigl
{

CCPACSControlSurfaceDeviceBorderLeadingEdgeShape::CCPACSControlSurfaceDeviceBorderLeadingEdgeShape()
{
    relHeightLE = -1;
    xsiUpperSkin = -1;
    xsiLowerSkin = -1;
}

void CCPACSControlSurfaceDeviceBorderLeadingEdgeShape::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& leadingEdgeShapeXPath, TiglControlSurfaceType type)
{
    char* elementPath;
    std::string tempString;
    // getting subelements

    tempString = leadingEdgeShapeXPath + "/relHeightLE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &relHeightLE) != SUCCESS) {
        // couldnt read relHeightLE
    }

    tempString = leadingEdgeShapeXPath + "/xsiUpperSkin";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiUpperSkin) != SUCCESS) {
        // couldnt read xsiUpperSkin
    }

    tempString = leadingEdgeShapeXPath + "/xsiLowerSkin";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiLowerSkin) != SUCCESS) {
        // couldnt read xsiLowerSkin
    }
}


double CCPACSControlSurfaceDeviceBorderLeadingEdgeShape::getRelHeightLE() const
{
    return relHeightLE;
}

double CCPACSControlSurfaceDeviceBorderLeadingEdgeShape::getXsiUpperSkin() const
{
    return xsiUpperSkin;
}
double CCPACSControlSurfaceDeviceBorderLeadingEdgeShape::getXsiLowerSkin() const
{
    return xsiLowerSkin;
}

}
