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
#include "CPACSCompartment.h"
#include "CPACSCompartmentGeometry.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSCompartmentGeometry::CPACSCompartmentGeometry(CPACSCompartment* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSCompartmentGeometry::~CPACSCompartmentGeometry()
    {
        if (m_uidMgr) {
            for (std::vector<std::string>::iterator it = m_boundaryElementUIDs.begin(); it != m_boundaryElementUIDs.end(); ++it) {
                if (!it->empty()) m_uidMgr->TryUnregisterReference(*it, *this);
            }
        }
    }

    const CPACSCompartment* CPACSCompartmentGeometry::GetParent() const
    {
        return m_parent;
    }

    CPACSCompartment* CPACSCompartmentGeometry::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSCompartmentGeometry::GetNextUIDParent() const
    {
        if (m_parent) {
            if (m_parent->GetUID())
                return m_parent;
            else
                return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSCompartmentGeometry::GetNextUIDParent()
    {
        if (m_parent) {
            if (m_parent->GetUID())
                return m_parent;
            else
                return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSCompartmentGeometry::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSCompartmentGeometry::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSCompartmentGeometry::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element boundaryElementUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/boundaryElementUID")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/boundaryElementUID", m_boundaryElementUIDs, 1, tixi::xsdUnbounded);
            if (m_uidMgr) {
                for (std::vector<std::string>::iterator it = m_boundaryElementUIDs.begin(); it != m_boundaryElementUIDs.end(); ++it) {
                    if (!it->empty()) m_uidMgr->RegisterReference(*it, *this);
                }
            }
        }

    }

    void CPACSCompartmentGeometry::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element boundaryElementUID
        tixi::TixiSaveElements(tixiHandle, xpath + "/boundaryElementUID", m_boundaryElementUIDs);

    }

    const std::vector<std::string>& CPACSCompartmentGeometry::GetBoundaryElementUIDs() const
    {
        return m_boundaryElementUIDs;
    }

    void CPACSCompartmentGeometry::AddToBoundaryElementUIDs(const std::string& value)
    {
        if (m_uidMgr) {
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_boundaryElementUIDs.push_back(value);
    }

    bool CPACSCompartmentGeometry::RemoveFromBoundaryElementUIDs(const std::string& value)
    {
        const auto it = std::find(m_boundaryElementUIDs.begin(), m_boundaryElementUIDs.end(), value);
        if (it != m_boundaryElementUIDs.end()) {
            if (m_uidMgr && !it->empty()) {
                m_uidMgr->TryUnregisterReference(*it, *this);
            }
            m_boundaryElementUIDs.erase(it);
            return true;
        }
        return false;
    }

    const CTiglUIDObject* CPACSCompartmentGeometry::GetNextUIDObject() const
    {
        return GetNextUIDParent();
    }

    void CPACSCompartmentGeometry::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        for (auto& entry : m_boundaryElementUIDs) {
            if (entry == oldUid) {
                entry = newUid;
            }
        }
    }

} // namespace generated
} // namespace tigl
