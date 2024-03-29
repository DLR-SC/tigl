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
#include "CPACSStructuralWallElement.h"
#include "CPACSStructuralWallElements.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSStructuralWallElement::CPACSStructuralWallElement(CPACSStructuralWallElements* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSStructuralWallElement::~CPACSStructuralWallElement()
    {
        if (m_uidMgr && m_uID) m_uidMgr->TryUnregisterObject(*m_uID);
    }

    const CPACSStructuralWallElements* CPACSStructuralWallElement::GetParent() const
    {
        return m_parent;
    }

    CPACSStructuralWallElements* CPACSStructuralWallElement::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSStructuralWallElement::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSStructuralWallElement::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSStructuralWallElement::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSStructuralWallElement::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSStructuralWallElement::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read attribute uID
        if (tixi::TixiCheckAttribute(tixiHandle, xpath, "uID")) {
            m_uID = tixi::TixiGetAttribute<std::string>(tixiHandle, xpath, "uID");
            if (m_uID->empty()) {
                LOG(WARNING) << "Optional attribute uID is present but empty at xpath " << xpath;
            }
        }

        // read element sheetElementUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/sheetElementUID")) {
            m_sheetElementUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/sheetElementUID");
            if (m_sheetElementUID.empty()) {
                LOG(WARNING) << "Required element sheetElementUID is empty at xpath " << xpath;
            }
        }
        else {
            LOG(ERROR) << "Required element sheetElementUID is missing at xpath " << xpath;
        }

        // read element innerLongitudinalCap
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/innerLongitudinalCap")) {
            m_innerLongitudinalCap = boost::in_place(this, m_uidMgr);
            try {
                m_innerLongitudinalCap->ReadCPACS(tixiHandle, xpath + "/innerLongitudinalCap");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read innerLongitudinalCap at xpath " << xpath << ": " << e.what();
                m_innerLongitudinalCap = boost::none;
            }
        }

        // read element outerLongitudinalCap
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/outerLongitudinalCap")) {
            m_outerLongitudinalCap = boost::in_place(this, m_uidMgr);
            try {
                m_outerLongitudinalCap->ReadCPACS(tixiHandle, xpath + "/outerLongitudinalCap");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read outerLongitudinalCap at xpath " << xpath << ": " << e.what();
                m_outerLongitudinalCap = boost::none;
            }
        }

        // read element lateralCaps
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/lateralCaps")) {
            m_lateralCaps = boost::in_place(this, m_uidMgr);
            try {
                m_lateralCaps->ReadCPACS(tixiHandle, xpath + "/lateralCaps");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read lateralCaps at xpath " << xpath << ": " << e.what();
                m_lateralCaps = boost::none;
            }
        }

        if (m_uidMgr && m_uID) m_uidMgr->RegisterObject(*m_uID, *this);
    }

    void CPACSStructuralWallElement::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write attribute uID
        if (m_uID) {
            tixi::TixiSaveAttribute(tixiHandle, xpath, "uID", *m_uID);
        }
        else {
            if (tixi::TixiCheckAttribute(tixiHandle, xpath, "uID")) {
                tixi::TixiRemoveAttribute(tixiHandle, xpath, "uID");
            }
        }

        // write element sheetElementUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/sheetElementUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/sheetElementUID", m_sheetElementUID);

        // write element innerLongitudinalCap
        if (m_innerLongitudinalCap) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/innerLongitudinalCap");
            m_innerLongitudinalCap->WriteCPACS(tixiHandle, xpath + "/innerLongitudinalCap");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/innerLongitudinalCap")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/innerLongitudinalCap");
            }
        }

        // write element outerLongitudinalCap
        if (m_outerLongitudinalCap) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/outerLongitudinalCap");
            m_outerLongitudinalCap->WriteCPACS(tixiHandle, xpath + "/outerLongitudinalCap");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/outerLongitudinalCap")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/outerLongitudinalCap");
            }
        }

        // write element lateralCaps
        if (m_lateralCaps) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/lateralCaps");
            m_lateralCaps->WriteCPACS(tixiHandle, xpath + "/lateralCaps");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/lateralCaps")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/lateralCaps");
            }
        }

    }

    const boost::optional<std::string>& CPACSStructuralWallElement::GetUID() const
    {
        return m_uID;
    }

    void CPACSStructuralWallElement::SetUID(const boost::optional<std::string>& value)
    {
        if (m_uidMgr && value != m_uID) {
            if (!m_uID && value) {
                m_uidMgr->RegisterObject(*value, *this);
            }
            else if (m_uID && !value) {
                m_uidMgr->TryUnregisterObject(*m_uID);
            }
            else if (m_uID && value) {
                m_uidMgr->UpdateObjectUID(*m_uID, *value);
            }
        }
        m_uID = value;
    }

    const std::string& CPACSStructuralWallElement::GetSheetElementUID() const
    {
        return m_sheetElementUID;
    }

    void CPACSStructuralWallElement::SetSheetElementUID(const std::string& value)
    {
        m_sheetElementUID = value;
    }

    const boost::optional<CPACSCap>& CPACSStructuralWallElement::GetInnerLongitudinalCap() const
    {
        return m_innerLongitudinalCap;
    }

    boost::optional<CPACSCap>& CPACSStructuralWallElement::GetInnerLongitudinalCap()
    {
        return m_innerLongitudinalCap;
    }

    const boost::optional<CPACSCap>& CPACSStructuralWallElement::GetOuterLongitudinalCap() const
    {
        return m_outerLongitudinalCap;
    }

    boost::optional<CPACSCap>& CPACSStructuralWallElement::GetOuterLongitudinalCap()
    {
        return m_outerLongitudinalCap;
    }

    const boost::optional<CPACSLateralCap>& CPACSStructuralWallElement::GetLateralCaps() const
    {
        return m_lateralCaps;
    }

    boost::optional<CPACSLateralCap>& CPACSStructuralWallElement::GetLateralCaps()
    {
        return m_lateralCaps;
    }

    CPACSCap& CPACSStructuralWallElement::GetInnerLongitudinalCap(CreateIfNotExistsTag)
    {
        if (!m_innerLongitudinalCap)
            m_innerLongitudinalCap = boost::in_place(this, m_uidMgr);
        return *m_innerLongitudinalCap;
    }

    void CPACSStructuralWallElement::RemoveInnerLongitudinalCap()
    {
        m_innerLongitudinalCap = boost::none;
    }

    CPACSCap& CPACSStructuralWallElement::GetOuterLongitudinalCap(CreateIfNotExistsTag)
    {
        if (!m_outerLongitudinalCap)
            m_outerLongitudinalCap = boost::in_place(this, m_uidMgr);
        return *m_outerLongitudinalCap;
    }

    void CPACSStructuralWallElement::RemoveOuterLongitudinalCap()
    {
        m_outerLongitudinalCap = boost::none;
    }

    CPACSLateralCap& CPACSStructuralWallElement::GetLateralCaps(CreateIfNotExistsTag)
    {
        if (!m_lateralCaps)
            m_lateralCaps = boost::in_place(this, m_uidMgr);
        return *m_lateralCaps;
    }

    void CPACSStructuralWallElement::RemoveLateralCaps()
    {
        m_lateralCaps = boost::none;
    }

} // namespace generated
} // namespace tigl
