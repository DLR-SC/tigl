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
#include "CPACSTrackSecondaryStructure.h"
#include "CPACSTrackStructure.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSTrackSecondaryStructure::CPACSTrackSecondaryStructure(CPACSTrackStructure* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSTrackSecondaryStructure::~CPACSTrackSecondaryStructure()
    {
        if (m_uidMgr) {
            if (!m_materialUID.empty()) m_uidMgr->TryUnregisterReference(m_materialUID, *this);
        }
    }

    const CPACSTrackStructure* CPACSTrackSecondaryStructure::GetParent() const
    {
        return m_parent;
    }

    CPACSTrackStructure* CPACSTrackSecondaryStructure::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSTrackSecondaryStructure::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSTrackSecondaryStructure::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSTrackSecondaryStructure::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSTrackSecondaryStructure::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSTrackSecondaryStructure::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element materialUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/materialUID")) {
            m_materialUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/materialUID");
            if (m_materialUID.empty()) {
                LOG(WARNING) << "Required element materialUID is empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_materialUID.empty()) m_uidMgr->RegisterReference(m_materialUID, *this);
        }
        else {
            LOG(ERROR) << "Required element materialUID is missing at xpath " << xpath;
        }

    }

    void CPACSTrackSecondaryStructure::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element materialUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/materialUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/materialUID", m_materialUID);

    }

    const std::string& CPACSTrackSecondaryStructure::GetMaterialUID() const
    {
        return m_materialUID;
    }

    void CPACSTrackSecondaryStructure::SetMaterialUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_materialUID.empty()) m_uidMgr->TryUnregisterReference(m_materialUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_materialUID = value;
    }

    const CTiglUIDObject* CPACSTrackSecondaryStructure::GetNextUIDObject() const
    {
        return GetNextUIDParent();
    }

    void CPACSTrackSecondaryStructure::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_materialUID == oldUid) {
            m_materialUID = newUid;
        }
    }

} // namespace generated
} // namespace tigl
