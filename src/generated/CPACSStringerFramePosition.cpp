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
#include "CCPACSFrame.h"
#include "CCPACSFuselageStringer.h"
#include "CPACSStringerFramePosition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSStringerFramePosition::CPACSStringerFramePosition(CCPACSFrame* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_referenceY(0)
        , m_referenceZ(0)
        , m_referenceAngle(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CCPACSFrame);
    }

    CPACSStringerFramePosition::CPACSStringerFramePosition(CCPACSFuselageStringer* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_referenceY(0)
        , m_referenceZ(0)
        , m_referenceAngle(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CCPACSFuselageStringer);
    }

    CPACSStringerFramePosition::~CPACSStringerFramePosition()
    {
        if (m_uidMgr) m_uidMgr->TryUnregisterObject(m_uID);
        if (m_uidMgr) {
            if (!m_structuralElementUID.empty()) m_uidMgr->TryUnregisterReference(m_structuralElementUID, *this);
            if (m_sectionElementUID_choice2 && !m_sectionElementUID_choice2->empty()) m_uidMgr->TryUnregisterReference(*m_sectionElementUID_choice2, *this);
        }
    }

    const CTiglUIDObject* CPACSStringerFramePosition::GetNextUIDParent() const
    {
        if (m_parent) {
            if (IsParent<CCPACSFrame>()) {
                return GetParent<CCPACSFrame>();
            }
            if (IsParent<CCPACSFuselageStringer>()) {
                return GetParent<CCPACSFuselageStringer>();
            }
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSStringerFramePosition::GetNextUIDParent()
    {
        if (m_parent) {
            if (IsParent<CCPACSFrame>()) {
                return GetParent<CCPACSFrame>();
            }
            if (IsParent<CCPACSFuselageStringer>()) {
                return GetParent<CCPACSFuselageStringer>();
            }
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSStringerFramePosition::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSStringerFramePosition::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSStringerFramePosition::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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

        // read element positionX
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/positionX")) {
            m_positionX_choice1 = tixi::TixiGetElement<double>(tixiHandle, xpath + "/positionX");
        }

        // read element sectionElementUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/sectionElementUID")) {
            m_sectionElementUID_choice2 = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/sectionElementUID");
            if (m_sectionElementUID_choice2->empty()) {
                LOG(WARNING) << "Optional element sectionElementUID is present but empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_sectionElementUID_choice2->empty()) m_uidMgr->RegisterReference(*m_sectionElementUID_choice2, *this);
        }

        // read element referenceY
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/referenceY")) {
            m_referenceY = tixi::TixiGetElement<double>(tixiHandle, xpath + "/referenceY");
        }
        else {
            LOG(ERROR) << "Required element referenceY is missing at xpath " << xpath;
        }

        // read element referenceZ
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/referenceZ")) {
            m_referenceZ = tixi::TixiGetElement<double>(tixiHandle, xpath + "/referenceZ");
        }
        else {
            LOG(ERROR) << "Required element referenceZ is missing at xpath " << xpath;
        }

        // read element referenceAngle
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/referenceAngle")) {
            m_referenceAngle = tixi::TixiGetElement<double>(tixiHandle, xpath + "/referenceAngle");
        }
        else {
            LOG(ERROR) << "Required element referenceAngle is missing at xpath " << xpath;
        }

        // read element alignment
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/alignment")) {
            m_alignment = boost::in_place(reinterpret_cast<CCPACSFuselageStringerFramePosition*>(this), m_uidMgr);
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
        if (!ValidateChoices()) {
            LOG(ERROR) << "Invalid choice configuration at xpath " << xpath;
        }
    }

    void CPACSStringerFramePosition::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        const std::vector<std::string> childElemOrder = { "structuralElementUID", "positionX", "sectionElementUID", "referenceY", "referenceZ", "referenceAngle", "alignment", "continuity", "interpolation" };

        // write attribute uID
        tixi::TixiSaveAttribute(tixiHandle, xpath, "uID", m_uID);

        // write element structuralElementUID
        tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/structuralElementUID", childElemOrder);
        tixi::TixiSaveElement(tixiHandle, xpath + "/structuralElementUID", m_structuralElementUID);

        // write element positionX
        if (m_positionX_choice1) {
            tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/positionX", childElemOrder);
            tixi::TixiSaveElement(tixiHandle, xpath + "/positionX", *m_positionX_choice1);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/positionX")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/positionX");
            }
        }

        // write element sectionElementUID
        if (m_sectionElementUID_choice2) {
            tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/sectionElementUID", childElemOrder);
            tixi::TixiSaveElement(tixiHandle, xpath + "/sectionElementUID", *m_sectionElementUID_choice2);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/sectionElementUID")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/sectionElementUID");
            }
        }

        // write element referenceY
        tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/referenceY", childElemOrder);
        tixi::TixiSaveElement(tixiHandle, xpath + "/referenceY", m_referenceY);

        // write element referenceZ
        tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/referenceZ", childElemOrder);
        tixi::TixiSaveElement(tixiHandle, xpath + "/referenceZ", m_referenceZ);

        // write element referenceAngle
        tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/referenceAngle", childElemOrder);
        tixi::TixiSaveElement(tixiHandle, xpath + "/referenceAngle", m_referenceAngle);

        // write element alignment
        if (m_alignment) {
            tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/alignment", childElemOrder);
            m_alignment->WriteCPACS(tixiHandle, xpath + "/alignment");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/alignment")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/alignment");
            }
        }

        // write element continuity
        if (m_continuity) {
            tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/continuity", childElemOrder);
            tixi::TixiSaveElement(tixiHandle, xpath + "/continuity", CPACSContinuityToString(*m_continuity));
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/continuity")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/continuity");
            }
        }

        // write element interpolation
        if (m_interpolation) {
            tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/interpolation", childElemOrder);
            tixi::TixiSaveElement(tixiHandle, xpath + "/interpolation", CPACSInterpolationToString(*m_interpolation));
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/interpolation")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/interpolation");
            }
        }

    }

    bool CPACSStringerFramePosition::ValidateChoices() const
    {
        return
        (
            (
                (
                    // mandatory elements of this choice must be there
                    m_positionX_choice1.is_initialized()
                    &&
                    // elements of other choices must not be there
                    !(
                        m_sectionElementUID_choice2.is_initialized()
                    )
                )
                +
                (
                    // mandatory elements of this choice must be there
                    m_sectionElementUID_choice2.is_initialized()
                    &&
                    // elements of other choices must not be there
                    !(
                        m_positionX_choice1.is_initialized()
                    )
                )
                == 1
            )
        )
        ;
    }

    const std::string& CPACSStringerFramePosition::GetUID() const
    {
        return m_uID;
    }

    void CPACSStringerFramePosition::SetUID(const std::string& value)
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

    const std::string& CPACSStringerFramePosition::GetStructuralElementUID() const
    {
        return m_structuralElementUID;
    }

    void CPACSStringerFramePosition::SetStructuralElementUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_structuralElementUID.empty()) m_uidMgr->TryUnregisterReference(m_structuralElementUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_structuralElementUID = value;
    }

    const boost::optional<double>& CPACSStringerFramePosition::GetPositionX_choice1() const
    {
        return m_positionX_choice1;
    }

    void CPACSStringerFramePosition::SetPositionX_choice1(const boost::optional<double>& value)
    {
        m_positionX_choice1 = value;
    }

    const boost::optional<std::string>& CPACSStringerFramePosition::GetSectionElementUID_choice2() const
    {
        return m_sectionElementUID_choice2;
    }

    void CPACSStringerFramePosition::SetSectionElementUID_choice2(const boost::optional<std::string>& value)
    {
        if (m_uidMgr) {
            if (m_sectionElementUID_choice2 && !m_sectionElementUID_choice2->empty()) m_uidMgr->TryUnregisterReference(*m_sectionElementUID_choice2, *this);
            if (value && !value->empty()) m_uidMgr->RegisterReference(*value, *this);
        }
        m_sectionElementUID_choice2 = value;
    }

    const double& CPACSStringerFramePosition::GetReferenceY() const
    {
        return m_referenceY;
    }

    void CPACSStringerFramePosition::SetReferenceY(const double& value)
    {
        m_referenceY = value;
    }

    const double& CPACSStringerFramePosition::GetReferenceZ() const
    {
        return m_referenceZ;
    }

    void CPACSStringerFramePosition::SetReferenceZ(const double& value)
    {
        m_referenceZ = value;
    }

    const double& CPACSStringerFramePosition::GetReferenceAngle() const
    {
        return m_referenceAngle;
    }

    void CPACSStringerFramePosition::SetReferenceAngle(const double& value)
    {
        m_referenceAngle = value;
    }

    const boost::optional<CPACSAlignmentStringFrame>& CPACSStringerFramePosition::GetAlignment() const
    {
        return m_alignment;
    }

    boost::optional<CPACSAlignmentStringFrame>& CPACSStringerFramePosition::GetAlignment()
    {
        return m_alignment;
    }

    const boost::optional<CPACSContinuity>& CPACSStringerFramePosition::GetContinuity() const
    {
        return m_continuity;
    }

    void CPACSStringerFramePosition::SetContinuity(const boost::optional<CPACSContinuity>& value)
    {
        m_continuity = value;
    }

    const boost::optional<CPACSInterpolation>& CPACSStringerFramePosition::GetInterpolation() const
    {
        return m_interpolation;
    }

    void CPACSStringerFramePosition::SetInterpolation(const boost::optional<CPACSInterpolation>& value)
    {
        m_interpolation = value;
    }

    CPACSAlignmentStringFrame& CPACSStringerFramePosition::GetAlignment(CreateIfNotExistsTag)
    {
        if (!m_alignment)
            m_alignment = boost::in_place(reinterpret_cast<CCPACSFuselageStringerFramePosition*>(this), m_uidMgr);
        return *m_alignment;
    }

    void CPACSStringerFramePosition::RemoveAlignment()
    {
        m_alignment = boost::none;
    }

    const CTiglUIDObject* CPACSStringerFramePosition::GetNextUIDObject() const
    {
        return this;
    }

    void CPACSStringerFramePosition::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_structuralElementUID == oldUid) {
            m_structuralElementUID = newUid;
        }
        if (m_sectionElementUID_choice2 && *m_sectionElementUID_choice2 == oldUid) {
            m_sectionElementUID_choice2 = newUid;
        }
    }

} // namespace generated
} // namespace tigl
