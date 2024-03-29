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
#include "CPACSVehicleConfiguration.h"
#include "CPACSVehicleConfigurations.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSVehicleConfiguration::CPACSVehicleConfiguration(CPACSVehicleConfigurations* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSVehicleConfiguration::~CPACSVehicleConfiguration()
    {
        if (m_uidMgr) m_uidMgr->TryUnregisterObject(m_uID);
    }

    const CPACSVehicleConfigurations* CPACSVehicleConfiguration::GetParent() const
    {
        return m_parent;
    }

    CPACSVehicleConfigurations* CPACSVehicleConfiguration::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSVehicleConfiguration::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSVehicleConfiguration::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSVehicleConfiguration::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSVehicleConfiguration::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSVehicleConfiguration::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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
            if (m_name.empty()) {
                LOG(WARNING) << "Required element name is empty at xpath " << xpath;
            }
        }
        else {
            LOG(ERROR) << "Required element name is missing at xpath " << xpath;
        }

        // read element description
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/description")) {
            m_description = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/description");
            if (m_description->empty()) {
                LOG(WARNING) << "Optional element description is present but empty at xpath " << xpath;
            }
        }

        // read element controlElements
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/controlElements")) {
            m_controlElements = boost::in_place(this, m_uidMgr);
            try {
                m_controlElements->ReadCPACS(tixiHandle, xpath + "/controlElements");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read controlElements at xpath " << xpath << ": " << e.what();
                m_controlElements = boost::none;
            }
        }

        // read element internalPressures
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/internalPressures")) {
            m_internalPressures = boost::in_place(this, m_uidMgr);
            try {
                m_internalPressures->ReadCPACS(tixiHandle, xpath + "/internalPressures");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read internalPressures at xpath " << xpath << ": " << e.what();
                m_internalPressures = boost::none;
            }
        }

        if (m_uidMgr && !m_uID.empty()) m_uidMgr->RegisterObject(m_uID, *this);
    }

    void CPACSVehicleConfiguration::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write attribute uID
        tixi::TixiSaveAttribute(tixiHandle, xpath, "uID", m_uID);

        // write element name
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/name");
        tixi::TixiSaveElement(tixiHandle, xpath + "/name", m_name);

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

        // write element controlElements
        if (m_controlElements) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/controlElements");
            m_controlElements->WriteCPACS(tixiHandle, xpath + "/controlElements");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/controlElements")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/controlElements");
            }
        }

        // write element internalPressures
        if (m_internalPressures) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/internalPressures");
            m_internalPressures->WriteCPACS(tixiHandle, xpath + "/internalPressures");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/internalPressures")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/internalPressures");
            }
        }

    }

    const std::string& CPACSVehicleConfiguration::GetUID() const
    {
        return m_uID;
    }

    void CPACSVehicleConfiguration::SetUID(const std::string& value)
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

    const std::string& CPACSVehicleConfiguration::GetName() const
    {
        return m_name;
    }

    void CPACSVehicleConfiguration::SetName(const std::string& value)
    {
        m_name = value;
    }

    const boost::optional<std::string>& CPACSVehicleConfiguration::GetDescription() const
    {
        return m_description;
    }

    void CPACSVehicleConfiguration::SetDescription(const boost::optional<std::string>& value)
    {
        m_description = value;
    }

    const boost::optional<CPACSAircraftControlElements>& CPACSVehicleConfiguration::GetControlElements() const
    {
        return m_controlElements;
    }

    boost::optional<CPACSAircraftControlElements>& CPACSVehicleConfiguration::GetControlElements()
    {
        return m_controlElements;
    }

    const boost::optional<CPACSInternalPressures>& CPACSVehicleConfiguration::GetInternalPressures() const
    {
        return m_internalPressures;
    }

    boost::optional<CPACSInternalPressures>& CPACSVehicleConfiguration::GetInternalPressures()
    {
        return m_internalPressures;
    }

    CPACSAircraftControlElements& CPACSVehicleConfiguration::GetControlElements(CreateIfNotExistsTag)
    {
        if (!m_controlElements)
            m_controlElements = boost::in_place(this, m_uidMgr);
        return *m_controlElements;
    }

    void CPACSVehicleConfiguration::RemoveControlElements()
    {
        m_controlElements = boost::none;
    }

    CPACSInternalPressures& CPACSVehicleConfiguration::GetInternalPressures(CreateIfNotExistsTag)
    {
        if (!m_internalPressures)
            m_internalPressures = boost::in_place(this, m_uidMgr);
        return *m_internalPressures;
    }

    void CPACSVehicleConfiguration::RemoveInternalPressures()
    {
        m_internalPressures = boost::none;
    }

} // namespace generated
} // namespace tigl
