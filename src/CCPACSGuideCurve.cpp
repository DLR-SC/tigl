/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of a CPACS guide curve
*/

#include "tigl.h"
#include "CCPACSGuideCurve.h"
#include "CTiglError.h"
#include "CTiglPoint.h"
#include "CTiglLogging.h"

namespace tigl
{

// Constructor
CCPACSGuideCurve::CCPACSGuideCurve(const std::string& path) : GuideCurveXPath(path)
{
    Cleanup();
}

// Destructor
CCPACSGuideCurve::~CCPACSGuideCurve(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSGuideCurve::Cleanup(void)
{
    name       = "";
    uid        = "";
    description= "";
    coordinates.clear();
}

// Read guide curve file
void CCPACSGuideCurve::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    Cleanup();
    std::string namePath        = GuideCurveXPath + "/name";
    std::string describtionPath = GuideCurveXPath + "/description";
    std::string elementPath     = GuideCurveXPath + "/pointList";

    // Get subelement "name"
    char* ptrName = NULL;
    if (tixiGetTextElement(tixiHandle, const_cast<char*>(namePath.c_str()), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get guide curve "uid"
    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(GuideCurveXPath.c_str()), "uID", &ptrUID) == SUCCESS) {
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
        throw CTiglError("Error: tixiGetNamedChildrenCount failed in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }

    std::string xXpath = elementPath + "/x";
    std::string yXpath = elementPath + "/y";
    std::string zXpath = elementPath + "/z";

    // check the number of elements in all three vectors. It has to be the same, otherwise cancel
    int countX;
    int countY;
    int countZ;
    if (tixiGetVectorSize(tixiHandle, const_cast<char*>(xXpath.c_str()), &countX) != SUCCESS){
        throw CTiglError("Error: XML error while reading point vector <x> in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetVectorSize(tixiHandle, const_cast<char*>(yXpath.c_str()), &countY) != SUCCESS){
        throw CTiglError("Error: XML error while reading point vector <y> in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetVectorSize(tixiHandle, const_cast<char*>(zXpath.c_str()), &countZ) != SUCCESS){
        throw CTiglError("Error: XML error while reading point vector <z> in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }

    if (countX != countY || countX != countZ || countY != countZ) {
        throw CTiglError("Error: Vector size for guide curve points are not eqal in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }

    // read in vectors, vectors are allocated and freed by tixi
    double *xCoordinates = NULL;
    double *yCoordinates = NULL;
    double *zCoordinates = NULL;

    if (tixiGetFloatVector(tixiHandle, const_cast<char*>(xXpath.c_str()), &xCoordinates, countX) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <x> in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetFloatVector(tixiHandle, const_cast<char*>(yXpath.c_str()), &yCoordinates, countY) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <y> in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
    if (tixiGetFloatVector(tixiHandle, const_cast<char*>(zXpath.c_str()), &zCoordinates, countZ) != SUCCESS) {
        throw CTiglError("Error: XML error while reading point vector <z> in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all points in the vector
    for (int i = 0; i < countX; i++) {
        PCTiglPoint point (new CTiglPoint(xCoordinates[i], yCoordinates[i], zCoordinates[i]));
        coordinates.push_back(point);
    }
}

// Returns the filename of the guide curve file
const std::string& CCPACSGuideCurve::GetFileName(void) const
{
    return GuideCurveXPath;
}

// Returns the name of the guide curve
const std::string& CCPACSGuideCurve::GetName(void) const
{
    return name;
}

// Returns the UID of the guide curve
const std::string& CCPACSGuideCurve::GetUID(void) const
{
    return uid;
}

// Returns the guide curve points as read from TIXI.
std::vector<PCTiglPoint> CCPACSGuideCurve::GetGuideCurvePoints()
{
    return coordinates;
}

} // end namespace tigl
