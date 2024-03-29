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
#include "CCPACSControlSurfaceBorderTrailingEdge.h"
#include "CPACSLeadingEdgeShape.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSLeadingEdgeShape::CPACSLeadingEdgeShape(CCPACSControlSurfaceBorderTrailingEdge* parent)
        : m_relHeightLE(0)
        , m_xsiUpperSkin(0)
        , m_xsiLowerSkin(0)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSLeadingEdgeShape::~CPACSLeadingEdgeShape()
    {
    }

    const CCPACSControlSurfaceBorderTrailingEdge* CPACSLeadingEdgeShape::GetParent() const
    {
        return m_parent;
    }

    CCPACSControlSurfaceBorderTrailingEdge* CPACSLeadingEdgeShape::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSLeadingEdgeShape::GetNextUIDParent() const
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSLeadingEdgeShape::GetNextUIDParent()
    {
        if (m_parent) {
            return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    void CPACSLeadingEdgeShape::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element relHeightLE
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/relHeightLE")) {
            m_relHeightLE = tixi::TixiGetElement<double>(tixiHandle, xpath + "/relHeightLE");
        }
        else {
            LOG(ERROR) << "Required element relHeightLE is missing at xpath " << xpath;
        }

        // read element xsiUpperSkin
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/xsiUpperSkin")) {
            m_xsiUpperSkin = tixi::TixiGetElement<double>(tixiHandle, xpath + "/xsiUpperSkin");
        }
        else {
            LOG(ERROR) << "Required element xsiUpperSkin is missing at xpath " << xpath;
        }

        // read element xsiLowerSkin
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/xsiLowerSkin")) {
            m_xsiLowerSkin = tixi::TixiGetElement<double>(tixiHandle, xpath + "/xsiLowerSkin");
        }
        else {
            LOG(ERROR) << "Required element xsiLowerSkin is missing at xpath " << xpath;
        }

    }

    void CPACSLeadingEdgeShape::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element relHeightLE
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/relHeightLE");
        tixi::TixiSaveElement(tixiHandle, xpath + "/relHeightLE", m_relHeightLE);

        // write element xsiUpperSkin
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/xsiUpperSkin");
        tixi::TixiSaveElement(tixiHandle, xpath + "/xsiUpperSkin", m_xsiUpperSkin);

        // write element xsiLowerSkin
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/xsiLowerSkin");
        tixi::TixiSaveElement(tixiHandle, xpath + "/xsiLowerSkin", m_xsiLowerSkin);

    }

    const double& CPACSLeadingEdgeShape::GetRelHeightLE() const
    {
        return m_relHeightLE;
    }

    void CPACSLeadingEdgeShape::SetRelHeightLE(const double& value)
    {
        m_relHeightLE = value;
    }

    const double& CPACSLeadingEdgeShape::GetXsiUpperSkin() const
    {
        return m_xsiUpperSkin;
    }

    void CPACSLeadingEdgeShape::SetXsiUpperSkin(const double& value)
    {
        m_xsiUpperSkin = value;
    }

    const double& CPACSLeadingEdgeShape::GetXsiLowerSkin() const
    {
        return m_xsiLowerSkin;
    }

    void CPACSLeadingEdgeShape::SetXsiLowerSkin(const double& value)
    {
        m_xsiLowerSkin = value;
    }

} // namespace generated
} // namespace tigl
