/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
*
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
#include "CCPACSHeader.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSRotors.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"
#include "CCPACSFarField.h"
#include "CCPACSGuideCurveProfiles.h"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"
#include "CTiglShapeCache.h"
#include "CSharedPtr.h"

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
    TIGL_EXPORT virtual ~CCPACSConfiguration(void);

    // Invalidates the internal state of the configuration and forces
    // recalculation of wires, lofts etc.
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS configuration
    TIGL_EXPORT void ReadCPACS(const char* configurationUID);

    // Returns the underlying tixi document handle used by a CPACS configuration
    TIGL_EXPORT TixiDocumentHandle GetTixiDocumentHandle(void) const;

    // Returns whether this configuration is a rotorcraft
    TIGL_EXPORT bool IsRotorcraft(void) const;

    // Returns the total count of wing profiles in this configuration
    TIGL_EXPORT int GetWingProfileCount(void) const;

    // Returns the wing profile for a given index - TODO: depricated!
    TIGL_EXPORT CCPACSWingProfile& GetWingProfile(int index) const;

    // Returns the wing profile for a given uid.
    TIGL_EXPORT CCPACSWingProfile& GetWingProfile(std::string uid) const;

    // Returns the total count of wings in a configuration
    TIGL_EXPORT int GetWingCount(void) const;

    // Returns the count of wings in a configuration with the property isRotorBlade set to true
    TIGL_EXPORT int GetRotorBladeCount(void) const;

    // Returns the wing for a given index.
    TIGL_EXPORT CCPACSWing& GetWing(int index) const;

    // Returns the wing for a given UID.
    TIGL_EXPORT CCPACSWing& GetWing(const std::string& UID) const;

    // Returns the total count of rotors in a configuration
    TIGL_EXPORT int GetRotorCount(void) const;

    // Returns the rotor for a given index.
    TIGL_EXPORT CCPACSRotor& GetRotor(int index) const;

    // Returns the rotor for a given UID.
    TIGL_EXPORT CCPACSRotor& GetRotor(const std::string& UID) const;

    TIGL_EXPORT TopoDS_Shape GetParentLoft(const std::string& UID);

    // Returns the total count of fuselage profiles in this configuration
    TIGL_EXPORT int GetFuselageProfileCount(void) const;

    // Returns the fuselage profile for a given index.
    TIGL_EXPORT CCPACSFuselageProfile& GetFuselageProfile(int index) const;

    // Returns the fuselage profile for a given uid.
    TIGL_EXPORT CCPACSFuselageProfile& GetFuselageProfile(std::string uid) const;

    // Returns the total count of fuselages in a configuration
    TIGL_EXPORT int GetFuselageCount(void) const;

    // Returns the fuselage for a given index.
    TIGL_EXPORT CCPACSFuselage& GetFuselage(int index) const;

    // Returns the fuselage for a given UID.
    TIGL_EXPORT CCPACSFuselage& GetFuselage(std::string UID) const;

    // Returns the farfield
    TIGL_EXPORT CCPACSFarField& GetFarField();

    // Returns the guide curve profile for a given UID.
    TIGL_EXPORT CCPACSGuideCurveProfile& GetGuideCurveProfile(std::string UID) const;

    // Returns the uid manager
    TIGL_EXPORT CTiglUIDManager& GetUIDManager(void);

    // Returns the algorithm for fusing the aircraft
    TIGL_EXPORT PTiglFusePlane AircraftFusingAlgo(void);

    // Returns the length of the airplane
    TIGL_EXPORT double GetAirplaneLenth(void);

    // Returns the UID of the loaded configuration.
    TIGL_EXPORT const std::string& GetUID(void) const;

    TIGL_EXPORT CTiglShapeCache& GetShapeCache(void);

protected:
    // transform all components relative to their parents
    void transformAllComponents(CTiglAbstractPhysicalComponent* parent);

private:
    // Copy constructor
    CCPACSConfiguration(const CCPACSConfiguration&);

    // Assignment operator
    void operator=(const CCPACSConfiguration&);

private:
    TixiDocumentHandle           tixiDocumentHandle;   /**< Handle for internal TixiDocument */
    bool                         isRotorcraft;         /**< Indicates whether this configuration is a rotorcraft */
    CCPACSHeader                 header;               /**< Configuration header element */
    CCPACSWings                  wings;                /**< Configuration wings element */
    CCPACSFuselages              fuselages;            /**< Configuration fuselages element */
    CCPACSRotors                 rotors;               /**< Configuration rotors element */
    CCPACSFarField               farField;             /**< Far field configuration for CFD tools */
    CCPACSGuideCurveProfiles     guideCurveProfiles;   /**< Guide curve profiles */
    CTiglUIDManager              uidManager;           /**< Stores the unique ids of the components */
    PTiglFusePlane               aircraftFuser;        /**< The aircraft fusing algo */
    std::string                  configUID;            /**< UID of the opened configuration   */
    CTiglShapeCache              shapeCache;
};

} // end namespace tigl

#endif // CCPACSCONFIGURATION_H

