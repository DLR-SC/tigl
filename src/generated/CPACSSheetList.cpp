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
#include "CPACSSheet.h"
#include "CPACSSheetList.h"
#include "CPACSStructuralProfile.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSSheetList::CPACSSheetList(CPACSStructuralProfile* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSSheetList::~CPACSSheetList()
    {
    }

    const CPACSStructuralProfile* CPACSSheetList::GetParent() const
    {
        return m_parent;
    }

    CPACSStructuralProfile* CPACSSheetList::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSSheetList::GetNextUIDParent() const
    {
        return m_parent;
    }

    CTiglUIDObject* CPACSSheetList::GetNextUIDParent()
    {
        return m_parent;
    }

    CTiglUIDManager& CPACSSheetList::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSSheetList::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSSheetList::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element sheet
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/sheet")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/sheet", m_sheets, 1, tixi::xsdUnbounded, reinterpret_cast<CCPACSSheetList*>(this), m_uidMgr);
        }

    }

    void CPACSSheetList::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element sheet
        tixi::TixiSaveElements(tixiHandle, xpath + "/sheet", m_sheets);

    }

    const std::vector<std::unique_ptr<CPACSSheet>>& CPACSSheetList::GetSheets() const
    {
        return m_sheets;
    }

    std::vector<std::unique_ptr<CPACSSheet>>& CPACSSheetList::GetSheets()
    {
        return m_sheets;
    }

    size_t CPACSSheetList::GetSheetCount() const
    {
        return m_sheets.size();
    }

    size_t CPACSSheetList::GetSheetIndex(const std::string& UID) const
    {
        for (size_t i=0; i < GetSheetCount(); i++) {
            const std::string tmpUID(m_sheets[i]->GetUID());
            if (tmpUID == UID) {
                return i+1;
            }
        }
        throw CTiglError("Invalid UID in CPACSSheetList::GetSheetIndex", TIGL_UID_ERROR);
    }

    CPACSSheet& CPACSSheetList::GetSheet(size_t index)
    {
        if (index < 1 || index > GetSheetCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSSheet>>::GetSheet", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_sheets[index];
    }

    const CPACSSheet& CPACSSheetList::GetSheet(size_t index) const
    {
        if (index < 1 || index > GetSheetCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSSheet>>::GetSheet", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_sheets[index];
    }

    CPACSSheet& CPACSSheetList::GetSheet(const std::string& UID)
    {
        for (auto& elem : m_sheets ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSSheetList::GetSheet. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }

    const CPACSSheet& CPACSSheetList::GetSheet(const std::string& UID) const
    {
        for (auto& elem : m_sheets ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSSheetList::GetSheet. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }


    CPACSSheet& CPACSSheetList::AddSheet()
    {
        m_sheets.push_back(make_unique<CPACSSheet>(reinterpret_cast<CCPACSSheetList*>(this), m_uidMgr));
        return *m_sheets.back();
    }

    void CPACSSheetList::RemoveSheet(CPACSSheet& ref)
    {
        for (std::size_t i = 0; i < m_sheets.size(); i++) {
            if (m_sheets[i].get() == &ref) {
                m_sheets.erase(m_sheets.begin() + i);
                return;
            }
        }
        throw CTiglError("Element not found");
    }

} // namespace generated
} // namespace tigl
