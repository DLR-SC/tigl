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
#include "CPACSGalleyElement.h"
#include "CPACSGalleyElements.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSGalleyElement::CPACSGalleyElement(CPACSGalleyElements* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_geometry(this, m_uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSGalleyElement::~CPACSGalleyElement()
    {
        if (m_uidMgr) m_uidMgr->TryUnregisterObject(m_uID);
    }

    const CPACSGalleyElements* CPACSGalleyElement::GetParent() const
    {
        return m_parent;
    }

    CPACSGalleyElements* CPACSGalleyElement::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSGalleyElement::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSGalleyElement::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSGalleyElement::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSGalleyElement::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSGalleyElement::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
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

        // read element description
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/description")) {
            m_description = tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/description");
            if (m_description->empty()) {
                LOG(WARNING) << "Optional element description is present but empty at xpath " << xpath;
            }
        }

        // read element geometry
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/geometry")) {
            m_geometry.ReadCPACS(tixiHandle, xpath + "/geometry");
        }
        else {
            LOG(ERROR) << "Required element geometry is missing at xpath " << xpath;
        }

        // read element mass
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/mass")) {
            m_mass = boost::in_place(this, m_uidMgr);
            try {
                m_mass->ReadCPACS(tixiHandle, xpath + "/mass");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read mass at xpath " << xpath << ": " << e.what();
                m_mass = boost::none;
            }
        }

        // read element numberOfTrolleys
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/numberOfTrolleys")) {
            m_numberOfTrolleys = boost::in_place(this);
            try {
                m_numberOfTrolleys->ReadCPACS(tixiHandle, xpath + "/numberOfTrolleys");
            } catch(const std::exception& e) {
                LOG(ERROR) << "Failed to read numberOfTrolleys at xpath " << xpath << ": " << e.what();
                m_numberOfTrolleys = boost::none;
            }
        }

        if (m_uidMgr && !m_uID.empty()) m_uidMgr->RegisterObject(m_uID, *this);
    }

    void CPACSGalleyElement::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write attribute uID
        tixi::TixiSaveAttribute(tixiHandle, xpath, "uID", m_uID);

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

        // write element geometry
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/geometry");
        m_geometry.WriteCPACS(tixiHandle, xpath + "/geometry");

        // write element mass
        if (m_mass) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/mass");
            m_mass->WriteCPACS(tixiHandle, xpath + "/mass");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/mass")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/mass");
            }
        }

        // write element numberOfTrolleys
        if (m_numberOfTrolleys) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/numberOfTrolleys");
            m_numberOfTrolleys->WriteCPACS(tixiHandle, xpath + "/numberOfTrolleys");
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/numberOfTrolleys")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/numberOfTrolleys");
            }
        }

    }

    const std::string& CPACSGalleyElement::GetUID() const
    {
        return m_uID;
    }

    void CPACSGalleyElement::SetUID(const std::string& value)
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

    const boost::optional<std::string>& CPACSGalleyElement::GetDescription() const
    {
        return m_description;
    }

    void CPACSGalleyElement::SetDescription(const boost::optional<std::string>& value)
    {
        m_description = value;
    }

    const CPACSDeckElementGeometry& CPACSGalleyElement::GetGeometry() const
    {
        return m_geometry;
    }

    CPACSDeckElementGeometry& CPACSGalleyElement::GetGeometry()
    {
        return m_geometry;
    }

    const boost::optional<CPACSDeckElementMass>& CPACSGalleyElement::GetMass() const
    {
        return m_mass;
    }

    boost::optional<CPACSDeckElementMass>& CPACSGalleyElement::GetMass()
    {
        return m_mass;
    }

    const boost::optional<CPACSPosExcl0IntBase>& CPACSGalleyElement::GetNumberOfTrolleys() const
    {
        return m_numberOfTrolleys;
    }

    boost::optional<CPACSPosExcl0IntBase>& CPACSGalleyElement::GetNumberOfTrolleys()
    {
        return m_numberOfTrolleys;
    }

    CPACSDeckElementMass& CPACSGalleyElement::GetMass(CreateIfNotExistsTag)
    {
        if (!m_mass)
            m_mass = boost::in_place(this, m_uidMgr);
        return *m_mass;
    }

    void CPACSGalleyElement::RemoveMass()
    {
        m_mass = boost::none;
    }

    CPACSPosExcl0IntBase& CPACSGalleyElement::GetNumberOfTrolleys(CreateIfNotExistsTag)
    {
        if (!m_numberOfTrolleys)
            m_numberOfTrolleys = boost::in_place(this);
        return *m_numberOfTrolleys;
    }

    void CPACSGalleyElement::RemoveNumberOfTrolleys()
    {
        m_numberOfTrolleys = boost::none;
    }

} // namespace generated
} // namespace tigl