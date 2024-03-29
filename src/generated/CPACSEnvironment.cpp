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
#include "CPACSEnvironment.h"
#include "CPACSGlobalFlightPoint.h"
#include "CPACSPointPerformanceDefinition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglUIDObject.h"
#include "TixiHelper.h"

namespace tigl
{
namespace generated
{
    CPACSEnvironment::CPACSEnvironment(CPACSGlobalFlightPoint* parent)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CPACSGlobalFlightPoint);
    }

    CPACSEnvironment::CPACSEnvironment(CPACSPointPerformanceDefinition* parent)
    {
        //assert(parent != NULL);
        m_parent = parent;
        m_parentType = &typeid(CPACSPointPerformanceDefinition);
    }

    CPACSEnvironment::~CPACSEnvironment()
    {
    }

    const CTiglUIDObject* CPACSEnvironment::GetNextUIDParent() const
    {
        if (m_parent) {
            if (IsParent<CPACSGlobalFlightPoint>()) {
                return GetParent<CPACSGlobalFlightPoint>();
            }
            if (IsParent<CPACSPointPerformanceDefinition>()) {
                return GetParent<CPACSPointPerformanceDefinition>();
            }
        }
        return nullptr;
    }

    CTiglUIDObject* CPACSEnvironment::GetNextUIDParent()
    {
        if (m_parent) {
            if (IsParent<CPACSGlobalFlightPoint>()) {
                return GetParent<CPACSGlobalFlightPoint>();
            }
            if (IsParent<CPACSPointPerformanceDefinition>()) {
                return GetParent<CPACSPointPerformanceDefinition>();
            }
        }
        return nullptr;
    }

    void CPACSEnvironment::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
    {
        // read element atmosphericModel
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/atmosphericModel")) {
            m_atmosphericModel = stringToCPACSAtmosphericModel(tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/atmosphericModel"));
        }
        else {
            LOG(ERROR) << "Required element atmosphericModel is missing at xpath " << xpath;
        }

        // read element deltaTemperature
        if (tixi::TixiCheckElement(tixiHandle, xpath + "/deltaTemperature")) {
            m_deltaTemperature = tixi::TixiGetElement<double>(tixiHandle, xpath + "/deltaTemperature");
        }

    }

    void CPACSEnvironment::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
    {
        // write element atmosphericModel
        tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/atmosphericModel");
        tixi::TixiSaveElement(tixiHandle, xpath + "/atmosphericModel", CPACSAtmosphericModelToString(m_atmosphericModel));

        // write element deltaTemperature
        if (m_deltaTemperature) {
            tixi::TixiCreateElementIfNotExists(tixiHandle, xpath + "/deltaTemperature");
            tixi::TixiSaveElement(tixiHandle, xpath + "/deltaTemperature", *m_deltaTemperature);
        }
        else {
            if (tixi::TixiCheckElement(tixiHandle, xpath + "/deltaTemperature")) {
                tixi::TixiRemoveElement(tixiHandle, xpath + "/deltaTemperature");
            }
        }

    }

    const CPACSAtmosphericModel& CPACSEnvironment::GetAtmosphericModel() const
    {
        return m_atmosphericModel;
    }

    void CPACSEnvironment::SetAtmosphericModel(const CPACSAtmosphericModel& value)
    {
        m_atmosphericModel = value;
    }

    const boost::optional<double>& CPACSEnvironment::GetDeltaTemperature() const
    {
        return m_deltaTemperature;
    }

    void CPACSEnvironment::SetDeltaTemperature(const boost::optional<double>& value)
    {
        m_deltaTemperature = value;
    }

} // namespace generated
} // namespace tigl
