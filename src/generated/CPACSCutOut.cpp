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
#include "CPACSCutOut.h"
#include "CPACSWindows.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSCutOut::CPACSCutOut(CPACSWindows* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_width(0)
        , m_height(0)
        , m_filletRadius(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSCutOut::~CPACSCutOut()
    {
        if (m_uidMgr) m_uidMgr->TryUnregisterObject(m_uID);
        if (m_uidMgr) {
            if (m_reinforcementElementUID && !m_reinforcementElementUID->empty()) m_uidMgr->TryUnregisterReference(*m_reinforcementElementUID, *this);
        }
    }

    const CPACSWindows* CPACSCutOut::GetParent() const
    {
        return m_parent;
    }

    CPACSWindows* CPACSCutOut::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSCutOut::GetNextUIDParent() const
    {
        return m_parent;
    }

    CTiglUIDObject* CPACSCutOut::GetNextUIDParent()
    {
        return m_parent;
    }

    CTiglUIDManager& CPACSCutOut::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSCutOut::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSCutOut::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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

        // read element name
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/name")) {
            m_name = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/name");
            if (m_name->empty()) {
                LOG(WARNING) << "Optional element name is present but empty at xpath " << xpath;
            }
        }

        // read element description
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/description")) {
            m_description = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/description");
            if (m_description->empty()) {
                LOG(WARNING) << "Optional element description is present but empty at xpath " << xpath;
            }
        }

        // read element width
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/width")) {
            m_width = tixi::TixiGetElement<double>(tixiHandle, xpath + "/width");
        }
        else {
            LOG(ERROR) << "Required element width is missing at xpath " << xpath;
        }

        // read element height
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/height")) {
            m_height = tixi::TixiGetElement<double>(tixiHandle, xpath + "/height");
        }
        else {
            LOG(ERROR) << "Required element height is missing at xpath " << xpath;
        }

        // read element filletRadius
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/filletRadius")) {
            m_filletRadius = tixi::TixiGetElement<double>(tixiHandle, xpath + "/filletRadius");
        }
        else {
            LOG(ERROR) << "Required element filletRadius is missing at xpath " << xpath;
        }

        // read element reinforcementElementUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/reinforcementElementUID")) {
            m_reinforcementElementUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/reinforcementElementUID");
            if (m_reinforcementElementUID->empty()) {
                LOG(WARNING) << "Optional element reinforcementElementUID is present but empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_reinforcementElementUID->empty()) m_uidMgr->RegisterReference(*m_reinforcementElementUID, *this);
        }

        if (m_uidMgr && !m_uID.empty()) m_uidMgr->RegisterObject(m_uID, *this);
    }

    void CPACSCutOut::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write attribute uID
        tixi::TixiSaveAttribute(tixiHandle, xpath, "uID", m_uID);

        // write element name
        if (m_name) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/name");
            tixi::TixiSaveElement(tixiHandle, xpath + "/name", *m_name);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/name")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/name");
            }
        }

        // write element description
        if (m_description) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/description");
            tixi::TixiSaveElement(tixiHandle, xpath + "/description", *m_description);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/description")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/description");
            }
        }

        // write element width
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/width");
        tixi::TixiSaveElement(tixiHandle, xpath + "/width", m_width);

        // write element height
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/height");
        tixi::TixiSaveElement(tixiHandle, xpath + "/height", m_height);

        // write element filletRadius
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/filletRadius");
        tixi::TixiSaveElement(tixiHandle, xpath + "/filletRadius", m_filletRadius);

        // write element reinforcementElementUID
        if (m_reinforcementElementUID) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/reinforcementElementUID");
            tixi::TixiSaveElement(tixiHandle, xpath + "/reinforcementElementUID", *m_reinforcementElementUID);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/reinforcementElementUID")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/reinforcementElementUID");
            }
        }

    }

    const std::string& CPACSCutOut::GetUID() const
    {
        return m_uID;
    }

    void CPACSCutOut::SetUID(const std::string& value)
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

    const boost::optional<std::string>& CPACSCutOut::GetName() const
    {
        return m_name;
    }

    void CPACSCutOut::SetName(const boost::optional<std::string>& value)
    {
        m_name = value;
    }

    const boost::optional<std::string>& CPACSCutOut::GetDescription() const
    {
        return m_description;
    }

    void CPACSCutOut::SetDescription(const boost::optional<std::string>& value)
    {
        m_description = value;
    }

    const double& CPACSCutOut::GetWidth() const
    {
        return m_width;
    }

    void CPACSCutOut::SetWidth(const double& value)
    {
        m_width = value;
    }

    const double& CPACSCutOut::GetHeight() const
    {
        return m_height;
    }

    void CPACSCutOut::SetHeight(const double& value)
    {
        m_height = value;
    }

    const double& CPACSCutOut::GetFilletRadius() const
    {
        return m_filletRadius;
    }

    void CPACSCutOut::SetFilletRadius(const double& value)
    {
        m_filletRadius = value;
    }

    const boost::optional<std::string>& CPACSCutOut::GetReinforcementElementUID() const
    {
        return m_reinforcementElementUID;
    }

    void CPACSCutOut::SetReinforcementElementUID(const boost::optional<std::string>& value)
    {
        if (m_uidMgr) {
            if (m_reinforcementElementUID && !m_reinforcementElementUID->empty()) m_uidMgr->TryUnregisterReference(*m_reinforcementElementUID, *this);
            if (value && !value->empty()) m_uidMgr->RegisterReference(*value, *this);
        }
        m_reinforcementElementUID = value;
    }

    const CTiglUIDObject* CPACSCutOut::GetNextUIDObject() const
    {
        return this;
    }

    void CPACSCutOut::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_reinforcementElementUID && *m_reinforcementElementUID == oldUid) {
            m_reinforcementElementUID = newUid;
        }
    }

} // namespace generated
} // namespace tigl
