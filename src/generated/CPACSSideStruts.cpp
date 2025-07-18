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
#include "CPACSLandingGearComponentAssembly.h"
#include "CPACSSideStruts.h"
#include "CPACSStrutAssembly.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSSideStruts::CPACSSideStruts(CPACSLandingGearComponentAssembly* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSSideStruts::~CPACSSideStruts()
    {
    }

    const CPACSLandingGearComponentAssembly* CPACSSideStruts::GetParent() const
    {
        return m_parent;
    }

    CPACSLandingGearComponentAssembly* CPACSSideStruts::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSSideStruts::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSSideStruts::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSSideStruts::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSSideStruts::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSSideStruts::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element sideStrut
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/sideStrut")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/sideStrut", m_sideStruts, 1, 2, this, m_uidMgr);
        }

    }

    void CPACSSideStruts::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element sideStrut
        tixi::TixiSaveElements(tixiHandle, xpath + "/sideStrut", m_sideStruts);

    }

    const std::vector<std::unique_ptr<CPACSStrutAssembly>>& CPACSSideStruts::GetSideStruts() const
    {
        return m_sideStruts;
    }

    std::vector<std::unique_ptr<CPACSStrutAssembly>>& CPACSSideStruts::GetSideStruts()
    {
        return m_sideStruts;
    }

    size_t CPACSSideStruts::GetSideStrutCount() const
    {
        return m_sideStruts.size();
    }

    size_t CPACSSideStruts::GetSideStrutIndex(const std::string& UID) const
    {
        for (size_t i=0; i < GetSideStrutCount(); i++) {
            const std::string tmpUID(m_sideStruts[i]->GetUID());
            if (tmpUID == UID) {
                return i+1;
            }
        }
        throw CTiglError("Invalid UID in CPACSSideStruts::GetSideStrutIndex", TIGL_UID_ERROR);
    }

    CPACSStrutAssembly& CPACSSideStruts::GetSideStrut(size_t index)
    {
        if (index < 1 || index > GetSideStrutCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSStrutAssembly>>::GetSideStrut", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_sideStruts[index];
    }

    const CPACSStrutAssembly& CPACSSideStruts::GetSideStrut(size_t index) const
    {
        if (index < 1 || index > GetSideStrutCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSStrutAssembly>>::GetSideStrut", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_sideStruts[index];
    }

    CPACSStrutAssembly& CPACSSideStruts::GetSideStrut(const std::string& UID)
    {
        for (auto& elem : m_sideStruts ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSSideStruts::GetSideStrut. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }

    const CPACSStrutAssembly& CPACSSideStruts::GetSideStrut(const std::string& UID) const
    {
        for (auto& elem : m_sideStruts ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSSideStruts::GetSideStrut. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }


    CPACSStrutAssembly& CPACSSideStruts::AddSideStrut()
    {
        m_sideStruts.push_back(make_unique<CPACSStrutAssembly>(this, m_uidMgr));
        return *m_sideStruts.back();
    }

    void CPACSSideStruts::RemoveSideStrut(CPACSStrutAssembly& ref)
    {
        for (std::size_t i = 0; i < m_sideStruts.size(); i++) {
            if (m_sideStruts[i].get() == &ref) {
                m_sideStruts.erase(m_sideStruts.begin() + i);
                return;
            }
        }
        throw CTiglError("Element not found");
    }

} // namespace generated
} // namespace tigl
