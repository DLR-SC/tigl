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
#include "CCPACSNacelleProfile.h"
#include "CPACSCst2D.h"
#include "CPACSProfileGeometry.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSCst2D::CPACSCst2D(CPACSProfileGeometry* parent)
        : m_psi(reinterpret_cast<CCPACSWingProfileCST*>(this))
        , m_upperN1(0)
        , m_upperN2(0)
        , m_upperB(reinterpret_cast<CCPACSWingProfileCST*>(this))
        , m_lowerN1(0)
        , m_lowerN2(0)
        , m_lowerB(reinterpret_cast<CCPACSWingProfileCST*>(this))
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CPACSProfileGeometry);
    }

    CPACSCst2D::CPACSCst2D(CCPACSNacelleProfile* parent)
        : m_psi(reinterpret_cast<CCPACSWingProfileCST*>(this))
        , m_upperN1(0)
        , m_upperN2(0)
        , m_upperB(reinterpret_cast<CCPACSWingProfileCST*>(this))
        , m_lowerN1(0)
        , m_lowerN2(0)
        , m_lowerB(reinterpret_cast<CCPACSWingProfileCST*>(this))
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CCPACSNacelleProfile);
    }

    CPACSCst2D::~CPACSCst2D()
    {
    }

    const CTiglUIDObject* CPACSCst2D::GetNextUIDParent() const
    {
        if (m_parent) {
            if (IsParent<CPACSProfileGeometry>()) {
                return GetParent<CPACSProfileGeometry>();
            }
            if (IsParent<CCPACSNacelleProfile>()) {
                return GetParent<CCPACSNacelleProfile>();
            }
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSCst2D::GetNextUIDParent()
    {
        if (m_parent) {
            if (IsParent<CPACSProfileGeometry>()) {
                return GetParent<CPACSProfileGeometry>();
            }
            if (IsParent<CCPACSNacelleProfile>()) {
                return GetParent<CCPACSNacelleProfile>();
            }
        }
        return nullptr;
    }

    void CPACSCst2D::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element psi
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/psi")) {
            m_psi.ReadCPACS(tixiHandle, xpath + "/psi");
        }
        else {
            LOG(ERROR) << "Required element psi is missing at xpath " << xpath;
        }

        // read element upperN1
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/upperN1")) {
            m_upperN1 = tixi::TixiGetElement<double>(tixiHandle, xpath + "/upperN1");
        }
        else {
            LOG(ERROR) << "Required element upperN1 is missing at xpath " << xpath;
        }

        // read element upperN2
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/upperN2")) {
            m_upperN2 = tixi::TixiGetElement<double>(tixiHandle, xpath + "/upperN2");
        }
        else {
            LOG(ERROR) << "Required element upperN2 is missing at xpath " << xpath;
        }

        // read element upperB
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/upperB")) {
            m_upperB.ReadCPACS(tixiHandle, xpath + "/upperB");
        }
        else {
            LOG(ERROR) << "Required element upperB is missing at xpath " << xpath;
        }

        // read element lowerN1
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/lowerN1")) {
            m_lowerN1 = tixi::TixiGetElement<double>(tixiHandle, xpath + "/lowerN1");
        }
        else {
            LOG(ERROR) << "Required element lowerN1 is missing at xpath " << xpath;
        }

        // read element lowerN2
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/lowerN2")) {
            m_lowerN2 = tixi::TixiGetElement<double>(tixiHandle, xpath + "/lowerN2");
        }
        else {
            LOG(ERROR) << "Required element lowerN2 is missing at xpath " << xpath;
        }

        // read element lowerB
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/lowerB")) {
            m_lowerB.ReadCPACS(tixiHandle, xpath + "/lowerB");
        }
        else {
            LOG(ERROR) << "Required element lowerB is missing at xpath " << xpath;
        }

        // read element trailingEdgeThickness
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/trailingEdgeThickness")) {
            m_trailingEdgeThickness = tixi::TixiGetElement<double>(tixiHandle, xpath + "/trailingEdgeThickness");
        }

    }

    void CPACSCst2D::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element psi
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/psi");
        m_psi.WriteCPACS(tixiHandle, xpath + "/psi");

        // write element upperN1
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/upperN1");
        tixi::TixiSaveElement(tixiHandle, xpath + "/upperN1", m_upperN1);

        // write element upperN2
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/upperN2");
        tixi::TixiSaveElement(tixiHandle, xpath + "/upperN2", m_upperN2);

        // write element upperB
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/upperB");
        m_upperB.WriteCPACS(tixiHandle, xpath + "/upperB");

        // write element lowerN1
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/lowerN1");
        tixi::TixiSaveElement(tixiHandle, xpath + "/lowerN1", m_lowerN1);

        // write element lowerN2
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/lowerN2");
        tixi::TixiSaveElement(tixiHandle, xpath + "/lowerN2", m_lowerN2);

        // write element lowerB
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/lowerB");
        m_lowerB.WriteCPACS(tixiHandle, xpath + "/lowerB");

        // write element trailingEdgeThickness
        if (m_trailingEdgeThickness) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/trailingEdgeThickness");
            tixi::TixiSaveElement(tixiHandle, xpath + "/trailingEdgeThickness", *m_trailingEdgeThickness);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/trailingEdgeThickness")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/trailingEdgeThickness");
            }
        }

    }

    const CCPACSStringVector& CPACSCst2D::GetPsi() const
    {
        return m_psi;
    }

    CCPACSStringVector& CPACSCst2D::GetPsi()
    {
        return m_psi;
    }

    const double& CPACSCst2D::GetUpperN1() const
    {
        return m_upperN1;
    }

    void CPACSCst2D::SetUpperN1(const double& value)
    {
        m_upperN1 = value;
    }

    const double& CPACSCst2D::GetUpperN2() const
    {
        return m_upperN2;
    }

    void CPACSCst2D::SetUpperN2(const double& value)
    {
        m_upperN2 = value;
    }

    const CCPACSStringVector& CPACSCst2D::GetUpperB() const
    {
        return m_upperB;
    }

    CCPACSStringVector& CPACSCst2D::GetUpperB()
    {
        return m_upperB;
    }

    const double& CPACSCst2D::GetLowerN1() const
    {
        return m_lowerN1;
    }

    void CPACSCst2D::SetLowerN1(const double& value)
    {
        m_lowerN1 = value;
    }

    const double& CPACSCst2D::GetLowerN2() const
    {
        return m_lowerN2;
    }

    void CPACSCst2D::SetLowerN2(const double& value)
    {
        m_lowerN2 = value;
    }

    const CCPACSStringVector& CPACSCst2D::GetLowerB() const
    {
        return m_lowerB;
    }

    CCPACSStringVector& CPACSCst2D::GetLowerB()
    {
        return m_lowerB;
    }

    const boost::optional<double>& CPACSCst2D::GetTrailingEdgeThickness() const
    {
        return m_trailingEdgeThickness;
    }

    void CPACSCst2D::SetTrailingEdgeThickness(const boost::optional<double>& value)
    {
        m_trailingEdgeThickness = value;
    }

} // namespace generated
} // namespace tigl
