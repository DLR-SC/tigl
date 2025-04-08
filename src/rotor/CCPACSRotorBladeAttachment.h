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

#pragma warning( push )
// disable misleading warning to diamond inheritance
#pragma warning( disable : 4250 )

class CCPACSRotorBladeAttachment : private generated::CPACSRotorBladeAttachment, public virtual CTiglUIDObject
{

public:
    TIGL_EXPORT CCPACSRotorBladeAttachment(CCPACSRotorBladeAttachments* parent, CTiglUIDManager* uidMgr);

    using generated::CPACSRotorBladeAttachment::GetParent;

    using generated::CPACSRotorBladeAttachment::ReadCPACS;
    using generated::CPACSRotorBladeAttachment::WriteCPACS;

    using generated::CPACSRotorBladeAttachment::GetUID;
    using generated::CPACSRotorBladeAttachment::SetUID;
    using generated::CPACSRotorBladeAttachment::GetNextUIDParent;

    using generated::CPACSRotorBladeAttachment::GetName;
    using generated::CPACSRotorBladeAttachment::SetName;

    using generated::CPACSRotorBladeAttachment::GetDescription;
    using generated::CPACSRotorBladeAttachment::SetDescription;

    using generated::CPACSRotorBladeAttachment::GetHinges;

    using generated::CPACSRotorBladeAttachment::GetRotorBladeUID;
    using generated::CPACSRotorBladeAttachment::SetRotorBladeUID;

    // Builds and returns the transformation matrix for an attached rotor blade
    TIGL_EXPORT CTiglTransformation GetRotorBladeTransformationMatrix(double thetaDeg=0., double bladeDeltaThetaDeg=0., bool doHingeTransformation=true, bool doRotationDirTransformation=true, bool doRotorTransformation=false);

    // Returns the number of attached rotor blades
    TIGL_EXPORT size_t GetNumberOfBlades() const;

    // Returns the azimuth angle of the attached rotor blade with the given index
    TIGL_EXPORT double GetAzimuthAngle(size_t index) const;

    // Returns the index of the referenced wing definition
    TIGL_EXPORT size_t GetWingIndex() const;

    // Get hinge count
    TIGL_EXPORT size_t GetHingeCount() const;

    // Returns the hinge for a given index
    TIGL_EXPORT const CCPACSRotorHinge& GetHinge(size_t index) const;
    TIGL_EXPORT CCPACSRotorHinge& GetHinge(size_t index);

    // Returns the rotor blade for a given index
    TIGL_EXPORT const CTiglAttachedRotorBlade& GetAttachedRotorBlade(size_t index) const;
    TIGL_EXPORT CTiglAttachedRotorBlade& GetAttachedRotorBlade(size_t index);

    // Returns the parent rotor
    TIGL_EXPORT const CCPACSRotor& GetRotor() const;
    TIGL_EXPORT CCPACSRotor& GetRotor();

    // Returns the parent configuration
    TIGL_EXPORT const CCPACSConfiguration& GetConfiguration() const;
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration();

private:
    // Invalidates internal state
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

    void lazyCreateAttachedRotorBlades() const;

    mutable std::vector<std::unique_ptr<CTiglAttachedRotorBlade>> attachedRotorBlades;
};

#pragma warning( pop )

} // end namespace tigl

#endif // CCPACSROTORBLADEATTACHMENT_H
