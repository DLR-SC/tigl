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
CCPACSRotorBladeAttachment::CCPACSRotorBladeAttachment(CCPACSRotorBladeAttachments * parent)
    : generated::CPACSRotorBladeAttachment(parent) {}


// Invalidates internal state
void CCPACSRotorBladeAttachment::Invalidate(void)
{
    invalidated = true;
    for (int i = 0; i < attachedRotorBlades.size(); i++) {
        attachedRotorBlades[i]->Invalidate();
    }
}

// Read CPACS rotor blade attachment elements
void CCPACSRotorBladeAttachment::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& rotorBladeAttachmentXPath)
{
    generated::CPACSRotorBladeAttachment::ReadCPACS(tixiHandle, rotorBladeAttachmentXPath);

    if (m_numberOfBlades_choice2) {
        for (int i = 0; i < *m_numberOfBlades_choice2; i++) {
            cachedAzimuthAngles.push_back(i * 360.0 / static_cast<double>(*m_numberOfBlades_choice2));
        }
    }

    // Create wrappers for attached rotor blades
    CCPACSWing& blade = GetConfiguration().GetWing(m_rotorBladeUID);
    for (int i = 1; i <= GetNumberOfBlades(); i++) {
        attachedRotorBlades.push_back(make_unique<CTiglAttachedRotorBlade>(this, blade, i));
    }
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
            const generated::CPACSRotorHubHinge_type& curHingeType = GetHinge(k+1).GetType();
            // a. move to origin
            rotorBladeTransformation.AddTranslation(-curHingePosition.x, -curHingePosition.y, -curHingePosition.z);
            // b. rotate around hinge axis
            if (curHingeType == generated::CPACSRotorHubHinge_type::pitch) {
                rotorBladeTransformation.AddRotationX( (GetHinge(k+1).GetHingeAngle(bladeThetaDeg)));
            }
            else if (curHingeType == generated::CPACSRotorHubHinge_type::flap) {
                rotorBladeTransformation.AddRotationY(-(GetHinge(k+1).GetHingeAngle(bladeThetaDeg)));
            }
            else if (curHingeType == generated::CPACSRotorHubHinge_type::leadLag) {
                rotorBladeTransformation.AddRotationZ( (GetHinge(k+1).GetHingeAngle(bladeThetaDeg)));
            }
            // c. move back to origin
            rotorBladeTransformation.AddTranslation(curHingePosition.x, curHingePosition.y, curHingePosition.z);
            //TODO: account for rotation and scaling of hinge?
        }
    }
    // 2. If the rotation direction is clockwise (e.g. french rotor): mirror rotor blade in x direction
    if (doRotationDirTransformation) {
        if (GetRotor().GetNominalRotationsPerMinute() < 0. ) {
            rotorBladeTransformation.AddMirroringAtYZPlane();
            bladeThetaDeg *= -1.;
        }
    }
    // 3. Rotate the rotor blade around z to its azimuth position
    rotorBladeTransformation.AddRotationZ(bladeThetaDeg);

    // 4. Add rotor transformation if desired
    if (doRotorTransformation) {
        rotorBladeTransformation.PreMultiply(GetRotor().GetTransformation());
    }

    return rotorBladeTransformation;
}

// Returns the number of attached rotor blades
int CCPACSRotorBladeAttachment::GetNumberOfBlades(void) const
{
    if (m_numberOfBlades_choice2)
        return *m_numberOfBlades_choice2;
    else
        return static_cast<int>(m_azimuthAngles_choice1->AsVector().size());
}

// Returns the azimuth angle of the attached rotor blade with the given index
const double& CCPACSRotorBladeAttachment::GetAzimuthAngle(int index) const
{
    index --;
    if (index < 0 || index >= GetNumberOfBlades()) {
        throw CTiglError("Error: Invalid index in CCPACSRotorBladeAttachment::GetAzimuthAngle", TIGL_INDEX_ERROR);
    }
    if (m_azimuthAngles_choice1)
        return m_azimuthAngles_choice1->AsVector()[index];
    else
        return cachedAzimuthAngles[index];
}

// Returns the index of the referenced wing definition
int CCPACSRotorBladeAttachment::GetWingIndex(void) const
{
    return GetConfiguration().GetWingIndex(m_rotorBladeUID);
}

// Get hinge count
int CCPACSRotorBladeAttachment::GetHingeCount(void) const
{
    if (m_hinges)
        return m_hinges->GetRotorHingeCount();
    else
        return 0;
}

// Returns the hinge for a given index
CCPACSRotorHinge& CCPACSRotorBladeAttachment::GetHinge(const int index) const
{
    return m_hinges->GetRotorHinge(index);
}

// Returns the rotor blade for a given index
CTiglAttachedRotorBlade& CCPACSRotorBladeAttachment::GetAttachedRotorBlade(int index)
{
    index--;
    if (index < 0 || index >= GetNumberOfBlades()) {
        throw CTiglError("Error: Invalid index in CCPACSRotorBladeAttachment::GetAttachedRotorBlade", TIGL_INDEX_ERROR);
    }
    return *attachedRotorBlades[index];
}

// Returns the rotor blade for a given index
const CTiglAttachedRotorBlade& CCPACSRotorBladeAttachment::GetAttachedRotorBlade(int index) const {
    return const_cast<CCPACSRotorBladeAttachment*>(this)->GetAttachedRotorBlade(index);
}

// Returns the parent rotor
CCPACSRotor& CCPACSRotorBladeAttachment::GetRotor(void) const
{
    return *m_parent->GetParent()->GetParent();
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorBladeAttachment::GetConfiguration(void) const
{
    return m_parent->GetConfiguration();
}

} // end namespace tigl
