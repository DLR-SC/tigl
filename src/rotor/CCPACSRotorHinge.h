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

#ifndef CCPACSROTORHINGE_H
#define CCPACSROTORHINGE_H

#include <vector>

#include "generated/CPACSRotorHubHinge.h"
#include "CTiglPoint.h"
#include "CTiglTransformation.h"


namespace tigl
{

enum TiglRotorHingeType
{
    TIGLROTORHINGE_UNDEFINED,
    TIGLROTORHINGE_FLAP,
    TIGLROTORHINGE_PITCH,
    TIGLROTORHINGE_LEAD_LAG
};

class CCPACSConfiguration;
class CCPACSRotor;
class CCPACSRotorBladeAttachment;

class CCPACSRotorHinge : public generated::CPACSRotorHubHinge
{

public:
    // Constructor
    TIGL_EXPORT CCPACSRotorHinge(CCPACSRotorHinges* parent);

    // Returns the rotor hinge transformation
    TIGL_EXPORT CTiglTransformation GetTransformation() const;

    // Returns the scaling vector of the rotor hinge transformation
    TIGL_EXPORT CTiglPoint GetScaling() const;

    // Returns the rotation vector of the rotor hinge transformation
    TIGL_EXPORT CTiglPoint GetRotation() const;

    // Returns the translation vector of the rotor hinge transformation
    TIGL_EXPORT CTiglPoint GetTranslation() const;

    // Returns the rotor hinge angle
    TIGL_EXPORT double GetHingeAngle(double thetaDeg=0., double a0=0., std::vector<double> aSin=std::vector<double>(), std::vector<double> aCos=std::vector<double>()) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor() const;

    // Returns the parent rotor blade attachment
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment() const;
};

} // end namespace tigl

#endif // CCPACSROTORHINGE_H
