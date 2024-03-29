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

#include <string>
#include <tixi.h>
#include "CPACSLandingGearSupportBeamPosition.h"
#include "CPACSSparCrossSection.h"
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDManager;
class CTiglUIDObject;

namespace generated
{
    class CPACSLandingGearStrutAttachment;

    // This class is used in:
    // CPACSLandingGearStrutAttachment

    /// @brief Main landing gear support beam
    /// 
    /// Definition of the main landing gear support beam, if a
    /// support beam is used for the attachment. The defintion includes
    /// cross section properties as well as the position of the support
    /// beam.
    /// 
    class CPACSSupportBeam
    {
    public:
        TIGL_EXPORT CPACSSupportBeam(CPACSLandingGearStrutAttachment* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT virtual ~CPACSSupportBeam();

        TIGL_EXPORT CPACSLandingGearStrutAttachment* GetParent();

        TIGL_EXPORT const CPACSLandingGearStrutAttachment* GetParent() const;

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT CTiglUIDManager& GetUIDManager();
        TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const CPACSLandingGearSupportBeamPosition& GetPosition() const;
        TIGL_EXPORT virtual CPACSLandingGearSupportBeamPosition& GetPosition();

        TIGL_EXPORT virtual const CPACSSparCrossSection& GetStructure() const;
        TIGL_EXPORT virtual CPACSSparCrossSection& GetStructure();

    protected:
        CPACSLandingGearStrutAttachment* m_parent;

        CTiglUIDManager* m_uidMgr;

        CPACSLandingGearSupportBeamPosition m_position;
        CPACSSparCrossSection               m_structure;

    private:
        CPACSSupportBeam(const CPACSSupportBeam&) = delete;
        CPACSSupportBeam& operator=(const CPACSSupportBeam&) = delete;

        CPACSSupportBeam(CPACSSupportBeam&&) = delete;
        CPACSSupportBeam& operator=(CPACSSupportBeam&&) = delete;
    };
} // namespace generated

// Aliases in tigl namespace
using CCPACSSupportBeam = generated::CPACSSupportBeam;
using CCPACSLandingGearStrutAttachment = generated::CPACSLandingGearStrutAttachment;
} // namespace tigl
