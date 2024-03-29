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
#include "CPACSSeatModule.h"
#include "CPACSSeatModules.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSSeatModule::CPACSSeatModule(CPACSSeatModules* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_numberOfSeats(0)
        , m_widthOfSeatModule(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSSeatModule::~CPACSSeatModule()
    {
        if (m_uidMgr) m_uidMgr->TryUnregisterObject(m_uID);
    }

    const CPACSSeatModules* CPACSSeatModule::GetParent() const
    {
        return m_parent;
    }

    CPACSSeatModules* CPACSSeatModule::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSSeatModule::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSSeatModule::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSSeatModule::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSSeatModule::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSSeatModule::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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

        // read element numberOfSeats
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/numberOfSeats")) {
            m_numberOfSeats = tixi::TixiGetElement<int>(tixiHandle, xpath + "/numberOfSeats");
        }
        else {
            LOG(ERROR) << "Required element numberOfSeats is missing at xpath " << xpath;
        }

        // read element widthOfSeatModule
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/widthOfSeatModule")) {
            m_widthOfSeatModule = tixi::TixiGetElement<double>(tixiHandle, xpath + "/widthOfSeatModule");
        }
        else {
            LOG(ERROR) << "Required element widthOfSeatModule is missing at xpath " << xpath;
        }

        // read element moduleMass
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/moduleMass")) {
            m_moduleMass = tixi::TixiGetElement<double>(tixiHandle, xpath + "/moduleMass");
        }

        // read element moduleCoG
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/moduleCoG")) {
            m_moduleCoG = boost::in_place(this, m_uidMgr);
            try {
                m_moduleCoG->ReadCPACS(tixiHandle, xpath + "/moduleCoG");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read moduleCoG at xpath " << xpath << ": " << e.what();
                m_moduleCoG = boost::none;
            }
        }

        if (m_uidMgr && !m_uID.empty()) m_uidMgr->RegisterObject(m_uID, *this);
    }

    void CPACSSeatModule::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
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

        // write element numberOfSeats
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/numberOfSeats");
        tixi::TixiSaveElement(tixiHandle, xpath + "/numberOfSeats", m_numberOfSeats);

        // write element widthOfSeatModule
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/widthOfSeatModule");
        tixi::TixiSaveElement(tixiHandle, xpath + "/widthOfSeatModule", m_widthOfSeatModule);

        // write element moduleMass
        if (m_moduleMass) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/moduleMass");
            tixi::TixiSaveElement(tixiHandle, xpath + "/moduleMass", *m_moduleMass);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/moduleMass")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/moduleMass");
            }
        }

        // write element moduleCoG
        if (m_moduleCoG) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/moduleCoG");
            m_moduleCoG->WriteCPACS(tixiHandle, xpath + "/moduleCoG");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/moduleCoG")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/moduleCoG");
            }
        }

    }

    const std::string& CPACSSeatModule::GetUID() const
    {
        return m_uID;
    }

    void CPACSSeatModule::SetUID(const std::string& value)
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

    const boost::optional<std::string>& CPACSSeatModule::GetName() const
    {
        return m_name;
    }

    void CPACSSeatModule::SetName(const boost::optional<std::string>& value)
    {
        m_name = value;
    }

    const boost::optional<std::string>& CPACSSeatModule::GetDescription() const
    {
        return m_description;
    }

    void CPACSSeatModule::SetDescription(const boost::optional<std::string>& value)
    {
        m_description = value;
    }

    const int& CPACSSeatModule::GetNumberOfSeats() const
    {
        return m_numberOfSeats;
    }

    void CPACSSeatModule::SetNumberOfSeats(const int& value)
    {
        m_numberOfSeats = value;
    }

    const double& CPACSSeatModule::GetWidthOfSeatModule() const
    {
        return m_widthOfSeatModule;
    }

    void CPACSSeatModule::SetWidthOfSeatModule(const double& value)
    {
        m_widthOfSeatModule = value;
    }

    const boost::optional<double>& CPACSSeatModule::GetModuleMass() const
    {
        return m_moduleMass;
    }

    void CPACSSeatModule::SetModuleMass(const boost::optional<double>& value)
    {
        m_moduleMass = value;
    }

    const boost::optional<CCPACSPoint>& CPACSSeatModule::GetModuleCoG() const
    {
        return m_moduleCoG;
    }

    boost::optional<CCPACSPoint>& CPACSSeatModule::GetModuleCoG()
    {
        return m_moduleCoG;
    }

    CCPACSPoint& CPACSSeatModule::GetModuleCoG(CreateIfNotExistsTag)
    {
        if (!m_moduleCoG)
            m_moduleCoG = boost::in_place(this, m_uidMgr);
        return *m_moduleCoG;
    }

    void CPACSSeatModule::RemoveModuleCoG()
    {
        m_moduleCoG = boost::none;
    }

} // namespace generated
} // namespace tigl
