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

#include <cassert>
#include <CCPACSPressureBulkheadAssemblyPosition.h>
#include "CCPACSFuselageStructure.h"
#include "CPACSPressureBulkheadAssembly.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSPressureBulkheadAssembly::CPACSPressureBulkheadAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSPressureBulkheadAssembly::~CPACSPressureBulkheadAssembly()
    {
    }

    const CCPACSFuselageStructure* CPACSPressureBulkheadAssembly::GetParent() const
    {
        return m_parent;
    }

    CCPACSFuselageStructure* CPACSPressureBulkheadAssembly::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSPressureBulkheadAssembly::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSPressureBulkheadAssembly::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSPressureBulkheadAssembly::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSPressureBulkheadAssembly::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSPressureBulkheadAssembly::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element pressureBulkhead
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/pressureBulkhead")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/pressureBulkhead", m_pressureBulkheads, 1, tixi::xsdUnbounded, reinterpret_cast<CCPACSPressureBulkheadAssembly*>(this), m_uidMgr);
        }

    }

    void CPACSPressureBulkheadAssembly::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element pressureBulkhead
        tixi::TixiSaveElements(tixiHandle, xpath + "/pressureBulkhead", m_pressureBulkheads);

    }

    const std::vector<std::unique_ptr<CCPACSPressureBulkheadAssemblyPosition>>& CPACSPressureBulkheadAssembly::GetPressureBulkheads() const
    {
        return m_pressureBulkheads;
    }

    std::vector<std::unique_ptr<CCPACSPressureBulkheadAssemblyPosition>>& CPACSPressureBulkheadAssembly::GetPressureBulkheads()
    {
        return m_pressureBulkheads;
    }

    size_t CPACSPressureBulkheadAssembly::GetPressureBulkheadCount() const
    {
        return m_pressureBulkheads.size();
    }

    size_t CPACSPressureBulkheadAssembly::GetPressureBulkheadIndex(const std::string& UID) const
    {
        for (size_t i=0; i < GetPressureBulkheadCount(); i++) {
            const std::string tmpUID(m_pressureBulkheads[i]->GetUID());
            if (tmpUID == UID) {
                return i+1;
            }
        }
        throw CTiglError("Invalid UID in CPACSPressureBulkheadAssembly::GetPressureBulkheadIndex", TIGL_UID_ERROR);
    }

    CCPACSPressureBulkheadAssemblyPosition& CPACSPressureBulkheadAssembly::GetPressureBulkhead(size_t index)
    {
        if (index < 1 || index > GetPressureBulkheadCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CCPACSPressureBulkheadAssemblyPosition>>::GetPressureBulkhead", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_pressureBulkheads[index];
    }

    const CCPACSPressureBulkheadAssemblyPosition& CPACSPressureBulkheadAssembly::GetPressureBulkhead(size_t index) const
    {
        if (index < 1 || index > GetPressureBulkheadCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CCPACSPressureBulkheadAssemblyPosition>>::GetPressureBulkhead", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_pressureBulkheads[index];
    }

    CCPACSPressureBulkheadAssemblyPosition& CPACSPressureBulkheadAssembly::GetPressureBulkhead(const std::string& UID)
    {
        for (auto& elem : m_pressureBulkheads ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSPressureBulkheadAssembly::GetPressureBulkhead. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }

    const CCPACSPressureBulkheadAssemblyPosition& CPACSPressureBulkheadAssembly::GetPressureBulkhead(const std::string& UID) const
    {
        for (auto& elem : m_pressureBulkheads ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSPressureBulkheadAssembly::GetPressureBulkhead. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }


    CCPACSPressureBulkheadAssemblyPosition& CPACSPressureBulkheadAssembly::AddPressureBulkhead()
    {
        m_pressureBulkheads.push_back(make_unique<CCPACSPressureBulkheadAssemblyPosition>(reinterpret_cast<CCPACSPressureBulkheadAssembly*>(this), m_uidMgr));
        return *m_pressureBulkheads.back();
    }

    void CPACSPressureBulkheadAssembly::RemovePressureBulkhead(CCPACSPressureBulkheadAssemblyPosition& ref)
    {
        for (std::size_t i = 0; i < m_pressureBulkheads.size(); i++) {
            if (m_pressureBulkheads[i].get() == &ref) {
                m_pressureBulkheads.erase(m_pressureBulkheads.begin() + i);
                return;
            }
        }
        throw CTiglError("Element not found");
    }

} // namespace generated
} // namespace tigl
