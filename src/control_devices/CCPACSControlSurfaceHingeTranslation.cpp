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

#include "CCPACSControlSurfaceHingeTranslation.h"

namespace tigl
{

CCPACSControlSurfaceHingeTranslation::CCPACSControlSurfaceHingeTranslation()
{
    x = 0;
    y = 0;
    z = 0;
}

// Read CPACS HingeTranslation element
void CCPACSControlSurfaceHingeTranslation::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& HingeTranslationXPath)
{
    char* elementPath;
    std::string tempString;

    // getting subelements
    tempString = HingeTranslationXPath + "/x";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &x) != SUCCESS) {
    }


    tempString = HingeTranslationXPath + "/y";
    elementPath = const_cast<char*>(tempString.c_str());

    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetDoubleElement(tixiHandle, elementPath, &y) != SUCCESS) {
            // outer Translations dont have a Y coordinate
        }
    }


    tempString = HingeTranslationXPath + "/z";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &z) != SUCCESS) {
    }
}

double CCPACSControlSurfaceHingeTranslation::getX()
{
    return x;
}
double CCPACSControlSurfaceHingeTranslation::getY()
{
    return y;
}
double CCPACSControlSurfaceHingeTranslation::getZ()
{
    return z;
}

}

// end namespace tigl
