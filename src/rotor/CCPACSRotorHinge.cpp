/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-11 Philipp Kunze <Philipp.Kunze@dlr.de>
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
* @brief  Implementation of CPACS rotor hinge handling routines.
*/

#include <cmath>

#include "CCPACSRotorHinge.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSRotorHinge::CCPACSRotorHinge(CCPACSRotorBladeAttachment* rotorBladeAttachment)
    : rotorBladeAttachment(rotorBladeAttachment)
{
    Cleanup();
}

// Virtual destructor
CCPACSRotorHinge::~CCPACSRotorHinge(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSRotorHinge::Cleanup(void)
{
    uID                 = "";
    name                = "";
    description         = "";
    transformation.SetIdentity();
    translation         = CTiglPoint(0.0, 0.0, 0.0);
    scaling             = CTiglPoint(1.0, 1.0, 1.0);
    rotation            = CTiglPoint(0.0, 0.0, 0.0);
    type                = TIGLROTORHINGE_UNDEFINED;
    neutralPosition     = 0.;
    staticStiffness     = 0.;
    dynamicStiffness    = 0.;
    damping             = 0.;

    Invalidate();
}

// Build transformation matrix for the hinge
void CCPACSRotorHinge::BuildMatrix(void)
{
    transformation.SetIdentity();

    // scale normalized coordinates relative to (0,0,0)
    transformation.AddScaling(scaling.x, scaling.y, scaling.z);

    // rotate section element around z
    transformation.AddRotationZ(rotation.z);
    // rotate section element by angle of incidence
    transformation.AddRotationY(rotation.y);
    // rotate section element according to wing profile roll
    transformation.AddRotationX(rotation.x);

    // Translate section element to final position relative to section plane
    transformation.AddTranslation(translation.x, translation.y, translation.z);
}

// Update internal rotor blade attachment data
void CCPACSRotorHinge::Update(void)
{
    if (!invalidated) {
        return;
    }

    BuildMatrix();
    invalidated = false;
}

// Invalidates internal state
void CCPACSRotorHinge::Invalidate(void)
{
    invalidated = true;
}

// Read CPACS rotorHinge elements
void CCPACSRotorHinge::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorHingeXPath)
{
    Cleanup();

    std::string tempString;
    double tmpDouble;
    double tmpX, tmpY, tmpZ;

    // Get attribute "uID"
    char* ptrUID  = NULL;
    if (tixiGetTextAttribute(tixiHandle, rotorHingeXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        uID = ptrUID;
    }

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = rotorHingeXPath + "/name";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    // Get subelement "description"
    char* ptrDescription = NULL;
    tempString    = rotorHingeXPath + "/description";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrDescription) == SUCCESS) {
        description = ptrDescription;
    }

    // Get subelement "/transformation/translation"
    tempString  = rotorHingeXPath + "/transformation/translation";
    if (tixiCheckElement(tixiHandle, tempString.c_str()) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, tempString.c_str(), &tmpX, &tmpY, &tmpZ) == SUCCESS) {
            translation.x = tmpX;
            translation.y = tmpY;
            translation.z = tmpZ;
        }
    }

    // Get subelement "/transformation/scaling"
    tempString  = rotorHingeXPath + "/transformation/scaling";
    if (tixiCheckElement(tixiHandle, tempString.c_str()) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, tempString.c_str(), &tmpX, &tmpY, &tmpZ) == SUCCESS) {
            scaling.x = tmpX;
            scaling.y = tmpY;
            scaling.z = tmpZ;
        }
    }

    // Get subelement "/transformation/rotation"
    tempString  = rotorHingeXPath + "/transformation/rotation";
    if (tixiCheckElement(tixiHandle, tempString.c_str()) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, tempString.c_str(), &tmpX, &tmpY, &tmpZ) == SUCCESS) {
            rotation.x = tmpX;
            rotation.y = tmpY;
            rotation.z = tmpZ;
        }
    }

    // Get subelement "type"
    char* ptrType = NULL;
    tempString    = rotorHingeXPath + "/type";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrType) == SUCCESS) {
        std::string strType(ptrType);
        if (strType == "flap") {
            type = TIGLROTORHINGE_FLAP;
        }
        else if (strType == "pitch") {
            type = TIGLROTORHINGE_PITCH;
        }
        else if (strType == "leadLag") {
            type = TIGLROTORHINGE_LEAD_LAG;
        }
        else {
            throw CTiglError("Error: XML error while reading <type> in CCPACSRotorHinge::ReadCPACS: illegal value", TIGL_XML_ERROR);
        }
    }
    else {
        throw CTiglError("Error: XML error while reading <type> in CCPACSRotorHinge::ReadCPACS: unable to read required element", TIGL_XML_ERROR);
    }

    // Get subelement "neutralPosition"
    tempString    = rotorHingeXPath + "/neutralPosition";
    if (tixiCheckElement(tixiHandle, tempString.c_str())== SUCCESS)
        if (tixiGetDoubleElement(tixiHandle, tempString.c_str(), &tmpDouble) == SUCCESS) {
            neutralPosition = tmpDouble;
        }

    // Get subelement "staticStiffness"
    tempString    = rotorHingeXPath + "/staticStiffness";
    if (tixiCheckElement(tixiHandle, tempString.c_str())== SUCCESS)
        if (tixiGetDoubleElement(tixiHandle, tempString.c_str(), &tmpDouble) == SUCCESS) {
            staticStiffness = tmpDouble;
        }

    // Get subelement "dynamicStiffness"
    tempString    = rotorHingeXPath + "/dynamicStiffness";
    if (tixiCheckElement(tixiHandle, tempString.c_str())== SUCCESS)
        if (tixiGetDoubleElement(tixiHandle, tempString.c_str(), &tmpDouble) == SUCCESS) {
            dynamicStiffness = tmpDouble;
        }

    // Get subelement "damping"
    tempString    = rotorHingeXPath + "/damping";
    if (tixiCheckElement(tixiHandle, tempString.c_str())== SUCCESS)
        if (tixiGetDoubleElement(tixiHandle, tempString.c_str(), &tmpDouble) == SUCCESS) {
            damping = tmpDouble;
        }

    Update();
}

// Returns the UID of the rotor blade attachment
const std::string& CCPACSRotorHinge::GetUID(void) const
{
    return uID;
}

// Returns the name of the rotor blade attachment
const std::string& CCPACSRotorHinge::GetName(void) const
{
    return name;
}

// Returns the description of the rotor blade attachment
const std::string& CCPACSRotorHinge::GetDescription(void) const
{
    return description;
}

// Returns the type of the rotor blade attachment
const TiglRotorHingeType& CCPACSRotorHinge::GetType(void) const
{
    return type;
}

// Gets the rotor hinge transformation
const CTiglTransformation& CCPACSRotorHinge::GetTransformation(void) const
{
    return transformation;
}

// Gets the scaling vector of the rotor hinge transformation
const CTiglPoint& CCPACSRotorHinge::GetScaling(void) const
{
    return scaling;
}

// Gets the rotation vector of the rotor hinge transformation
const CTiglPoint& CCPACSRotorHinge::GetRotation(void) const
{
    return rotation;
}

// Gets the translation vector of the rotor hinge transformation
const CTiglPoint& CCPACSRotorHinge::GetTranslation(void) const
{
    return translation;
}

// Gets the rotor hinge neutral position
const double& CCPACSRotorHinge::GetNeutralPosition(void) const
{
    return neutralPosition;
}

// Gets the rotor hinge static stiffness
const double& CCPACSRotorHinge::GetStaticStiffness(void) const
{
    return staticStiffness;
}

// Gets the rotor hinge dynamic stiffness
const double& CCPACSRotorHinge::GetDynamicStiffness(void) const
{
    return dynamicStiffness;
}

// Gets the rotor hinge damping
const double& CCPACSRotorHinge::GetDamping(void) const
{
    return damping;
}

// Gets the rotation angle of a hinge for a given azimuth position (thetaDeg in Degree) and given coefficients (a0, aSin[], aCos[])
double CCPACSRotorHinge::GetHingeAngle(double thetaDeg, double a0, std::vector<double> aSin, std::vector<double> aCos) const
{
    double theta = thetaDeg * M_PI/180.; // convert theta from degree to rad
    double hingeAngle = neutralPosition + a0; // Constant portion
    for (int i=0; i<aSin.size(); ++i) { // Sine coefficients
        hingeAngle += aSin[i] * sin(static_cast<double>(i+1) * theta);
    }
    for (int i=0; i<aCos.size(); ++i) { // Cosine coefficients
        hingeAngle += aCos[i] * cos(static_cast<double>(i+1) * theta);
    }
    return hingeAngle;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorHinge::GetConfiguration(void) const
{
    return rotorBladeAttachment->GetConfiguration();
}

// Returns the parent rotor
CCPACSRotor& CCPACSRotorHinge::GetRotor(void) const
{
    return rotorBladeAttachment->GetRotor();
}

// Returns the parent rotor blade attachment
CCPACSRotorBladeAttachment& CCPACSRotorHinge::GetRotorBladeAttachment(void) const
{
    return *rotorBladeAttachment;
}

} // end namespace tigl
