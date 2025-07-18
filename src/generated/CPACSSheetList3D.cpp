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
#include "CPACSSheet3D.h"
#include "CPACSSheetList3D.h"
#include "CPACSStructuralProfile3D.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSSheetList3D::CPACSSheetList3D(CPACSStructuralProfile3D* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSSheetList3D::~CPACSSheetList3D()
    {
    }

    const CPACSStructuralProfile3D* CPACSSheetList3D::GetParent() const
    {
        return m_parent;
    }

    CPACSStructuralProfile3D* CPACSSheetList3D::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSSheetList3D::GetNextUIDParent() const
    {
        return m_parent;
    }

    CTiglUIDObject* CPACSSheetList3D::GetNextUIDParent()
    {
        return m_parent;
    }

    CTiglUIDManager& CPACSSheetList3D::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSSheetList3D::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSSheetList3D::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element sheet3D
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/sheet3D")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/sheet3D", m_sheet3Ds, 1, tixi::xsdUnbounded, this, m_uidMgr);
        }

    }

    void CPACSSheetList3D::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element sheet3D
        tixi::TixiSaveElements(tixiHandle, xpath + "/sheet3D", m_sheet3Ds);

    }

    const std::vector<std::unique_ptr<CPACSSheet3D>>& CPACSSheetList3D::GetSheet3Ds() const
    {
        return m_sheet3Ds;
    }

    std::vector<std::unique_ptr<CPACSSheet3D>>& CPACSSheetList3D::GetSheet3Ds()
    {
        return m_sheet3Ds;
    }

    size_t CPACSSheetList3D::GetSheet3DCount() const
    {
        return m_sheet3Ds.size();
    }

    size_t CPACSSheetList3D::GetSheet3DIndex(const std::string& UID) const
    {
        for (size_t i=0; i < GetSheet3DCount(); i++) {
            const std::string tmpUID(m_sheet3Ds[i]->GetUID());
            if (tmpUID == UID) {
                return i+1;
            }
        }
        throw CTiglError("Invalid UID in CPACSSheetList3D::GetSheet3DIndex", TIGL_UID_ERROR);
    }

    CPACSSheet3D& CPACSSheetList3D::GetSheet3D(size_t index)
    {
        if (index < 1 || index > GetSheet3DCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSSheet3D>>::GetSheet3D", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_sheet3Ds[index];
    }

    const CPACSSheet3D& CPACSSheetList3D::GetSheet3D(size_t index) const
    {
        if (index < 1 || index > GetSheet3DCount()) {
            throw CTiglError("Invalid index in std::vector<std::unique_ptr<CPACSSheet3D>>::GetSheet3D", TIGL_INDEX_ERROR);
        }
        index--;
        return *m_sheet3Ds[index];
    }

    CPACSSheet3D& CPACSSheetList3D::GetSheet3D(const std::string& UID)
    {
        for (auto& elem : m_sheet3Ds ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSSheetList3D::GetSheet3D. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }

    const CPACSSheet3D& CPACSSheetList3D::GetSheet3D(const std::string& UID) const
    {
        for (auto& elem : m_sheet3Ds ) {
            if (elem->GetUID() == UID)
                return *elem;
            }
            throw CTiglError("Invalid UID in CPACSSheetList3D::GetSheet3D. \""+ UID + "\" not found in CPACS file!" , TIGL_UID_ERROR);
    }


    CPACSSheet3D& CPACSSheetList3D::AddSheet3D()
    {
        m_sheet3Ds.push_back(make_unique<CPACSSheet3D>(this, m_uidMgr));
        return *m_sheet3Ds.back();
    }

    void CPACSSheetList3D::RemoveSheet3D(CPACSSheet3D& ref)
    {
        for (std::size_t i = 0; i < m_sheet3Ds.size(); i++) {
            if (m_sheet3Ds[i].get() == &ref) {
                m_sheet3Ds.erase(m_sheet3Ds.begin() + i);
                return;
            }
        }
        throw CTiglError("Element not found");
    }

} // namespace generated
} // namespace tigl
