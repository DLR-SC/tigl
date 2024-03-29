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
#include "CPACSLateralCap.h"
#include "CPACSStructuralWallElement.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSLateralCap::CPACSLateralCap(CPACSStructuralWallElement* parent, CTiglUIDManager* uidMgr)
        : m_uidMgr(uidMgr)
        , m_area(0)
        , m_material(this, m_uidMgr)
    {
        //assert(parent != NULL);
        m_parent = parent;
    }

    CPACSLateralCap::~CPACSLateralCap()
    {
    }

    const CPACSStructuralWallElement* CPACSLateralCap::GetParent() const
    {
        return m_parent;
    }

    CPACSStructuralWallElement* CPACSLateralCap::GetParent()
    {
        return m_parent;
    }

    const CTiglUIDObject* CPACSLateralCap::GetNextUIDParent() const
    {
        if (m_parent) {
            if (m_parent->GetUID())
                return m_parent;
            else
                return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSLateralCap::GetNextUIDParent()
    {
        if (m_parent) {
            if (m_parent->GetUID())
                return m_parent;
            else
                return m_parent->GetNextUIDParent();
        }
        return nullptr;
    }

    CTiglUIDManager& CPACSLateralCap::GetUIDManager()
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    const CTiglUIDManager& CPACSLateralCap::GetUIDManager() const
    {
        if (!m_uidMgr) {
            throw CTiglError("UIDManager is null");
        }
        return *m_uidMgr;
    }

    void CPACSLateralCap::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element area
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/area")) {
            m_area = tixi::TixiGetElement<double>(tixiHandle, xpath + "/area");
        }
        else {
            LOG(ERROR) << "Required element area is missing at xpath " << xpath;
        }

        // read element material
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/material")) {
            m_material.ReadCPACS(tixiHandle, xpath + "/material");
        }
        else {
            LOG(ERROR) << "Required element material is missing at xpath " << xpath;
        }

        // read element placement
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/placement")) {
            m_placement = stringToCPACSLateralCap_placement(tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/placement"));
        }
        else {
            LOG(ERROR) << "Required element placement is missing at xpath " << xpath;
        }

    }

    void CPACSLateralCap::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element area
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/area");
        tixi::TixiSaveElement(tixiHandle, xpath + "/area", m_area);

        // write element material
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/material");
        m_material.WriteCPACS(tixiHandle, xpath + "/material");

        // write element placement
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/placement");
        tixi::TixiSaveElement(tixiHandle, xpath + "/placement", CPACSLateralCap_placementToString(m_placement));

    }

    const double& CPACSLateralCap::GetArea() const
    {
        return m_area;
    }

    void CPACSLateralCap::SetArea(const double& value)
    {
        m_area = value;
    }

    const CCPACSMaterialDefinition& CPACSLateralCap::GetMaterial() const
    {
        return m_material;
    }

    CCPACSMaterialDefinition& CPACSLateralCap::GetMaterial()
    {
        return m_material;
    }

    const CPACSLateralCap_placement& CPACSLateralCap::GetPlacement() const
    {
        return m_placement;
    }

    void CPACSLateralCap::SetPlacement(const CPACSLateralCap_placement& value)
    {
        m_placement = value;
    }

} // namespace generated
} // namespace tigl
