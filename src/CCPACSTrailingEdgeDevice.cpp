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

#include "CCPACSTrailingEdgeDevice.h"

namespace tigl {

CCPACSTrailingEdgeDevice::CCPACSTrailingEdgeDevice()
{

}

// Read CPACS trailingEdgeDevice elements
void CCPACSTrailingEdgeDevice::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& trailingEdgeDeviceXPath)
{
    char*       elementPath;
    std::string tempString;

    // Get subelement "outerShape"
    char* ptrName = NULL;
    tempString    = trailingEdgeDeviceXPath + "/outerShape";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS)
    {
        outerShape.ReadCPACS(tixiHandle, elementPath);
    }
}


CCPACSOuterShape CCPACSTrailingEdgeDevice::getOuterShape()
{
    return outerShape;
}

}

// end namespace tigl
