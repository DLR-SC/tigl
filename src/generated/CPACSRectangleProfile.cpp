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
#include "CPACSRectangleProfile.h"
#include "CPACSStandardProfile.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSRectangleProfile::CPACSRectangleProfile(CPACSStandardProfile* parent)
        : m_heightToWidthRatio(this)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSRectangleProfile::~CPACSRectangleProfile()
    {
    }

    const CPACSStandardProfile* CPACSRectangleProfile::GetParent() const
    {
        return m_parent;
    }

    CPACSStandardProfile* CPACSRectangleProfile::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSRectangleProfile::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSRectangleProfile::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    void CPACSRectangleProfile::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element cornerRadius
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/cornerRadius")) {
            m_cornerRadius = tixi::TixiGetElement<double>(tixiHandle, xpath + "/cornerRadius");
        }

        // read element heightToWidthRatio
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/heightToWidthRatio")) {
            m_heightToWidthRatio.ReadCPACS(tixiHandle, xpath + "/heightToWidthRatio");
        }
        else {
            LOG(ERROR) << "Required element heightToWidthRatio is missing at xpath " << xpath;
        }

    }

    void CPACSRectangleProfile::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element cornerRadius
        if (m_cornerRadius) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/cornerRadius");
            tixi::TixiSaveElement(tixiHandle, xpath + "/cornerRadius", *m_cornerRadius);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/cornerRadius")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/cornerRadius");
            }
        }

        // write element heightToWidthRatio
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/heightToWidthRatio");
        m_heightToWidthRatio.WriteCPACS(tixiHandle, xpath + "/heightToWidthRatio");

    }

    const boost::optional<double>& CPACSRectangleProfile::GetCornerRadius() const
    {
        return m_cornerRadius;
    }

    void CPACSRectangleProfile::SetCornerRadius(const boost::optional<double>& value)
    {
        m_cornerRadius = value;
    }

    const CPACSPosExcl0DoubleBase& CPACSRectangleProfile::GetHeightToWidthRatio() const
    {
        return m_heightToWidthRatio;
    }

    CPACSPosExcl0DoubleBase& CPACSRectangleProfile::GetHeightToWidthRatio()
    {
        return m_heightToWidthRatio;
    }

} // namespace generated
} // namespace tigl
