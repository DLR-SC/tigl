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
#include "CCPACSLongFloorBeam.h"
#include "CPACSLongFloorBeamPosition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSLongFloorBeamPosition::CPACSLongFloorBeamPosition(CCPACSLongFloorBeam* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_positionY(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSLongFloorBeamPosition::~CPACSLongFloorBeamPosition()
    {
        if (m_uidMgr) m_uidMgr->TryUnregisterObject(m_uID);
        if (m_uidMgr) {
            if (!m_structuralElementUID.empty()) m_uidMgr->TryUnregisterReference(m_structuralElementUID, *this);
            if (!m_crossBeamUID.empty()) m_uidMgr->TryUnregisterReference(m_crossBeamUID, *this);
        }
    }

    const CCPACSLongFloorBeam* CPACSLongFloorBeamPosition::GetParent() const
    {
        return m_parent;
    }

    CCPACSLongFloorBeam* CPACSLongFloorBeamPosition::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSLongFloorBeamPosition::GetNextUIDParent() const
    {
        return m_parent;
    }

    CTiglUIDObject* CPACSLongFloorBeamPosition::GetNextUIDParent()
    {
        return m_parent;
    }

    CTiglUIDManager& CPACSLongFloorBeamPosition::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSLongFloorBeamPosition::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSLongFloorBeamPosition::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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

        // read element structuralElementUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/structuralElementUID")) {
            m_structuralElementUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/structuralElementUID");
            if (m_structuralElementUID.empty()) {
                LOG(WARNING) << "Required element structuralElementUID is empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_structuralElementUID.empty()) m_uidMgr->RegisterReference(m_structuralElementUID, *this);
        }
        else {
            LOG(ERROR) << "Required element structuralElementUID is missing at xpath " << xpath;
        }

        // read element crossBeamUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/crossBeamUID")) {
            m_crossBeamUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/crossBeamUID");
            if (m_crossBeamUID.empty()) {
                LOG(WARNING) << "Required element crossBeamUID is empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_crossBeamUID.empty()) m_uidMgr->RegisterReference(m_crossBeamUID, *this);
        }
        else {
            LOG(ERROR) << "Required element crossBeamUID is missing at xpath " << xpath;
        }

        // read element positionY
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/positionY")) {
            m_positionY = tixi::TixiGetElement<double>(tixiHandle, xpath + "/positionY");
        }
        else {
            LOG(ERROR) << "Required element positionY is missing at xpath " << xpath;
        }

        // read element alignment
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/alignment")) {
            m_alignment = boost::in_place(reinterpret_cast<CCPACSLongFloorBeamPosition*>(this), m_uidMgr);
            try {
                m_alignment->ReadCPACS(tixiHandle, xpath + "/alignment");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read alignment at xpath " << xpath << ": " << e.what();
                m_alignment = boost::none;
            }
        }

        // read element continuity
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/continuity")) {
            m_continuity = stringToCPACSContinuity(tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/continuity"));
        }

        // read element interpolation
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/interpolation")) {
            m_interpolation = stringToCPACSInterpolation(tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/interpolation"));
        }

        if (m_uidMgr && !m_uID.empty()) m_uidMgr->RegisterObject(m_uID, *this);
    }

    void CPACSLongFloorBeamPosition::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write attribute uID
        tixi::TixiSaveAttribute(tixiHandle, xpath, "uID", m_uID);

        // write element structuralElementUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/structuralElementUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/structuralElementUID", m_structuralElementUID);

        // write element crossBeamUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/crossBeamUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/crossBeamUID", m_crossBeamUID);

        // write element positionY
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/positionY");
        tixi::TixiSaveElement(tixiHandle, xpath + "/positionY", m_positionY);

        // write element alignment
        if (m_alignment) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/alignment");
            m_alignment->WriteCPACS(tixiHandle, xpath + "/alignment");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/alignment")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/alignment");
            }
        }

        // write element continuity
        if (m_continuity) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/continuity");
            tixi::TixiSaveElement(tixiHandle, xpath + "/continuity", CPACSContinuityToString(*m_continuity));
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/continuity")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/continuity");
            }
        }

        // write element interpolation
        if (m_interpolation) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/interpolation");
            tixi::TixiSaveElement(tixiHandle, xpath + "/interpolation", CPACSInterpolationToString(*m_interpolation));
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/interpolation")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/interpolation");
            }
        }

    }

    const std::string& CPACSLongFloorBeamPosition::GetUID() const
    {
        return m_uID;
    }

    void CPACSLongFloorBeamPosition::SetUID(const std::string& value)
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

    const std::string& CPACSLongFloorBeamPosition::GetStructuralElementUID() const
    {
        return m_structuralElementUID;
    }

    void CPACSLongFloorBeamPosition::SetStructuralElementUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_structuralElementUID.empty()) m_uidMgr->TryUnregisterReference(m_structuralElementUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_structuralElementUID = value;
    }

    const std::string& CPACSLongFloorBeamPosition::GetCrossBeamUID() const
    {
        return m_crossBeamUID;
    }

    void CPACSLongFloorBeamPosition::SetCrossBeamUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_crossBeamUID.empty()) m_uidMgr->TryUnregisterReference(m_crossBeamUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_crossBeamUID = value;
    }

    const double& CPACSLongFloorBeamPosition::GetPositionY() const
    {
        return m_positionY;
    }

    void CPACSLongFloorBeamPosition::SetPositionY(const double& value)
    {
        m_positionY = value;
    }

    const boost::optional<CPACSAlignmentStructMember>& CPACSLongFloorBeamPosition::GetAlignment() const
    {
        return m_alignment;
    }

    boost::optional<CPACSAlignmentStructMember>& CPACSLongFloorBeamPosition::GetAlignment()
    {
        return m_alignment;
    }

    const boost::optional<CPACSContinuity>& CPACSLongFloorBeamPosition::GetContinuity() const
    {
        return m_continuity;
    }

    void CPACSLongFloorBeamPosition::SetContinuity(const boost::optional<CPACSContinuity>& value)
    {
        m_continuity = value;
    }

    const boost::optional<CPACSInterpolation>& CPACSLongFloorBeamPosition::GetInterpolation() const
    {
        return m_interpolation;
    }

    void CPACSLongFloorBeamPosition::SetInterpolation(const boost::optional<CPACSInterpolation>& value)
    {
        m_interpolation = value;
    }

    CPACSAlignmentStructMember& CPACSLongFloorBeamPosition::GetAlignment(CreateIfNotExistsTag)
    {
        if (!m_alignment)
            m_alignment = boost::in_place(reinterpret_cast<CCPACSLongFloorBeamPosition*>(this), m_uidMgr);
        return *m_alignment;
    }

    void CPACSLongFloorBeamPosition::RemoveAlignment()
    {
        m_alignment = boost::none;
    }

    const CTiglUIDObject* CPACSLongFloorBeamPosition::GetNextUIDObject() const
    {
        return this;
    }

    void CPACSLongFloorBeamPosition::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_structuralElementUID == oldUid) {
            m_structuralElementUID = newUid;
        }
        if (m_crossBeamUID == oldUid) {
            m_crossBeamUID = newUid;
        }
    }

} // namespace generated
} // namespace tigl
