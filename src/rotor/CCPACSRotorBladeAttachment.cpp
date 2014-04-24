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
* @brief  Implementation of CPACS rotor blade attachment handling routines.
*/

#include "CCPACSRotorBladeAttachment.h"
#include "CCPACSRotor.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"

namespace tigl
{

// Constructor
CCPACSRotorBladeAttachment::CCPACSRotorBladeAttachment(CCPACSRotor* rotor)
    : rotor(rotor)
    , hinges(this)
    , rotorBlades(this)
    , azimuthAngles()
{
    Cleanup();
}

// Destructor
CCPACSRotorBladeAttachment::~CCPACSRotorBladeAttachment(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSRotorBladeAttachment::Cleanup(void)
{
    uID  = "";
    rotorBladeUID = "";
    //hinges.Cleanup();
    //rotorBlades.Cleanup();
    azimuthAngles.clear();
}

// Update internal rotor blade attachment data
void CCPACSRotorBladeAttachment::Update(void)
{
    if (!invalidated) {
        return;
    }

    invalidated = false;
}

// Invalidates internal state
void CCPACSRotorBladeAttachment::Invalidate(void)
{
    invalidated = true;
    for (int i = 1; i <= GetHingeCount(); i++) {
        GetHinge(i).Invalidate();
    }
    for (int i = 1; i <= GetRotorBladeCount(); i++) {
        GetRotorBlade(i).Invalidate();
    }
}

// Read CPACS rotor blade attachment elements
void CCPACSRotorBladeAttachment::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorBladeAttachmentXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get attribute "uID"
    char* ptrUID = NULL;
    tempString   = rotorBladeAttachmentXPath;
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, elementPath, "uID", &ptrUID) == SUCCESS) {
        uID = ptrUID;
    }

    // Get subelement "numberOfBlades"
    int numberOfBlades = 0;
    tempString  = rotorBladeAttachmentXPath + "/numberOfBlades";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS &&
            tixiGetIntegerElement(tixiHandle, elementPath, &numberOfBlades) == SUCCESS) {
        for (int i=0; i<numberOfBlades; ++i) {
            azimuthAngles.push_back(static_cast<double>(i)*360./static_cast<double>(numberOfBlades));
        }
    }

    // Get subelement "azimuthAngles"
    tempString = rotorBladeAttachmentXPath + "/azimuthAngles";
    elementPath = const_cast<char*>(tempString.c_str());
    int countAzimuthAngles;
    double* ptrAzimuthAngles = NULL;
    if (tixiGetVectorSize(tixiHandle, elementPath, &countAzimuthAngles) == SUCCESS) {
        if (tixiGetFloatVector(tixiHandle, elementPath, &ptrAzimuthAngles, countAzimuthAngles) == SUCCESS) {
            numberOfBlades = countAzimuthAngles;
            azimuthAngles.clear();
            for (int i=0; i<numberOfBlades; ++i) {
                azimuthAngles.push_back(ptrAzimuthAngles[i]);
            }
        }
        else {
            throw CTiglError("Error: XML error while reading azimuth angles vector <azimuthAngles> in CCPACSRotorBladeAttachment::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "rotorBladeUID"
    char* ptrRotorBladeUID = NULL;
    tempString    = rotorBladeAttachmentXPath + "/rotorBladeUID";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrRotorBladeUID) == SUCCESS) {
        rotorBladeUID = ptrRotorBladeUID;
    }


    // Get subelement "hinges"
    hinges.ReadCPACS(tixiHandle, rotorBladeAttachmentXPath + "/hinges", "hinge");

    // Create rotor blades
    CCPACSWing* ptrToUnattachedRotorBlade = &(GetConfiguration().GetWing(rotorBladeUID));
    for (int i=0; i<azimuthAngles.size(); ++i) {
        rotorBlades.AddRotorBlade(new CCPACSRotorBlade(this, ptrToUnattachedRotorBlade, i+1));
    }

    Update();
}

// Builds and returns the transformation matrix for an attached rotor blade
//     rotorThetaDeg:         current azimuthal position of the rotor in degrees
//     bladeDeltaThetaDeg:    azimuth angle offset of the attached blade
//     doRotorTransformation: include the parent rotor transformation?
CTiglTransformation CCPACSRotorBladeAttachment::GetRotorBladeTransformationMatrix(double rotorThetaDeg, double bladeDeltaThetaDeg, bool doHingeTransformation, bool doRotationDirTransformation, bool doRotorTransformation)
{
    double bladeThetaDeg = rotorThetaDeg + bladeDeltaThetaDeg; // current azimuthal position of the rotor blade in degrees

    // Rotor blade transformation chain:
    CTiglTransformation rotorBladeTransformation;

    // 0. Initialize matrix
    rotorBladeTransformation.SetIdentity();

    // 1. Rotation around hinges, beginning with the last
    if (doHingeTransformation) {
        for (int k=GetHingeCount()-1; k>=0; --k) {
            CTiglPoint curHingePosition = GetHinge(k+1).GetTranslation();
            TiglRotorHingeType curHingeType = GetHinge(k+1).GetType();
            // a. move to origin
            rotorBladeTransformation.AddTranslation(-curHingePosition.x, -curHingePosition.y, -curHingePosition.z);
            // b. rotate around hinge axis
            if (curHingeType == TIGLROTORHINGE_PITCH) {
                rotorBladeTransformation.AddRotationX( (GetHinge(k+1).GetHingeAngle(bladeThetaDeg)));
            }
            else if (curHingeType == TIGLROTORHINGE_FLAP) {
                rotorBladeTransformation.AddRotationY(-(GetHinge(k+1).GetHingeAngle(bladeThetaDeg)));
            }
            else if (curHingeType == TIGLROTORHINGE_LEAD_LAG) {
                rotorBladeTransformation.AddRotationZ( (GetHinge(k+1).GetHingeAngle(bladeThetaDeg)));
            }
            // c. move back to origin
            rotorBladeTransformation.AddTranslation(curHingePosition.x, curHingePosition.y, curHingePosition.z);
            //TODO: account for rotation and scaling of hinge?
        }
    }
    // 2. If the rotation direction is clockwise (e.g. french rotor): mirror rotor blade in x direction
    if (doRotationDirTransformation) {
        if (rotor->GetNominalRotationsPerMinute() < 0. ) {
            rotorBladeTransformation.AddMirroringAtYZPlane();
            bladeThetaDeg *= -1.;
        }
    }
    // 3. Rotate the rotor blade around z to its azimuth position
    rotorBladeTransformation.AddRotationZ(bladeThetaDeg);

    // 4. Add rotor transformation if desired
    if (doRotorTransformation) {
        rotorBladeTransformation.PreMultiply(rotor->GetTransformation());
    }

    return rotorBladeTransformation;
}

// Returns the UID of the rotor blade attachment
const std::string& CCPACSRotorBladeAttachment::GetUID(void) const
{
    return uID;
}

// Returns the number of attached rotor blades
int CCPACSRotorBladeAttachment::GetNumberOfBlades(void) const
{
    return static_cast<int>(azimuthAngles.size());
}

// Returns the azimuth angle of the attached rotor blade with the given index
const double& CCPACSRotorBladeAttachment::GetAzimuthAngle(int index) const
{
    index --;
    if (index < 0 || index >= azimuthAngles.size()) {
        throw CTiglError("Error: Invalid index in CCPACSRotorBladeAttachment::GetAzimuthAngle", TIGL_INDEX_ERROR);
    }
    return (azimuthAngles[index]);
}

// Returns the UID of the referenced wing definition
const std::string& CCPACSRotorBladeAttachment::GetWingUID(void) const
{
    return rotorBladeUID;
}

// Returns the index of the referenced wing definition
int CCPACSRotorBladeAttachment::GetWingIndex(void) const
{
    return GetConfiguration().GetWingIndex(rotorBladeUID);
}

// Get hinge count
int CCPACSRotorBladeAttachment::GetHingeCount(void) const
{
    return hinges.GetRotorHingeCount();
}

// Returns the hinge for a given index
CCPACSRotorHinge& CCPACSRotorBladeAttachment::GetHinge(const int index) const
{
    return hinges.GetRotorHinge(index);
}

// Get rotor blade count
int CCPACSRotorBladeAttachment::GetRotorBladeCount(void) const
{
    return rotorBlades.GetRotorBladeCount();
}

// Returns the rotor blade for a given index
CCPACSRotorBlade& CCPACSRotorBladeAttachment::GetRotorBlade(const int index) const
{
    return rotorBlades.GetRotorBlade(index);
}

// Returns the parent rotor
CCPACSRotor& CCPACSRotorBladeAttachment::GetRotor(void) const
{
    return *rotor;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorBladeAttachment::GetConfiguration(void) const
{
    return rotor->GetConfiguration();
}

} // end namespace tigl
