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
#include "CPACSSheetPoints.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSSheetPoints::CPACSSheetPoints(CPACSSheet3D* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSSheetPoints::~CPACSSheetPoints()
    {
        if (m_uidMgr) {
            for (std::vector<std::string>::iterator it = m_sheetPointUIDs.begin(); it != m_sheetPointUIDs.end(); ++it) {
                if (!it->empty()) m_uidMgr->TryUnregisterReference(*it, *this);
            }
        }
    }

    const CPACSSheet3D* CPACSSheetPoints::GetParent() const
    {
        return m_parent;
    }

    CPACSSheet3D* CPACSSheetPoints::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSSheetPoints::GetNextUIDParent() const
    {
        return m_parent;
    }

    CTiglUIDObject* CPACSSheetPoints::GetNextUIDParent()
    {
        return m_parent;
    }

    CTiglUIDManager& CPACSSheetPoints::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSSheetPoints::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSSheetPoints::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element sheetPointUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/sheetPointUID")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/sheetPointUID", m_sheetPointUIDs, 3, tixi::xsdUnbounded);
            if (m_uidMgr) {
                for (std::vector<std::string>::iterator it = m_sheetPointUIDs.begin(); it != m_sheetPointUIDs.end(); ++it) {
                    if (!it->empty()) m_uidMgr->RegisterReference(*it, *this);
                }
            }
        }

    }

    void CPACSSheetPoints::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element sheetPointUID
        tixi::TixiSaveElements(tixiHandle, xpath + "/sheetPointUID", m_sheetPointUIDs);

    }

    const std::vector<std::string>& CPACSSheetPoints::GetSheetPointUIDs() const
    {
        return m_sheetPointUIDs;
    }

    void CPACSSheetPoints::AddToSheetPointUIDs(const std::string& value)
    {
        if (m_uidMgr) {
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_sheetPointUIDs.push_back(value);
    }

    bool CPACSSheetPoints::RemoveFromSheetPointUIDs(const std::string& value)
    {
        const auto it = std::find(m_sheetPointUIDs.begin(), m_sheetPointUIDs.end(), value);
        if (it != m_sheetPointUIDs.end()) {
            if (m_uidMgr && !it->empty()) {
                m_uidMgr->TryUnregisterReference(*it, *this);
            }
            m_sheetPointUIDs.erase(it);
            return true;
        }
        return false;
    }

    const CTiglUIDObject* CPACSSheetPoints::GetNextUIDObject() const
    {
        return GetNextUIDParent();
    }

    void CPACSSheetPoints::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        for (auto& entry : m_sheetPointUIDs) {
            if (entry == oldUid) {
                entry = newUid;
            }
        }
    }

} // namespace generated
} // namespace tigl
