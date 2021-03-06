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
#include "CCPACSWingRibExplicitPositioning.h"
#include "CCPACSWingRibsPositioning.h"
#include "CPACSCurvePoint.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSCurvePoint::CPACSCurvePoint(CCPACSWingRibExplicitPositioning* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_eta(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CCPACSWingRibExplicitPositioning);
    }

    CPACSCurvePoint::CPACSCurvePoint(CCPACSWingRibsPositioning* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_eta(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CCPACSWingRibsPositioning);
    }

    CPACSCurvePoint::~CPACSCurvePoint()
    {
        if (m_uidMgr) {
            if (!m_referenceUID.empty()) m_uidMgr->TryUnregisterReference(m_referenceUID, *this);
        }
    }

    const CTiglUIDObject* CPACSCurvePoint::GetNextUIDParent() const
    {
        if (m_parent) {
            if (IsParent<CCPACSWingRibExplicitPositioning>()) {
                return GetParent<CCPACSWingRibExplicitPositioning>()->GetNextUIDParent();
            }
            if (IsParent<CCPACSWingRibsPositioning>()) {
                return GetParent<CCPACSWingRibsPositioning>()->GetNextUIDParent();
            }
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSCurvePoint::GetNextUIDParent()
    {
        if (m_parent) {
            if (IsParent<CCPACSWingRibExplicitPositioning>()) {
                return GetParent<CCPACSWingRibExplicitPositioning>()->GetNextUIDParent();
            }
            if (IsParent<CCPACSWingRibsPositioning>()) {
                return GetParent<CCPACSWingRibsPositioning>()->GetNextUIDParent();
            }
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSCurvePoint::GetUIDManager()
    {
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSCurvePoint::GetUIDManager() const
    {
        return *m_uidMgr;
    }

    void CPACSCurvePoint::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element eta
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/eta")) {
            m_eta = tixi::TixiGetElement<double>(tixiHandle, xpath + "/eta");
        }
        else {
            LOG(ERROR) << "Required element eta is missing at xpath " << xpath;
        }

        // read element referenceUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/referenceUID")) {
            m_referenceUID = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/referenceUID");
            if (m_referenceUID.empty()) {
                LOG(WARNING) << "Required element referenceUID is empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_referenceUID.empty()) m_uidMgr->RegisterReference(m_referenceUID, *this);
        }
        else {
            LOG(ERROR) << "Required element referenceUID is missing at xpath " << xpath;
        }

    }

    void CPACSCurvePoint::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element eta
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/eta");
        tixi::TixiSaveElement(tixiHandle, xpath + "/eta", m_eta);

        // write element referenceUID
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/referenceUID");
        tixi::TixiSaveElement(tixiHandle, xpath + "/referenceUID", m_referenceUID);

    }

    const double& CPACSCurvePoint::GetEta() const
    {
        return m_eta;
    }

    void CPACSCurvePoint::SetEta(const double& value)
    {
        m_eta = value;
    }

    const std::string& CPACSCurvePoint::GetReferenceUID() const
    {
        return m_referenceUID;
    }

    void CPACSCurvePoint::SetReferenceUID(const std::string& value)
    {
        if (m_uidMgr) {
            if (!m_referenceUID.empty()) m_uidMgr->TryUnregisterReference(m_referenceUID, *this);
            if (!value.empty()) m_uidMgr->RegisterReference(value, *this);
        }
        m_referenceUID = value;
    }

    const CTiglUIDObject* CPACSCurvePoint::GetNextUIDObject() const
    {
        return GetNextUIDParent();
    }

    void CPACSCurvePoint::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_referenceUID == oldUid) {
            m_referenceUID = newUid;
        }
    }

} // namespace generated
} // namespace tigl
