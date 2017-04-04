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
CCPACSRotorHinge::CCPACSRotorHinge(CCPACSRotorHinges* parent)
    : generated::CPACSRotorHubHinge(parent) {}

// Gets the rotor hinge transformation
CTiglTransformation CCPACSRotorHinge::GetTransformationMatrix() const
{
    return m_transformation.getTransformationMatrix();
}

// Gets the scaling vector of the rotor hinge transformation
CTiglPoint CCPACSRotorHinge::GetScaling() const
{
    return m_transformation.getScaling();
}

// Gets the rotation vector of the rotor hinge transformation
CTiglPoint CCPACSRotorHinge::GetRotation() const
{
    return m_transformation.getRotation();
}

// Gets the translation vector of the rotor hinge transformation
CTiglPoint CCPACSRotorHinge::GetTranslation() const
{
    return m_transformation.getTranslationVector();
}

// Gets the rotation angle of a hinge for a given azimuth position (thetaDeg in Degree) and given coefficients (a0, aSin[], aCos[])
double CCPACSRotorHinge::GetHingeAngle(double thetaDeg, double a0, std::vector<double> aSin, std::vector<double> aCos) const
{
    double theta = thetaDeg * M_PI/180.; // convert theta from degree to rad
    double hingeAngle = (m_neutralPosition ? *m_neutralPosition : 0.0) + a0; // Constant portion
    for (unsigned int i = 0; i < aSin.size(); ++i) { // Sine coefficients
        hingeAngle += aSin[i] * std::sin(static_cast<double>(i+1) * theta);
    }
    for (unsigned int i = 0;  i < aCos.size(); ++i) { // Cosine coefficients
        hingeAngle += aCos[i] * std::cos(static_cast<double>(i+1) * theta);
    }
    return hingeAngle;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotorHinge::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

// Returns the parent rotor
CCPACSRotor& CCPACSRotorHinge::GetRotor() const
{
    return m_parent->GetRotor();
}

// Returns the parent rotor blade attachment
CCPACSRotorBladeAttachment& CCPACSRotorHinge::GetRotorBladeAttachment() const
{
    return *m_parent->GetParent();
}

} // end namespace tigl
