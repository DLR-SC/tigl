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

#ifndef CCPACSROTORBLADEATTACHMENT_H
#define CCPACSROTORBLADEATTACHMENT_H

#include "generated/CPACSRotorBladeAttachment.h"
#include "CTiglAttachedRotorBlade.h"

namespace tigl
{

class CCPACSConfiguration;
class CCPACSRotor;

class CCPACSRotorBladeAttachment : private generated::CPACSRotorBladeAttachment
{

public:
    CCPACSRotorBladeAttachment(CCPACSRotorBladeAttachments* parent);

    using generated::CPACSRotorBladeAttachment::GetParent;

    using generated::CPACSRotorBladeAttachment::ReadCPACS;
    using generated::CPACSRotorBladeAttachment::WriteCPACS;

    using generated::CPACSRotorBladeAttachment::HasUID;
    using generated::CPACSRotorBladeAttachment::GetUID;
    using generated::CPACSRotorBladeAttachment::SetUID;

    using generated::CPACSRotorBladeAttachment::HasName;
    using generated::CPACSRotorBladeAttachment::GetName;
    using generated::CPACSRotorBladeAttachment::SetName;

    using generated::CPACSRotorBladeAttachment::HasDescription;
    using generated::CPACSRotorBladeAttachment::GetDescription;
    using generated::CPACSRotorBladeAttachment::SetDescription;

    using generated::CPACSRotorBladeAttachment::HasHinges;
    using generated::CPACSRotorBladeAttachment::GetHinges;

    using generated::CPACSRotorBladeAttachment::GetRotorBladeUID;
    using generated::CPACSRotorBladeAttachment::SetRotorBladeUID;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS rotor elements
    TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& rotorBladeAttachmentXPath) override;

    // Builds and returns the transformation matrix for an attached rotor blade
    TIGL_EXPORT CTiglTransformation GetRotorBladeTransformationMatrix(double thetaDeg=0., double bladeDeltaThetaDeg=0., bool doHingeTransformation=true, bool doRotationDirTransformation=true, bool doRotorTransformation=false);

    // Returns the number of attached rotor blades
    TIGL_EXPORT int GetNumberOfBlades(void) const;

    // Returns the azimuth angle of the attached rotor blade with the given index
    TIGL_EXPORT const double& GetAzimuthAngle(int index) const;

    // Returns the index of the referenced wing definition
    TIGL_EXPORT int GetWingIndex(void) const;

    // Get hinge count
    TIGL_EXPORT int GetHingeCount(void) const;

    // Returns the hinge for a given index
    TIGL_EXPORT CCPACSRotorHinge& GetHinge(int index) const;

    // Returns the rotor blade for a given index
    TIGL_EXPORT CTiglAttachedRotorBlade& GetAttachedRotorBlade(int index);
    TIGL_EXPORT const CTiglAttachedRotorBlade& GetAttachedRotorBlade(int index) const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor(void) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;

private:
	void lazyCreateAttachedRotorBlades();

    std::vector<unique_ptr<CTiglAttachedRotorBlade>> attachedRotorBlades;
    std::vector<double>                              cachedAzimuthAngles;
    bool                                             invalidated;              /**< Internal state flag  */
};

} // end namespace tigl

#endif // CCPACSROTORBLADEATTACHMENT_H
