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
#include "CCPACSPressureBulkheadAssembly.h"
#include "CPACSPressureBulkheadAssemblyPosition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSPressureBulkheadAssemblyPosition::CPACSPressureBulkheadAssemblyPosition(CCPACSPressureBulkheadAssembly* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSPressureBulkheadAssemblyPosition::~CPACSPressureBulkheadAssemblyPosition()
    {
        if (m_uidMgr) m_uidMgr->TryUnregisterObject(m_uID);
        if (m_uidMgr) {
            if (!m_frameUID.empty()) m_uidMgr->TryUnregisterReference(m_frameUID, *this);
            if (!m_pressureBulkheadElementUID.empty()) m_uidMgr->TryUnregisterReference(m_pressureBulkheadElementUID, *this);
        }
    }

    const CCPACSPressureBulkheadAssembly* CPACSPressureBulkheadAssemblyPosition::GetParent() const
    {
        return m_parent;
    }

    CCPACSPressureBulkheadAssembly* CPACSPressureBulkheadAssemblyPosition::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSPressureBulkheadAssemblyPosition::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSPressureBulkheadAssemblyPosition::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSPressureBulkheadAssemblyPosition::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSPressureBulkheadAssemblyPosition::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSPressureBulkheadAssemblyPosition::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read attribute uID
        if (tixi::TixiCheckAttribute(tixiHandle, xpath, "uID")) {
            m_uID = tixi::TixiGetAttribute<std::string>(tixiHandle, xpath, "uID");
            if (m_uID.empty()) {
                LOG(WARNING) << "Required attribute uID is empty at xpath " << xpath;
            }
        }
        else {
            LOG(ERROR) << "Required attribute uID is missing at xpath " << xpath;
        }

        // read element frameUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/frameUID")) {
            m_frameUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/frameUID");
            if (m_frameUID.empty()) {
                LOG(WARNING) << "Required element frameUID is empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_frameUID.empty()) m_uidMgr->RegisterReference(m_frameUID, *this);
        }
        else {
            LOG(ERROR) << "Required element frameUID is missing at xpath " << xpath;
        }

        // read element pressureBulkheadElementUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/pressureBulkheadElementUID")) {
            m_pressureBulkheadElementUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/pressureBulkheadElementUID");
            if (m_pressureBulkheadElementUID.empty()) {
                LOG(WARNING) << "Required element pressureBulkheadElementUID is empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_pressureBulkheadElementUID.empty()) m_uidMgr->RegisterReference(m_pressureBulkheadElementUID, *this);
        }
        else {
            LOG(ERROR) << "Required element pressureBulkheadElementUID is missing at xpath " << xpath;
        }

        if (m_uidMgr && !m_uID.empty()) m_uidMgr->RegisterObject(m_uID, *this);
    }

    void CPACSPressureBulkheadAssemblyPosition::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write attribute uID
        tixi::TixiSaveAttribute(tixiHandle, xpath, "uID", m_uID);

        // write element frameUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/frameUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/frameUID", m_frameUID);

        // write element pressureBulkheadElementUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/pressureBulkheadElementUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/pressureBulkheadElementUID", m_pressureBulkheadElementUID);

    }

    const std::string& CPACSPressureBulkheadAssemblyPosition::GetUID() const
    {
        return m_uID;
    }

    void CPACSPressureBulkheadAssemblyPosition::SetUID(const std::string& value)
    {
        if (m_uidMgr && value != m_uID) {
            if (m_uID.empty()) {
                m_uidMgr->RegisterObject(value, *this);
            }
            else {
                m_uidMgr->UpdateObjectUID(m_uID, value);
            }
        }
        m_uID = value;
    }

    const std::string& CPACSPressureBulkheadAssemblyPosition::GetFrameUID() const
    {
        return m_frameUID;
    }

    void CPACSPressureBulkheadAssemblyPosition::SetFrameUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_frameUID.empty()) m_uidMgr->TryUnregisterReference(m_frameUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_frameUID = value;
    }

    const std::string& CPACSPressureBulkheadAssemblyPosition::GetPressureBulkheadElementUID() const
    {
        return m_pressureBulkheadElementUID;
    }

    void CPACSPressureBulkheadAssemblyPosition::SetPressureBulkheadElementUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_pressureBulkheadElementUID.empty()) m_uidMgr->TryUnregisterReference(m_pressureBulkheadElementUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_pressureBulkheadElementUID = value;
    }

    const CTiglUIDObject* CPACSPressureBulkheadAssemblyPosition::GetNextUIDObject() const
    {
        return this;
    }

    void CPACSPressureBulkheadAssemblyPosition::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_frameUID == oldUid) {
            m_frameUID = newUid;
        }
        if (m_pressureBulkheadElementUID == oldUid) {
            m_pressureBulkheadElementUID = newUid;
        }
    }

} // namespace generated
} // namespace tigl
