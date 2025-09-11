/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file
* @brief  Implementation of CPACS configuration handling routines.
*/

#ifndef CCPACSCONFIGURATION_H
#define CCPACSCONFIGURATION_H

#include "tigl_internal.h"

#include "CTiglUIDManager.h"
#include "CTiglLogging.h"
#include "CCPACSAircraftModel.h"
#include "CCPACSRotorcraftModel.h"
#include "generated/CPACSHeader.h"
#include "CCPACSWings.h"
#include "CCPACSRotors.h"
#include "generated/CPACSFuselages.h"
#include "CCPACSACSystems.h"
#include "CCPACSFarField.h"
#include "generated/CPACSGuideCurveProfiles.h"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"
#include "CTiglShapeCache.h"
#include "CTiglMemoryPool.h"
#include "CSharedPtr.h"
#include "generated/CPACSProfiles.h"
#include "generated/CPACSEngines.h"


namespace tigl
{

class CTiglFusePlane;
typedef CSharedPtr<CTiglFusePlane> PTiglFusePlane;

class CCPACSConfiguration
{

public:
    // Constructor
    TIGL_EXPORT CCPACSConfiguration(TixiDocumentHandle tixiHandle);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSConfiguration();

    // Invalidates the internal state of the configuration and forces
    // recalculation of wires, lofts etc.
    TIGL_EXPORT void Invalidate();

    // Read CPACS configuration
    TIGL_EXPORT void ReadCPACS(const std::string& configurationUID);

    // Write CPACS configuration
    TIGL_EXPORT void WriteCPACS(const std::string& configurationUID);

    // Returns the underlying tixi document handle used by a CPACS configuration
    TIGL_EXPORT TixiDocumentHandle GetTixiDocumentHandle() const;

    // Returns whether this configuration is a rotorcraft
    TIGL_EXPORT bool IsRotorcraft() const;

    // Returns true if a wing or rotor profile with the given uid exists
    TIGL_EXPORT bool HasWingProfile(std::string uid) const;

    // Returns the class which holds all wing profiles
    TIGL_EXPORT boost::optional<const CCPACSWingProfiles&> GetWingProfiles() const;
    TIGL_EXPORT boost::optional<CCPACSWingProfiles&> GetWingProfiles();
    
    // Returns the class which holds all rotor profiles
    TIGL_EXPORT boost::optional<CCPACSRotorProfiles&> GetRotorProfiles();

    // Returns the class which holds all wing profiles
    TIGL_EXPORT boost::optional<CCPACSFuselageProfiles&> GetFuselageProfiles();

    // Returns the profiles
    TIGL_EXPORT boost::optional<CCPACSProfiles>& GetProfiles();

    // Returns the wing or rotor profile for a given uid.
    TIGL_EXPORT const CCPACSWingProfile& GetWingProfile(std::string uid) const;
    TIGL_EXPORT CCPACSWingProfile& GetWingProfile(std::string uid);

    // Returns the total count of wings (including rotor blades) in a configuration
    TIGL_EXPORT size_t GetWingCount() const;

    // Returns the count of rotor blade wings in a configuration
    TIGL_EXPORT size_t GetRotorBladeCount() const;

    // Returns the wing (or rotor blade) for a given index.
    TIGL_EXPORT const CCPACSWing& GetWing(size_t index) const ;
    TIGL_EXPORT CCPACSWing& GetWing(size_t index);

    // Returns the wing (or rotor blade) for a given UID.
    TIGL_EXPORT const CCPACSWing& GetWing(const std::string& UID) const;
    TIGL_EXPORT CCPACSWing& GetWing(const std::string& UID);

    // Returns the wing (or rotor blade) index for a given UID.
    TIGL_EXPORT size_t GetWingIndex(const std::string& UID) const;

    // Returns the total count of generic systems in a configuration
    TIGL_EXPORT size_t GetGenericSystemCount();

    // Returns the generic system for a given index.
    TIGL_EXPORT const CCPACSGenericSystem& GetGenericSystem(size_t index) const;
    TIGL_EXPORT CCPACSGenericSystem& GetGenericSystem(size_t index);

    // Returns the generic system for a given UID.
    TIGL_EXPORT const CCPACSGenericSystem& GetGenericSystem(const std::string& UID) const;
    TIGL_EXPORT CCPACSGenericSystem& GetGenericSystem(const std::string& UID);

    // Returns the total count of rotors in a configuration
    TIGL_EXPORT size_t GetRotorCount() const;

    // Returns the rotor for a given index.
    TIGL_EXPORT const CCPACSRotor& GetRotor(size_t index) const;
    TIGL_EXPORT CCPACSRotor& GetRotor(size_t index);

    // Returns the rotor for a given UID.
    TIGL_EXPORT const CCPACSRotor& GetRotor(const std::string& UID) const;
    TIGL_EXPORT CCPACSRotor& GetRotor(const std::string& UID);

    // Returns the rotor index for a given UID.
    TIGL_EXPORT size_t GetRotorIndex(const std::string& UID) const;

    TIGL_EXPORT TopoDS_Shape GetParentLoft(const std::string& UID);

    TIGL_EXPORT bool HasFuselageProfile(std::string uid) const;

    // Returns the total count of fuselage profiles in this configuration
    TIGL_EXPORT size_t GetFuselageProfileCount() const;

    // Returns the fuselage profile for a given index.
    TIGL_EXPORT const CCPACSFuselageProfile& GetFuselageProfile(size_t index) const;
    TIGL_EXPORT CCPACSFuselageProfile& GetFuselageProfile(size_t index);

    // Returns the fuselage profile for a given uid.
    TIGL_EXPORT const CCPACSFuselageProfile& GetFuselageProfile(std::string uid) const;
    TIGL_EXPORT CCPACSFuselageProfile& GetFuselageProfile(std::string uid);

    // Returns the total count of fuselages in a configuration
    TIGL_EXPORT size_t GetFuselageCount() const;

    // Returns the fuselage for a given index.
    TIGL_EXPORT const CCPACSFuselage& GetFuselage(size_t index) const;
    TIGL_EXPORT CCPACSFuselage& GetFuselage(size_t index);

    // Returns the fuselage for a given UID.
    TIGL_EXPORT const CCPACSFuselage& GetFuselage(const std::string& UID) const;
    TIGL_EXPORT CCPACSFuselage& GetFuselage(const std::string& UID);

    // Returns the fuselage index for a given UID.
    TIGL_EXPORT size_t GetFuselageIndex(const std::string& UID) const;

    // Returns list of fuselages
    TIGL_EXPORT const CCPACSFuselages& GetFuselages() const;
    TIGL_EXPORT CCPACSFuselages& GetFuselages();

    // Returns ducts
    TIGL_EXPORT bool HasDucts() const;
    TIGL_EXPORT const boost::optional<CCPACSDucts>& GetDucts() const;
    TIGL_EXPORT boost::optional<CCPACSDucts>& GetDucts();

    // Returns the total count of fuel tanks in a configuration
    TIGL_EXPORT size_t GetFuelTankCount() const;

    // Returns the fuel tank for a given index.
    TIGL_EXPORT const CCPACSFuelTank& GetFuelTank(size_t index) const;
    TIGL_EXPORT CCPACSFuelTank& GetFuelTank(size_t index);

    // Returns the fuel tank for a given UID.
    TIGL_EXPORT const CCPACSFuelTank& GetFuelTank(const std::string& UID) const;
    TIGL_EXPORT CCPACSFuelTank& GetFuelTank(const std::string& UID);

    // Returns the fuel tank index for a given UID.
    TIGL_EXPORT size_t GetFuelTankIndex(const std::string& UID) const;

    // Returns list of fuel tanks
    TIGL_EXPORT const CCPACSFuelTanks& GetFuelTanks() const;
    TIGL_EXPORT CCPACSFuelTanks& GetFuelTanks();

    // Returns list of engine pylons
    TIGL_EXPORT const boost::optional<CCPACSEnginePylons>& GetEnginePylons() const;
    TIGL_EXPORT boost::optional<CCPACSEnginePylons>& GetEnginePylons();

    // Returns list of engines
    TIGL_EXPORT const boost::optional<CCPACSEngines>& GetEngines() const;
    TIGL_EXPORT boost::optional<CCPACSEngines>& GetEngines();

    // Returns list of engine positions
    TIGL_EXPORT const boost::optional<CCPACSEnginePositions>& GetEnginePositions() const;
    TIGL_EXPORT boost::optional<CCPACSEnginePositions>& GetEnginePositions();

    TIGL_EXPORT const CCPACSEngine& GetEngine(const std::string& UID) const;
    TIGL_EXPORT CCPACSEngine& GetEngine(const std::string& UID);

    // Returns the farfield
    TIGL_EXPORT CCPACSFarField& GetFarField();

    // Returns the number of external objects (i.e. linked CAD files like STEP, IGES)
    TIGL_EXPORT size_t GetExternalObjectCount() const;

    TIGL_EXPORT CCPACSExternalObject& GetExternalObject(size_t index) const;

    // Returns the guide curve profile for a given UID.
    TIGL_EXPORT const CCPACSGuideCurveProfile& GetGuideCurveProfile(std::string UID) const;
    TIGL_EXPORT CCPACSGuideCurveProfile& GetGuideCurveProfile(std::string UID);

    // Returns the uid manager
    TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;
    TIGL_EXPORT CTiglUIDManager& GetUIDManager();

    // Returns the algorithm for fusing the aircraft
    TIGL_EXPORT PTiglFusePlane AircraftFusingAlgo();

    // Returns the length of the airplane
    TIGL_EXPORT double GetAirplaneLength();

    // Returns the UID of the loaded configuration.
    TIGL_EXPORT const std::string& GetUID() const;

    TIGL_EXPORT CTiglShapeCache& GetShapeCache();

    TIGL_EXPORT CTiglMemoryPool& GetMemoryPool();

    /** Getter/Setter for member name */
    TIGL_EXPORT std::string GetName() const;

    /** Getter/Setter for member description */
    TIGL_EXPORT std::string GetDescription() const;

    /** Getter for member header */
    TIGL_EXPORT CCPACSHeader* GetHeader();

    /** Getter for member wings */
    TIGL_EXPORT const CCPACSWings& GetWings() const;
    TIGL_EXPORT CCPACSWings& GetWings();

    TIGL_EXPORT boost::optional<const CCPACSACSystems&> GetACSystems() const;
    TIGL_EXPORT boost::optional<CCPACSACSystems&> GetACSystems();


private:
    // Copy constructor
    CCPACSConfiguration(const CCPACSConfiguration&);

    // Assignment operator
    void operator=(const CCPACSConfiguration&);

private:
    CTiglUIDManager                        uidManager;           /**< Stores the unique ids of the components. */ // list as first member, has to be created first and destroyed last
    boost::optional<CCPACSAircraftModel>   aircraftModel;
    boost::optional<CCPACSRotorcraftModel> rotorcraftModel;
    boost::optional<CCPACSProfiles>        profiles;             /**< Wing airfoils, fuselage profiles, rotor airfoils, guide curve profiles */
    boost::optional<CCPACSEngines>         engines;              /**< Engines under vehicle node */
    TixiDocumentHandle                     tixiDocumentHandle;   /**< Handle for internal TixiDocument */
    CCPACSHeader                           header;               /**< Configuration header element */
    CCPACSFarField                         farField;             /**< Far field configuration for CFD tools */
    PTiglFusePlane                         aircraftFuser;        /**< The aircraft fusing algo */
    CTiglShapeCache                        shapeCache;
    CTiglMemoryPool                        memoryPool;
};

/// Computes the bounding box coordinates of the specified component
TIGL_EXPORT void ComponentGetBoundingBox(const CCPACSConfiguration& config, const std::string& uid, CTiglPoint& min, CTiglPoint& max);

/// Computes the bounding box of the whole aircraft
TIGL_EXPORT void ConfigurationGetBoundingBox(const CCPACSConfiguration& config, CTiglPoint& min, CTiglPoint& max);

} // end namespace tigl

#endif // CCPACSCONFIGURATION_H

