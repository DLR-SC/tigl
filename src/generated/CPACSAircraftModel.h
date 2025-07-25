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

#pragma once

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <CCPACSACSystems.h>
#include <CCPACSDucts.h>
#include <CCPACSEnginePositions.h>
#include <CCPACSExternalObjects.h>
#include <CCPACSWings.h>
#include <string>
#include <tixi.h>
#include "CPACSEnginePylons.h"
#include "CPACSFuelTanks.h"
#include "CPACSFuselages.h"
#include "CPACSLandingGears.h"
#include "CPACSVehicleConfigurations.h"
#include "CreateIfNotExists.h"
#include "CTiglUIDObject.h"
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDManager;

namespace generated
{
    class CPACSAircraft;

    // This class is used in:
    // CPACSAircraft

    /// @brief Aircraft model
    /// 
    /// The aircraftModelType contains the geometric aircraft
    /// model and associated data.
    /// Elements specifying the geometry of the aircraft are fuselages , wings , engines (referenced via uID ), enginePylons , landingGear , systems (to some extend) and genericGeometryComponents .
    /// Other elements are dedicated to additional data associated to this aircraft model. Brief and concise analysis results are stored
    /// in the global node. The analysis node contains
    /// extensive results from multidisciplinary analysis modules.
    /// In the current CPACS version requirements only refer to the aircraft performance and are therefore specified in the performanceRequirements node.
    /// 
    class CPACSAircraftModel : public CTiglReqUIDObject
    {
    public:
        TIGL_EXPORT CPACSAircraftModel(CPACSAircraft* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT virtual ~CPACSAircraftModel();

        TIGL_EXPORT CPACSAircraft* GetParent();

        TIGL_EXPORT const CPACSAircraft* GetParent() const;

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT CTiglUIDManager& GetUIDManager();
        TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const std::string& GetUID() const;
        TIGL_EXPORT virtual void SetUID(const std::string& value);

        TIGL_EXPORT virtual const std::string& GetName() const;
        TIGL_EXPORT virtual void SetName(const std::string& value);

        TIGL_EXPORT virtual const boost::optional<std::string>& GetDescription() const;
        TIGL_EXPORT virtual void SetDescription(const boost::optional<std::string>& value);

        TIGL_EXPORT virtual const boost::optional<CCPACSDucts>& GetDucts() const;
        TIGL_EXPORT virtual boost::optional<CCPACSDucts>& GetDucts();

        TIGL_EXPORT virtual const boost::optional<CPACSFuselages>& GetFuselages() const;
        TIGL_EXPORT virtual boost::optional<CPACSFuselages>& GetFuselages();

        TIGL_EXPORT virtual const boost::optional<CCPACSWings>& GetWings() const;
        TIGL_EXPORT virtual boost::optional<CCPACSWings>& GetWings();

        TIGL_EXPORT virtual const boost::optional<CCPACSEnginePositions>& GetEngines() const;
        TIGL_EXPORT virtual boost::optional<CCPACSEnginePositions>& GetEngines();

        TIGL_EXPORT virtual const boost::optional<CPACSEnginePylons>& GetEnginePylons() const;
        TIGL_EXPORT virtual boost::optional<CPACSEnginePylons>& GetEnginePylons();

        TIGL_EXPORT virtual const boost::optional<CPACSLandingGears>& GetLandingGears() const;
        TIGL_EXPORT virtual boost::optional<CPACSLandingGears>& GetLandingGears();

        TIGL_EXPORT virtual const boost::optional<CPACSFuelTanks>& GetFuelTanks() const;
        TIGL_EXPORT virtual boost::optional<CPACSFuelTanks>& GetFuelTanks();

        TIGL_EXPORT virtual const boost::optional<CCPACSACSystems>& GetSystems() const;
        TIGL_EXPORT virtual boost::optional<CCPACSACSystems>& GetSystems();

        TIGL_EXPORT virtual const boost::optional<CCPACSExternalObjects>& GetGenericGeometryComponents() const;
        TIGL_EXPORT virtual boost::optional<CCPACSExternalObjects>& GetGenericGeometryComponents();

        TIGL_EXPORT virtual const boost::optional<CPACSVehicleConfigurations>& GetConfigurations() const;
        TIGL_EXPORT virtual boost::optional<CPACSVehicleConfigurations>& GetConfigurations();

        TIGL_EXPORT virtual CCPACSDucts& GetDucts(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveDucts();

        TIGL_EXPORT virtual CPACSFuselages& GetFuselages(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveFuselages();

        TIGL_EXPORT virtual CCPACSWings& GetWings(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveWings();

        TIGL_EXPORT virtual CCPACSEnginePositions& GetEngines(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveEngines();

        TIGL_EXPORT virtual CPACSEnginePylons& GetEnginePylons(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveEnginePylons();

        TIGL_EXPORT virtual CPACSLandingGears& GetLandingGears(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveLandingGears();

        TIGL_EXPORT virtual CPACSFuelTanks& GetFuelTanks(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveFuelTanks();

        TIGL_EXPORT virtual CCPACSACSystems& GetSystems(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveSystems();

        TIGL_EXPORT virtual CCPACSExternalObjects& GetGenericGeometryComponents(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveGenericGeometryComponents();

        TIGL_EXPORT virtual CPACSVehicleConfigurations& GetConfigurations(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveConfigurations();

    protected:
        CPACSAircraft* m_parent;

        CTiglUIDManager* m_uidMgr;

        std::string                                 m_uID;

        /// Name of the aircraft model
        std::string                                 m_name;

        /// Description of the aircraft model
        boost::optional<std::string>                m_description;

        boost::optional<CCPACSDucts>                m_ducts;

        boost::optional<CPACSFuselages>             m_fuselages;

        boost::optional<CCPACSWings>                m_wings;

        boost::optional<CCPACSEnginePositions>      m_engines;

        boost::optional<CPACSEnginePylons>          m_enginePylons;

        boost::optional<CPACSLandingGears>          m_landingGears;

        boost::optional<CPACSFuelTanks>             m_fuelTanks;

        boost::optional<CCPACSACSystems>            m_systems;

        boost::optional<CCPACSExternalObjects>      m_genericGeometryComponents;

        boost::optional<CPACSVehicleConfigurations> m_configurations;

    private:
        CPACSAircraftModel(const CPACSAircraftModel&) = delete;
        CPACSAircraftModel& operator=(const CPACSAircraftModel&) = delete;

        CPACSAircraftModel(CPACSAircraftModel&&) = delete;
        CPACSAircraftModel& operator=(CPACSAircraftModel&&) = delete;
    };
} // namespace generated

// CPACSAircraftModel is customized, use type CCPACSAircraftModel directly

// Aliases in tigl namespace
using CCPACSAircraft = generated::CPACSAircraft;
} // namespace tigl
