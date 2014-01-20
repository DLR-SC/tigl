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

#include "CCPACSBorder.h"

namespace tigl
{

CCPACSBorder::CCPACSBorder()
{
    xsiType = "";
    xsiLE = -1;
    xsiTE = -1;
    etaLE = -1;
    etaTE = -1;
}

// Read CPACS Border element
void CCPACSBorder::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& BorderXPath)
{

    char* elementPath;
    std::string tempString;

    char* attributeName = NULL;
    char* attributeValue = NULL;
    tixiGetAttributeName(tixiHandle, BorderXPath.c_str(), 1, &attributeName);
    tixiGetTextAttribute(tixiHandle, BorderXPath.c_str(), attributeName,
            &attributeValue);
    xsiType = attributeValue;

    if (xsiType == "controlSurfaceBorderTrailingEdgeType") {
        // getting subelements
        tempString = BorderXPath + "/etaLE";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiGetDoubleElement(tixiHandle, elementPath, &etaLE) != SUCCESS) {
            // couldnt read etaLE
        }

        tempString = BorderXPath + "/etaTE";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiGetDoubleElement(tixiHandle, elementPath, &etaTE) != SUCCESS) {
            etaTE = etaLE;
        }

        tempString = BorderXPath + "/xsiLE";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiLE) != SUCCESS) {
            // couldnt read xsiLE
        }
    }

}

double CCPACSBorder::getEtaLE()
{
    return etaLE;
}
double CCPACSBorder::getEtaTE()
{
    return etaTE;
}
double CCPACSBorder::getXsiLE()
{
    return xsiLE;
}
double CCPACSBorder::getxsiTE()
{
    return xsiTE;
}

}

// end namespace tigl
