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
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"
#include "CCPACSFarField.h"
#include "CCPACSGuideCurveProfiles.h"
#include "TopoDS_Compound.hxx"
#include "BRep_Builder.hxx"
#include "CTiglShapeCache.h"
#include "CTiglMemoryPool.h"
#include "CSharedPtr.h"
// [[CAS_AES]] added includes for structural profiles and elements
#include "CCPACSStructuralProfiles.h"
#include "CCPACSStructuralElements.h"
// [[CAS_AES]] added includes for free form surface support
// [[CAS_AES]] BEGIN
#include "CCPACSFreeFormSurfaces.h"
#include "CCPACSFreeFormSurface.h"
#include "CCPACSMaterials.h"
#include "CCPACSMaterialType.h"
#include "CCPACSComposite.h"
// [[CAS_AES]] END
// [[CAS_AES]] added include for Model as Root Component in CTiglUIDManager
#include "CCPACSModel.h"

#include "CTiglStructuralMountParent.h"


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

    // Write CPACS configuration
    TIGL_EXPORT void WriteCPACS(const std::string& configurationUID);

    // Returns the underlying tixi document handle used by a CPACS configuration
    TIGL_EXPORT TixiDocumentHandle GetTixiDocumentHandle(void) const;

    // Returns the total count of wing profiles in this configuration
    TIGL_EXPORT int GetWingProfileCount(void) const;

    TIGL_EXPORT bool HasWingProfile(std::string uid) const;

    // Returns the class which holds all wing profiles
    TIGL_EXPORT CCPACSWingProfiles& GetWingProfiles(void);
    
    // Returns the class which holds all wing profiles
    TIGL_EXPORT CCPACSFuselageProfiles& GetFuselageProfiles(void);

    // Returns the wing profile for a given index - TODO: depricated!
    TIGL_EXPORT CCPACSWingProfile& GetWingProfile(int index) const;

    // Returns the wing profile for a given uid.
    TIGL_EXPORT CCPACSWingProfile& GetWingProfile(std::string uid) const;

    // Returns the total count of wings in a configuration
    TIGL_EXPORT int GetWingCount(void) const;

    // Returns the wing for a given index.
    TIGL_EXPORT CCPACSWing& GetWing(int index) const;

    // Returns the wing for a given UID.
    TIGL_EXPORT CCPACSWing& GetWing(const std::string& UID) const;

    TIGL_EXPORT TopoDS_Shape GetParentLoft(const std::string& UID);

    TIGL_EXPORT bool HasFuselageProfile(std::string uid) const;

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

    // Returns list of fuselages
    TIGL_EXPORT CCPACSFuselages& GetFuselages();

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

    TIGL_EXPORT CTiglMemoryPool& GetMemoryPool(void);

    // [[CAS_AES]] Returns the total count of StructuralElements in this configuration
    TIGL_EXPORT int GetProfileElementCount(void) const;

    // [[CAS_AES]] Returns the StructuralElement for a given index
    TIGL_EXPORT CCPACSProfileBasedStructuralElement& GetProfileElement(int index) const;

    // [[CAS_AES]] Returns the StructuralElement for a given uid.
    TIGL_EXPORT CCPACSProfileBasedStructuralElement& GetProfileElement(std::string uid) const;

    // [[CAS_AES]] Returns the StructuralElement for a given index
    TIGL_EXPORT CCPACSSheetBasedStructuralElement& GetSheetElement(int index) const;

    // [[CAS_AES]] Returns the StructuralElement for a given uid.
    TIGL_EXPORT CCPACSSheetBasedStructuralElement& GetSheetElement(std::string uid) const;

    // [[CAS_AES]] Returns the total count of StructuralProfiles in this configuration
    TIGL_EXPORT int GetStructuralProfileCount(void) const;

    // [[CAS_AES]] Returns the StructuralElement for a given index
    TIGL_EXPORT CCPACSStructuralProfile2D& GetStructuralProfile(int index) const;

    // [[CAS_AES]] Returns the StructuralElement for a given uid.
    TIGL_EXPORT CCPACSStructuralProfile2D& GetStructuralProfile(std::string uid) const;

    // [[CAS_AES]] added getter for free form surfaces
    TIGL_EXPORT CCPACSFreeFormSurfaces& GetFFFS();

    // [[CAS_AES]] added getter for number of free form surfaces
    TIGL_EXPORT int GetFFFSCount();

    // [[CAS_AES]] added getter for free form surface by index
    TIGL_EXPORT CCPACSFreeFormSurface& GetFFFSbyIndex(int);

    // [[CAS_AES]]
    /** @brief getter for one specific Material by index
    /** @return CCPACSMaterialType*/
    TIGL_EXPORT CCPACSMaterialType* GetMaterial(int index) const;

    // [[CAS_AES]]
    /** @brief getter for one specific Material by UID
    /** @return CCPACSMaterialType*/
    TIGL_EXPORT CCPACSMaterialType* GetMaterial(std::string nUId) const;

    // [[CAS_AES]]
    /** @brief getter for the composite by UId
    /** @return CCPACSComposite*/
    TIGL_EXPORT CCPACSComposite* GetComposite(std::string nUId) const;

    // [[CAS_AES]]
    /** @brief getter for the composite by index
    /** @return CCPACSComposite*/
    TIGL_EXPORT CCPACSComposite* GetComposite(int index) const;

        /** @brief getter for the pressure bulkhead element by UId
    /** @return CCPACSPressureBulkheadElement*/
    TIGL_EXPORT CCPACSPressureBulkheadElement& getPressureBulheadElement(std::string nUId) const;

    /** @brief getter for the pressure bulkhead element by index
    /** @return CCPACSPressureBulkheadElement*/
    TIGL_EXPORT CCPACSPressureBulkheadElement& getPressureBulheadElement(int index) const;

    /** @brief getter for the door by UId
    /** @return CCPACSDoor*/
    TIGL_EXPORT CCPACSDoor& getDoorElement(std::string nUId) const;

    /** @brief getter for the door by index
    /** @return CCPACSDoor*/
    TIGL_EXPORT CCPACSDoor& getDoorElement(int index) const;
    
    /** @brief getter for the wingFuelTank by uid
    /** @return CCPACSWingFuelTank*/
    TIGL_EXPORT CCPACSWingFuelTank& getWingFuelTank(std::string nUId, bool& sym) const;
    
    /** @brief overloaded getter for the wingFuelTank by uid
    /** @return CCPACSWingFuelTank*/
    TIGL_EXPORT CCPACSWingFuelTank& getWingFuelTank(std::string nUId, std::string& wingUId) const;
    
    TIGL_EXPORT CTiglStructuralMountParent& getStructuralMount(std::string nUId, std::string& parentUID, TiglSymmetryAxis& symmetryAxis) const;

    /** Getter/Setter for member name */
    TIGL_EXPORT std::string GetName(void) const;   // EU

    /** Getter/Setter for member description */
    TIGL_EXPORT std::string GetDescription(void) const;   // EU

    /** Getter for member header */
    TIGL_EXPORT CCPACSHeader* GetHeader();    // EU

    /** Getter for member wings */
    TIGL_EXPORT CCPACSWings* GetWings();  // EU

    /** Getter for member materials */
    TIGL_EXPORT CCPACSMaterials* GetMaterials();  // EU

    /** Getter for member structuralProfiles */
    TIGL_EXPORT CCPACSStructuralProfiles* GetStructuralProfiles();  // EU

    /** Getter for member structuralElements */
    TIGL_EXPORT CCPACSStructuralElements* GetStructuralElements();  // EU

protected:
    // transform all components relative to their parents
    void transformAllComponents(CTiglAbstractPhysicalComponent* parent);

private:
    // Copy constructor
    CCPACSConfiguration(const CCPACSConfiguration&);

    // Assignment operator
    void operator=(const CCPACSConfiguration&);

private:
    std::string                  name;                 /**< Configuration name */
    std::string                  description;          /**< Configuration description */
    // [[CAS_AES]] added CCPACSModel
    CCPACSModel*                 cpacsModel;
    TixiDocumentHandle           tixiDocumentHandle;   /**< Handle for internal TixiDocument */
    CCPACSHeader                 header;               /**< Configuration header element */
    CCPACSWings                  wings;                /**< Configuration wings element */
    CCPACSFuselages              fuselages;            /**< Configuration fuselages element */
    CCPACSFarField               farField;             /**< Far field configuration for CFD tools */
    CCPACSGuideCurveProfiles     guideCurveProfiles;   /**< Guide curve profiles */
    CTiglUIDManager              uidManager;           /**< Stores the unique ids of the components */
    PTiglFusePlane               aircraftFuser;        /**< The aircraft fusing algo */
    std::string                  configUID;            /**< UID of the opened configuration   */
    CTiglShapeCache              shapeCache;
    CTiglMemoryPool              memoryPool;
    // [[CAS_AES]] added support for freeform surfaces
    CCPACSFreeFormSurfaces       freeFormSurfaces;
    // [[CAS_AES]] added support for materials
    CCPACSMaterials              materials;
    // [[CAS_AES]] added structural profiles and elements
    // TODO : this should be in another classe
    CCPACSStructuralProfiles     structuralProfiles;   /**< the profiles of the inner structure of the plane > */
    CCPACSStructuralElements     structuralElements;   /**< structural elements parameters */
};

} // end namespace tigl

#endif // CCPACSCONFIGURATION_H

