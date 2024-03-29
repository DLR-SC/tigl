// Copyright (c) 2020 RISC Software GmbH
//
// This file was generated by CPACSGen from CPACS XML Schema (c) German Aerospace Center (DLR/SC).
// Do not edit, all changes are lost when files are re-generated.
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <CCPACSStringVector.h>
#include <string>
#include <tixi.h>
#include "CPACSTrackJointCoordinates.h"
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDObject;

namespace generated
{
    class CPACSTrackJointPositions;

    // This class is used in:
    // CPACSTrackJointPositions

    /// @brief Set of joint coordinates
    /// 
    /// Definition of a set of joint coordinates.
    /// 
    class CPACSTrackJointPosition
    {
    public:
        TIGL_EXPORT CPACSTrackJointPosition(CPACSTrackJointPositions* parent);

        TIGL_EXPORT virtual ~CPACSTrackJointPosition();

        TIGL_EXPORT CPACSTrackJointPositions* GetParent();

        TIGL_EXPORT const CPACSTrackJointPositions* GetParent() const;

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const CCPACSStringVector& GetControlParameters() const;
        TIGL_EXPORT virtual CCPACSStringVector& GetControlParameters();

        TIGL_EXPORT virtual const CPACSTrackJointCoordinates& GetJointCoordinates() const;
        TIGL_EXPORT virtual CPACSTrackJointCoordinates& GetJointCoordinates();

    protected:
        CPACSTrackJointPositions* m_parent;

        /// Value of the command parameter of a control distributor. If not given explicitly in the control distributor, linear interpolation between the neighboring points is required.
        CCPACSStringVector         m_controlParameters;

        CPACSTrackJointCoordinates m_jointCoordinates;

    private:
        CPACSTrackJointPosition(const CPACSTrackJointPosition&) = delete;
        CPACSTrackJointPosition& operator=(const CPACSTrackJointPosition&) = delete;

        CPACSTrackJointPosition(CPACSTrackJointPosition&&) = delete;
        CPACSTrackJointPosition& operator=(CPACSTrackJointPosition&&) = delete;
    };
} // namespace generated

// Aliases in tigl namespace
using CCPACSTrackJointPosition = generated::CPACSTrackJointPosition;
using CCPACSTrackJointPositions = generated::CPACSTrackJointPositions;
} // namespace tigl
