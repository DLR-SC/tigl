/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-19 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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

#include <TopoDS_Edge.hxx>

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
    fromGuideCurveUID = "";
    fromRelativeCircumferenceIsSet = true;
    guideCurveTopo.Nullify();
}

// Read guide curve file
void CCPACSGuideCurve::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    Cleanup();
    std::string namePath                  = GuideCurveXPath + "/name";
    std::string describtionPath           = GuideCurveXPath + "/description";
    std::string fromGuideCurveUIDPath     = GuideCurveXPath + "/fromGuideCurveUID";
    std::string guideCurveProfileUIDPath  = GuideCurveXPath + "/guideCurveProfileUID";

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

    // Get guide curve profile UID
    char* ptrGuideCurveProfileUID = NULL;
    if (tixiGetTextElement(tixiHandle, (GuideCurveXPath+"/guideCurveProfileUID").c_str(), &ptrGuideCurveProfileUID) != SUCCESS) {
        throw CTiglError("Error: XML error while reading guideCurveProfileUID in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
    profileUID = ptrGuideCurveProfileUID;

    // check if fromRelativeCircumference or fromGuideCurveUID is set
    bool foundFromRelativeCircumference = (tixiCheckElement(tixiHandle, (GuideCurveXPath + "/fromRelativeCircumference").c_str()) == SUCCESS);
    bool foundFromGuideCurveUID = (tixiCheckElement(tixiHandle, (GuideCurveXPath + "/fromGuideCurveUID").c_str()) == SUCCESS);
    if (foundFromRelativeCircumference && foundFromGuideCurveUID) {
        throw CTiglError("Error: It is forbidden to give fromRelativeCircumference AND fromGuideCurveUID in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
    else if (foundFromRelativeCircumference) {
        fromRelativeCircumferenceIsSet = true;
        if (tixiGetDoubleElement(tixiHandle, (GuideCurveXPath + "/fromRelativeCircumference").c_str(), &fromRelativeCircumference) != SUCCESS) {
            throw CTiglError("Error: XML error while reading fromRelativeCircumference in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
        }
    }
    else if (foundFromGuideCurveUID) {
        fromRelativeCircumferenceIsSet = false;
        char* ptrFromGuideCurveUID = NULL;
        if (tixiGetTextElement(tixiHandle, (GuideCurveXPath+"/fromGuideCurveUID").c_str(), &ptrFromGuideCurveUID) != SUCCESS) {
            throw CTiglError("Error: XML error while reading fromGuideCurveUID in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
        }
        fromGuideCurveUID = ptrFromGuideCurveUID;
    }
    else {
        throw CTiglError("Error: Attribute fromRelativeCircumference OR fromGuideCurveUID missing in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
    // read in toRelativeCircumference
    if (tixiGetDoubleElement(tixiHandle, (GuideCurveXPath + "/toRelativeCircumference").c_str(), &toRelativeCircumference) != SUCCESS) {
        throw CTiglError("Error: XML error while reading toRelativeCircumference in CCPACSGuideCurve::ReadCPACS", TIGL_XML_ERROR);
    }
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

// Returns the UID of the guide curve
const std::string& CCPACSGuideCurve::GetGuideCurveProfileUID(void) const
{
    return profileUID;
}

// Check if fromRelativeCircumference is set
bool CCPACSGuideCurve::GetFromRelativeCircumferenceIsSet(void) const
{
    return fromRelativeCircumferenceIsSet;
}

// Returns the UID of the neighboring guide curve
const std::string& CCPACSGuideCurve::GetFromGuideCurveUID(void) const
{
    return fromGuideCurveUID;
}

// Returns the relative circumference of the starting profile
const double& CCPACSGuideCurve::GetFromRelativeCircumference(void) const
{
    return fromRelativeCircumference;
}

// Returns the relative circumference of the end profile
const double& CCPACSGuideCurve::GetToRelativeCircumference(void) const
{
    return toRelativeCircumference;
}

void CCPACSGuideCurve::SetCurve(const TopoDS_Edge& edge)
{
    guideCurveTopo = edge;
}

const TopoDS_Edge& CCPACSGuideCurve::GetCurve() const
{
    return guideCurveTopo;
}

} // end namespace tigl
