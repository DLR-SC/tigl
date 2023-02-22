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
#include "CPACSAxle.h"
#include "CPACSBogie.h"
#include "CPACSPiston.h"
#include "CPACSStrutAssembly.h"
#include "CPACSStrutProperties.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSStrutProperties::CPACSStrutProperties(CPACSAxle* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CPACSAxle);
    }

    CPACSStrutProperties::CPACSStrutProperties(CPACSBogie* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CPACSBogie);
    }

    CPACSStrutProperties::CPACSStrutProperties(CPACSPiston* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CPACSPiston);
    }

    CPACSStrutProperties::CPACSStrutProperties(CPACSStrutAssembly* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CPACSStrutAssembly);
    }

    CPACSStrutProperties::~CPACSStrutProperties()
    {
        if (m_uidMgr) {
            if (m_materialUID_choice1 && !m_materialUID_choice1->empty()) m_uidMgr->TryUnregisterReference(*m_materialUID_choice1, *this);
            if (m_structuralElementUID_choice2 && !m_structuralElementUID_choice2->empty()) m_uidMgr->TryUnregisterReference(*m_structuralElementUID_choice2, *this);
        }
    }

    const CTiglUIDObject* CPACSStrutProperties::GetNextUIDParent() const
    {
        if (m_parent) {
            if (IsParent<CPACSAxle>()) {
                return GetParent<CPACSAxle>();
            }
            if (IsParent<CPACSBogie>()) {
                return GetParent<CPACSBogie>();
            }
            if (IsParent<CPACSPiston>()) {
                return GetParent<CPACSPiston>();
            }
            if (IsParent<CPACSStrutAssembly>()) {
                return GetParent<CPACSStrutAssembly>();
            }
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSStrutProperties::GetNextUIDParent()
    {
        if (m_parent) {
            if (IsParent<CPACSAxle>()) {
                return GetParent<CPACSAxle>();
            }
            if (IsParent<CPACSBogie>()) {
                return GetParent<CPACSBogie>();
            }
            if (IsParent<CPACSPiston>()) {
                return GetParent<CPACSPiston>();
            }
            if (IsParent<CPACSStrutAssembly>()) {
                return GetParent<CPACSStrutAssembly>();
            }
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSStrutProperties::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSStrutProperties::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSStrutProperties::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element radius
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/radius")) {
            m_radius_choice1 = tixi::TixiGetElement<double>(tixiHandle, xpath + "/radius");
        }

        // read element materialUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/materialUID")) {
            m_materialUID_choice1 = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/materialUID");
            if (m_materialUID_choice1->empty()) {
                LOG(WARNING) << "Optional element materialUID is present but empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_materialUID_choice1->empty()) m_uidMgr->RegisterReference(*m_materialUID_choice1, *this);
        }

        // read element innerRadius
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/innerRadius")) {
            m_innerRadius_choice1 = tixi::TixiGetElement<double>(tixiHandle, xpath + "/innerRadius");
        }

        // read element structuralElementUID
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/structuralElementUID")) {
            m_structuralElementUID_choice2 = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/structuralElementUID");
            if (m_structuralElementUID_choice2->empty()) {
                LOG(WARNING) << "Optional element structuralElementUID is present but empty at xpath " << xpath;
            }
            if (m_uidMgr && !m_structuralElementUID_choice2->empty()) m_uidMgr->RegisterReference(*m_structuralElementUID_choice2, *this);
        }

        if (!ValidateChoices()) {
            LOG(ERROR) << "Invalid choice configuration at xpath " << xpath;
        }
    }

    void CPACSStrutProperties::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element radius
        if (m_radius_choice1) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/radius");
            tixi::TixiSaveElement(tixiHandle, xpath + "/radius", *m_radius_choice1);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/radius")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/radius");
            }
        }

        // write element materialUID
        if (m_materialUID_choice1) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/materialUID");
            tixi::TixiSaveElement(tixiHandle, xpath + "/materialUID", *m_materialUID_choice1);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/materialUID")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/materialUID");
            }
        }

        // write element innerRadius
        if (m_innerRadius_choice1) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/innerRadius");
            tixi::TixiSaveElement(tixiHandle, xpath + "/innerRadius", *m_innerRadius_choice1);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/innerRadius")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/innerRadius");
            }
        }

        // write element structuralElementUID
        if (m_structuralElementUID_choice2) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/structuralElementUID");
            tixi::TixiSaveElement(tixiHandle, xpath + "/structuralElementUID", *m_structuralElementUID_choice2);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/structuralElementUID")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/structuralElementUID");
            }
        }

    }

    bool CPACSStrutProperties::ValidateChoices() const
    {
        return
        (
            (
                (
                    // mandatory elements of this choice must be there
                    m_radius_choice1.is_initialized()
                    &&
                    m_materialUID_choice1.is_initialized()
                    &&
                    true // m_innerRadius_choice1 is optional in choice
                    &&
                    // elements of other choices must not be there
                    !(
                        m_structuralElementUID_choice2.is_initialized()
                    )
                )
                +
                (
                    // mandatory elements of this choice must be there
                    m_structuralElementUID_choice2.is_initialized()
                    &&
                    // elements of other choices must not be there
                    !(
                        m_radius_choice1.is_initialized()
                        ||
                        m_materialUID_choice1.is_initialized()
                        ||
                        m_innerRadius_choice1.is_initialized()
                    )
                )
                == 1
            )
        )
        ;
    }

    const boost::optional<double>& CPACSStrutProperties::GetRadius_choice1() const
    {
        return m_radius_choice1;
    }

    void CPACSStrutProperties::SetRadius_choice1(const boost::optional<double>& value)
    {
        m_radius_choice1 = value;
    }

    const boost::optional<std::string>& CPACSStrutProperties::GetMaterialUID_choice1() const
    {
        return m_materialUID_choice1;
    }

    void CPACSStrutProperties::SetMaterialUID_choice1(const boost::optional<std::string>& value)
    {
        if (m_uidMgr) {
            if (m_materialUID_choice1 && !m_materialUID_choice1->empty()) m_uidMgr->TryUnregisterReference(*m_materialUID_choice1, *this);
            if (value && !value->empty()) m_uidMgr->RegisterReference(*value, *this);
        }
        m_materialUID_choice1 = value;
    }

    const boost::optional<double>& CPACSStrutProperties::GetInnerRadius_choice1() const
    {
        return m_innerRadius_choice1;
    }

    void CPACSStrutProperties::SetInnerRadius_choice1(const boost::optional<double>& value)
    {
        m_innerRadius_choice1 = value;
    }

    const boost::optional<std::string>& CPACSStrutProperties::GetStructuralElementUID_choice2() const
    {
        return m_structuralElementUID_choice2;
    }

    void CPACSStrutProperties::SetStructuralElementUID_choice2(const boost::optional<std::string>& value)
    {
        if (m_uidMgr) {
            if (m_structuralElementUID_choice2 && !m_structuralElementUID_choice2->empty()) m_uidMgr->TryUnregisterReference(*m_structuralElementUID_choice2, *this);
            if (value && !value->empty()) m_uidMgr->RegisterReference(*value, *this);
        }
        m_structuralElementUID_choice2 = value;
    }

    const CTiglUIDObject* CPACSStrutProperties::GetNextUIDObject() const
    {
        return GetNextUIDParent();
    }

    void CPACSStrutProperties::NotifyUIDChange(const std::string& oldUid, const std::string& newUid)
    {
        if (m_materialUID_choice1 && *m_materialUID_choice1 == oldUid) {
            m_materialUID_choice1 = newUid;
        }
        if (m_structuralElementUID_choice2 && *m_structuralElementUID_choice2 == oldUid) {
            m_structuralElementUID_choice2 = newUid;
        }
    }

} // namespace generated
} // namespace tigl