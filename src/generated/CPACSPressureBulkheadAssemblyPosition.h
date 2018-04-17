// Copyright (c) 2018 RISC Software GmbH
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
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDManager;
class CCPACSPressureBulkheadAssembly;

namespace generated
{
    // This class is used in:
    // CPACSPressureBulkheadAssembly

    // generated from /xsd:schema/xsd:complexType[728]
    class CPACSPressureBulkheadAssemblyPosition
    {
    public:
        TIGL_EXPORT CPACSPressureBulkheadAssemblyPosition(CCPACSPressureBulkheadAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT virtual ~CPACSPressureBulkheadAssemblyPosition();

        TIGL_EXPORT CCPACSPressureBulkheadAssembly* GetParent() const;

        TIGL_EXPORT CTiglUIDManager& GetUIDManager();
        TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const std::string& GetUID() const;
        TIGL_EXPORT virtual void SetUID(const std::string& value);

        TIGL_EXPORT virtual const std::string& GetFrameUID() const;
        TIGL_EXPORT virtual void SetFrameUID(const std::string& value);

        TIGL_EXPORT virtual const std::string& GetPressureBulkheadElementUID() const;
        TIGL_EXPORT virtual void SetPressureBulkheadElementUID(const std::string& value);

    protected:
        CCPACSPressureBulkheadAssembly* m_parent;

        CTiglUIDManager* m_uidMgr;

        std::string m_uID;
        std::string m_frameUID;
        std::string m_pressureBulkheadElementUID;

    private:
#ifdef HAVE_CPP11
        CPACSPressureBulkheadAssemblyPosition(const CPACSPressureBulkheadAssemblyPosition&) = delete;
        CPACSPressureBulkheadAssemblyPosition& operator=(const CPACSPressureBulkheadAssemblyPosition&) = delete;

        CPACSPressureBulkheadAssemblyPosition(CPACSPressureBulkheadAssemblyPosition&&) = delete;
        CPACSPressureBulkheadAssemblyPosition& operator=(CPACSPressureBulkheadAssemblyPosition&&) = delete;
#else
        CPACSPressureBulkheadAssemblyPosition(const CPACSPressureBulkheadAssemblyPosition&);
        CPACSPressureBulkheadAssemblyPosition& operator=(const CPACSPressureBulkheadAssemblyPosition&);
#endif
    };
} // namespace generated

// CPACSPressureBulkheadAssemblyPosition is customized, use type CCPACSPressureBulkheadAssemblyPosition directly
} // namespace tigl