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
#include "CPACSTrackStrut.h"
#include "CPACSTrackStruts.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSTrackStrut::CPACSTrackStrut(CPACSTrackStruts* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSTrackStrut::~CPACSTrackStrut()
    {
        if (m_uidMgr) {
            if (!m_materialUID.empty()) m_uidMgr->TryUnregisterReference(m_materialUID, *this);
            if (!m_profileUID.empty()) m_uidMgr->TryUnregisterReference(m_profileUID, *this);
        }
    }

    const CPACSTrackStruts* CPACSTrackStrut::GetParent() const
    {
        return m_parent;
    }

    CPACSTrackStruts* CPACSTrackStrut::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSTrackStrut::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSTrackStrut::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSTrackStrut::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSTrackStrut::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSTrackStrut::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element name
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/name")) {
            m_name = stringToCPACSTrackStrut_name(tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/name"));
        }
        else {
            LOG(ERROR) << "Required element name is missing at xpath " << xpath;
        }

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

        // read element profileUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/profileUID")) {
            m_profileUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/profileUID");
            if (m_profileUID.empty()) {
                LOG(WARNING) << "Required element profileUID is empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_profileUID.empty()) m_uidMgr->RegisterReference(m_profileUID, *this);
        }
        else {
            LOG(ERROR) << "Required element profileUID is missing at xpath " << xpath;
        }

    }

    void CPACSTrackStrut::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element name
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/name");
        tixi::TixiSaveElement(tixiHandle, xpath + "/name", CPACSTrackStrut_nameToString(m_name));

        // write element materialUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/materialUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/materialUID", m_materialUID);

        // write element profileUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/profileUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/profileUID", m_profileUID);

    }

    const CPACSTrackStrut_name& CPACSTrackStrut::GetName() const
    {
        return m_name;
    }

    void CPACSTrackStrut::SetName(const CPACSTrackStrut_name& value)
    {
        m_name = value;
    }

    const std::string& CPACSTrackStrut::GetMaterialUID() const
    {
        return m_materialUID;
    }

    void CPACSTrackStrut::SetMaterialUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_materialUID.empty()) m_uidMgr->TryUnregisterReference(m_materialUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_materialUID = value;
    }

    const std::string& CPACSTrackStrut::GetProfileUID() const
    {
        return m_profileUID;
    }

    void CPACSTrackStrut::SetProfileUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_profileUID.empty()) m_uidMgr->TryUnregisterReference(m_profileUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_profileUID = value;
    }

    const CTiglUIDObject* CPACSTrackStrut::GetNextUIDObject() const
    {
        return GetNextUIDParent();
    }

    void CPACSTrackStrut::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_materialUID == oldUid) {
            m_materialUID = newUid;
        }
        if (m_profileUID == oldUid) {
            m_profileUID = newUid;
        }
    }

} // namespace generated
} // namespace tigl
