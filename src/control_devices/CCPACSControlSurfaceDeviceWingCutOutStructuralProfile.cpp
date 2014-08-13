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
 * @brief  Implementation of ..
 */

#include "CCPACSControlSurfaceDeviceWingCutOutStructuralProfile.h"

#include "CTiglError.h"
#include "CTiglPoint.h"
#include "CTiglLogging.h"

#include <sstream>
#include <iostream>
#include <fstream>


namespace tigl
{

// Constructor
CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::CCPACSControlSurfaceDeviceWingCutOutStructuralProfile(const std::string& path)
    : wingCutOutProfileXPath(path)
{

}

// Destructor
CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::~CCPACSControlSurfaceDeviceWingCutOutStructuralProfile(void)
{

}

// Read wingCutOut profile file
void CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    std::string namePath        = wingCutOutProfileXPath + "/name";
    std::string describtionPath = wingCutOutProfileXPath + "/description";
    std::string elementPath     = wingCutOutProfileXPath + "/pointList";

    // Get subelement "name"
    char* ptrName = NULL;
    if (tixiGetTextElement(tixiHandle, const_cast<char*>(namePath.c_str()), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get guide curve "uid"
    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingCutOutProfileXPath.c_str()), "uID", &ptrUID) == SUCCESS) {
        uid = ptrUID;
    }

    // Get subelement "description"
    char* ptrDescription = NULL;
    if (tixiGetTextElement(tixiHandle, const_cast<char*>(describtionPath.c_str()), &ptrDescription) == SUCCESS) {
        description = ptrDescription;
    }

    /* Get point count */
    int   pointCount;
    if (tixiGetNamedChildrenCount(tixiHandle, const_cast<char*>(elementPath.c_str()), "point", &pointCount) != SUCCESS) {
        throw CTiglError("Error: tixiGetNamedChildrenCount failed in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }

    std::string xXpath = elementPath + "/x";
    std::string yXpath = elementPath + "/y";
    std::string zXpath = elementPath + "/z";

    // check the number of elements in all three vectors. It has to be the same, otherwise cancel
    int countX;
    int countY;
    int countZ;
    if (tixiGetVectorSize(tixiHandle, const_cast<char*>(xXpath.c_str()), &countX) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <x> in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetVectorSize(tixiHandle, const_cast<char*>(yXpath.c_str()), &countY) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <y> in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetVectorSize(tixiHandle, const_cast<char*>(zXpath.c_str()), &countZ) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <z> in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }

    if (countX != countY || countX != countZ || countY != countZ) {
        throw CTiglError("Error: Vector size for wingCutOut points are not eqal in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }

    // read in vectors, vectors are allocated and freed by tixi
    double* xCoordinates = NULL;
    double* yCoordinates = NULL;
    double* zCoordinates = NULL;

    if (tixiGetFloatVector(tixiHandle, const_cast<char*>(xXpath.c_str()), &xCoordinates, countX) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <x> in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetFloatVector(tixiHandle, const_cast<char*>(yXpath.c_str()), &yCoordinates, countY) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <y> in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetFloatVector(tixiHandle, const_cast<char*>(zXpath.c_str()), &zCoordinates, countZ) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <z> in CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all points in the vector
    for (int i = 0; i < countX; i++) {
        PCTiglPoint point (new CTiglPoint(xCoordinates[i], yCoordinates[i], zCoordinates[i]));
        coordinates.push_back(point);
    }
}

// Returns the filename of the wingCutOut file
const std::string& CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::GetFileName(void) const
{
    return wingCutOutProfileXPath;
}

// Returns the name of the wingCutOut
const std::string& CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::GetName(void) const
{
    return name;
}

// Returns the UID of the wingCutOut
const std::string& CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::GetUID(void) const
{
    return uid;
}

// Returns the wingCutOut points as read from TIXI.
std::vector<PCTiglPoint> CCPACSControlSurfaceDeviceWingCutOutStructuralProfile::GetWingCutOutProfilePoints()
{
    return coordinates;
}



} // end namespace tigl

