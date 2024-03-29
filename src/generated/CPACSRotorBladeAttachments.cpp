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
#include <CCPACSRotorBladeAttachment.h>
#include "CCPACSRotorHub.h"
#include "CPACSRotorBladeAttachments.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSRotorBladeAttachments::CPACSRotorBladeAttachments(CCPACSRotorHub* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSRotorBladeAttachments::~CPACSRotorBladeAttachments()
    {
    }

    const CCPACSRotorHub* CPACSRotorBladeAttachments::GetParent() const
    {
        return m_parent;
    }

    CCPACSRotorHub* CPACSRotorBladeAttachments::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSRotorBladeAttachments::GetNextUIDParent() const
    {
        return m_parent;
    }

    CTiglUIDObject* CPACSRotorBladeAttachments::GetNextUIDParent()
    {
        return m_parent;
    }

    CTiglUIDManager& CPACSRotorBladeAttachments::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSRotorBladeAttachments::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSRotorBladeAttachments::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element rotorBladeAttachment
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/rotorBladeAttachment")) {
            tixi::TixiReadElements(tixiHandle, xpath + "/rotorBladeAttachment", m_rotorBladeAttachments, 1, tixi::xsdUnbounded, reinterpret_cast<CCPACSRotorBladeAttachments*>(this), m_uidMgr);
        }

    }

    void CPACSRotorBladeAttachments::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element rotorBladeAttachment
        tixi::TixiSaveElements(tixiHandle, xpath + "/rotorBladeAttachment", m_rotorBladeAttachments);

    }

    const std::vector<std::unique_ptr<CCPACSRotorBladeAttachment>>& CPACSRotorBladeAttachments::GetRotorBladeAttachments() const
    {
        return m_rotorBladeAttachments;
    }

    std::vector<std::unique_ptr<CCPACSRotorBladeAttachment>>& CPACSRotorBladeAttachments::GetRotorBladeAttachments()
    {
        return m_rotorBladeAttachments;
    }

    CCPACSRotorBladeAttachment& CPACSRotorBladeAttachments::AddRotorBladeAttachment()
    {
        m_rotorBladeAttachments.push_back(make_unique<CCPACSRotorBladeAttachment>(reinterpret_cast<CCPACSRotorBladeAttachments*>(this), m_uidMgr));
        return *m_rotorBladeAttachments.back();
    }

    void CPACSRotorBladeAttachments::RemoveRotorBladeAttachment(CCPACSRotorBladeAttachment& ref)
    {
        for (std::size_t i = 0; i < m_rotorBladeAttachments.size(); i++) {
            if (m_rotorBladeAttachments[i].get() == &ref) {
                m_rotorBladeAttachments.erase(m_rotorBladeAttachments.begin() + i);
                return;
            }
        }
        throw CTiglError("Element not found");
    }

} // namespace generated
} // namespace tigl
