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
#include <CCPACSRotorHinge.h>
#include "CCPACSRotorBladeAttachment.h"
#include "CPACSRotorHubHinges.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSRotorHubHinges::CPACSRotorHubHinges(CCPACSRotorBladeAttachment* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSRotorHubHinges::~CPACSRotorHubHinges()
    {
    }

    const CCPACSRotorBladeAttachment* CPACSRotorHubHinges::GetParent() const
    {
        return m_parent;
    }

    CCPACSRotorBladeAttachment* CPACSRotorHubHinges::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSRotorHubHinges::GetNextUIDParent() const
    {
        return m_parent;
    }

    CTiglUIDObject* CPACSRotorHubHinges::GetNextUIDParent()
    {
        return m_parent;
    }

    CTiglUIDManager& CPACSRotorHubHinges::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSRotorHubHinges::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSRotorHubHinges::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element hinge
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/hinge")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/hinge", m_hinges, 1, tixi::xsdUnbounded, reinterpret_cast<CCPACSRotorHinges*>(this), m_uidMgr);
        }

    }

    void CPACSRotorHubHinges::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element hinge
        tixi::TixiSaveElements(tixiHandle, xpath + "/hinge", m_hinges);

    }

    const std::vector<std::unique_ptr<CCPACSRotorHinge>>& CPACSRotorHubHinges::GetHinges() const
    {
        return m_hinges;
    }

    std::vector<std::unique_ptr<CCPACSRotorHinge>>& CPACSRotorHubHinges::GetHinges()
    {
        return m_hinges;
    }

    CCPACSRotorHinge& CPACSRotorHubHinges::AddHinge()
    {
        m_hinges.push_back(make_unique<CCPACSRotorHinge>(reinterpret_cast<CCPACSRotorHinges*>(this), m_uidMgr));
        return *m_hinges.back();
    }

    void CPACSRotorHubHinges::RemoveHinge(CCPACSRotorHinge& ref)
    {
        for (std::size_t i = 0; i < m_hinges.size(); i++) {
            if (m_hinges[i].get() == &ref) {
                m_hinges.erase(m_hinges.begin() + i);
                return;
            }
        }
        throw CTiglError("Element not found");
    }

} // namespace generated
} // namespace tigl
