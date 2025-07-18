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
#include <vector>
#include "tigl_internal.h"
#include "UniquePtr.h"

namespace tigl
{
class CTiglUIDManager;
class CTiglUIDObject;

namespace generated
{
    class CPACSDoorCutOut;
    class CPACSStructuralElements;

    // This class is used in:
    // CPACSStructuralElements

    /// @brief doorsType
    /// 
    /// Doors type, containing doors
    /// 
    class CPACSDoors
    {
    public:
        TIGL_EXPORT CPACSDoors(CPACSStructuralElements* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT virtual ~CPACSDoors();

        TIGL_EXPORT CPACSStructuralElements* GetParent();

        TIGL_EXPORT const CPACSStructuralElements* GetParent() const;

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT CTiglUIDManager& GetUIDManager();
        TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const std::vector<std::unique_ptr<CPACSDoorCutOut>>& GetDoors() const;
        TIGL_EXPORT virtual std::vector<std::unique_ptr<CPACSDoorCutOut>>& GetDoors();

        TIGL_EXPORT virtual size_t GetDoorCount() const;
        TIGL_EXPORT virtual size_t GetDoorIndex(const std::string& UID) const;

        TIGL_EXPORT virtual const CPACSDoorCutOut& GetDoor(size_t index) const;
        TIGL_EXPORT virtual CPACSDoorCutOut& GetDoor(size_t index);

        TIGL_EXPORT virtual const CPACSDoorCutOut& GetDoor(const std::string& UID) const;
        TIGL_EXPORT virtual CPACSDoorCutOut& GetDoor(const std::string& UID);

        TIGL_EXPORT virtual CPACSDoorCutOut& AddDoor();
        TIGL_EXPORT virtual void RemoveDoor(CPACSDoorCutOut& ref);

    protected:
        CPACSStructuralElements* m_parent;

        CTiglUIDManager* m_uidMgr;

        std::vector<std::unique_ptr<CPACSDoorCutOut>> m_doors;

    private:
        CPACSDoors(const CPACSDoors&) = delete;
        CPACSDoors& operator=(const CPACSDoors&) = delete;

        CPACSDoors(CPACSDoors&&) = delete;
        CPACSDoors& operator=(CPACSDoors&&) = delete;
    };
} // namespace generated

// Aliases in tigl namespace
using CCPACSDoors = generated::CPACSDoors;
using CCPACSDoorCutOut = generated::CPACSDoorCutOut;
using CCPACSStructuralElements = generated::CPACSStructuralElements;
} // namespace tigl
