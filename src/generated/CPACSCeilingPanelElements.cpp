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
#include "CPACSCeilingPanelElements.h"
#include "CPACSDeckElementBase.h"
#include "CPACSDeckElements.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSCeilingPanelElements::CPACSCeilingPanelElements(CPACSDeckElements* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSCeilingPanelElements::~CPACSCeilingPanelElements()
    {
    }

    const CPACSDeckElements* CPACSCeilingPanelElements::GetParent() const
    {
        return m_parent;
    }

    CPACSDeckElements* CPACSCeilingPanelElements::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSCeilingPanelElements::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSCeilingPanelElements::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSCeilingPanelElements::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSCeilingPanelElements::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSCeilingPanelElements::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element ceilingPanelElement
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/ceilingPanelElement")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/ceilingPanelElement", m_ceilingPanelElements, 1, tixi::xsdUnbounded, this, m_uidMgr);
        }

    }

    void CPACSCeilingPanelElements::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element ceilingPanelElement
        tixi::TixiSaveElements(tixiHandle, xpath + "/ceilingPanelElement", m_ceilingPanelElements);

    }

    const std::vector<std::unique_ptr<CPACSDeckElementBase>>& CPACSCeilingPanelElements::GetCeilingPanelElements() const
    {
        return m_ceilingPanelElements;
    }

    std::vector<std::unique_ptr<CPACSDeckElementBase>>& CPACSCeilingPanelElements::GetCeilingPanelElements()
    {
        return m_ceilingPanelElements;
    }

    size_t CPACSCeilingPanelElements::GetCeilingPanelElementCount() const
    {
        return m_ceilingPanelElements.size();
    }

    size_t CPACSCeilingPanelElements::GetCeilingPanelElementIndex(const std::string& UID) const
    {
        for (size_t i=0; i < GetCeilingPanelElementCount(); i++) {
            const std::string tmpUID(m_ceilingPanelElements[i]->GetUID());
            if (tmpUID == UID) {
                return i+1;
            }
        }
        throw CTiglError("Invalid UID in CPACSCeilingPanelElements::GetCeilingPanelElementIndex", TIGL_UID_ERROR);
    }

    CPACSDeckElementBase& CPACSCeilingPanelElements::GetCeilingPanelElement(size_t index)
    {
        if (index < 1 || index > GetCeilingPanelElementCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSDeckElementBase>>::GetCeilingPanelElement", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_ceilingPanelElements[index];
    }

    const CPACSDeckElementBase& CPACSCeilingPanelElements::GetCeilingPanelElement(size_t index) const
    {
        if (index < 1 || index > GetCeilingPanelElementCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSDeckElementBase>>::GetCeilingPanelElement", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_ceilingPanelElements[index];
    }

    CPACSDeckElementBase& CPACSCeilingPanelElements::GetCeilingPanelElement(const std::string& UID)
    {
        for (auto& elem : m_ceilingPanelElements ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSCeilingPanelElements::GetCeilingPanelElement. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }

    const CPACSDeckElementBase& CPACSCeilingPanelElements::GetCeilingPanelElement(const std::string& UID) const
    {
        for (auto& elem : m_ceilingPanelElements ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSCeilingPanelElements::GetCeilingPanelElement. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }


    CPACSDeckElementBase& CPACSCeilingPanelElements::AddCeilingPanelElement()
    {
        m_ceilingPanelElements.push_back(make_unique<CPACSDeckElementBase>(this, m_uidMgr));
        return *m_ceilingPanelElements.back();
    }

    void CPACSCeilingPanelElements::RemoveCeilingPanelElement(CPACSDeckElementBase& ref)
    {
        for (std::size_t i = 0; i < m_ceilingPanelElements.size(); i++) {
            if (m_ceilingPanelElements[i].get() == &ref) {
                m_ceilingPanelElements.erase(m_ceilingPanelElements.begin() + i);
                return;
            }
        }
        throw CTiglError("Element not found");
    }

} // namespace generated
} // namespace tigl
