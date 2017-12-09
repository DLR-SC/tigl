// Copyright (c) 2016 RISC Software GmbH
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

#include <CCPACSWingSection.h>
#include "CPACSWingSections.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "TixiHelper.h"

namespace tigl
{
    namespace generated
    {
        CPACSWingSections::CPACSWingSections(CTiglUIDManager* uidMgr) :
            m_uidMgr(uidMgr) {}
        
        CPACSWingSections::~CPACSWingSections() {}
        
        CTiglUIDManager& CPACSWingSections::GetUIDManager()
        {
            return *m_uidMgr;
        }
        
        const CTiglUIDManager& CPACSWingSections::GetUIDManager() const
        {
            return *m_uidMgr;
        }
        
        void CPACSWingSections::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
        {
            // read element section
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/section")) {
                tixi::TixiReadElements(tixiHandle, xpath + "/section", m_sections, m_uidMgr);
            }
            
        }
        
        void CPACSWingSections::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
        {
            // write element section
            tixi::TixiSaveElements(tixiHandle, xpath + "/section", m_sections);
            
        }
        
        const std::vector<unique_ptr<CCPACSWingSection> >& CPACSWingSections::GetSections() const
        {
            return m_sections;
        }
        
        std::vector<unique_ptr<CCPACSWingSection> >& CPACSWingSections::GetSections()
        {
            return m_sections;
        }
        
        CCPACSWingSection& CPACSWingSections::AddSection()
        {
            m_sections.push_back(make_unique<CCPACSWingSection>(m_uidMgr));
            return *m_sections.back();
        }
        
        void CPACSWingSections::RemoveSection(CCPACSWingSection& ref)
        {
            for (std::size_t i = 0; i < m_sections.size(); i++) {
                if (m_sections[i].get() == &ref) {
                    m_sections.erase(m_sections.begin() + i);
                    return;
                }
            }
            throw CTiglError("Element not found");
        }
        
    }
}