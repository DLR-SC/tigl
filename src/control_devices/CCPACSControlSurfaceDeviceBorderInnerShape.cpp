/*
 * Copyright (C) 2007-2017 German Aerospace Center (DLR/SC)
 *
 * Created: 2017-02-28 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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

#include "CCPACSControlSurfaceDeviceBorderInnerShape.h"

namespace tigl
{

CCPACSControlSurfaceDeviceBorderInnerShape::CCPACSControlSurfaceDeviceBorderInnerShape()
{
    relHeightTE = -1;
    xsiTE = -1;
}

void CCPACSControlSurfaceDeviceBorderInnerShape::ReadCPACS(
        TixiDocumentHandle tixiHandle, const std::string& innerShapeXPath,
        TiglControlSurfaceType /* type */)
{
    char* elementPath;
    std::string tempString;
    // getting subelements

    // @todo: handle missing cpacs values
    tempString = innerShapeXPath + "/relHeightTE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &relHeightTE) != SUCCESS) {
        // couldnt read relHeightTE
    }

    tempString = innerShapeXPath + "/xsiTE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiTE) != SUCCESS) {
        // couldnt read xsiTE
    }
}


double CCPACSControlSurfaceDeviceBorderInnerShape::getRelHeightTE() const
{
    return relHeightTE;
}

double CCPACSControlSurfaceDeviceBorderInnerShape::getXsiTE() const
{
    return xsiTE;
}

}
