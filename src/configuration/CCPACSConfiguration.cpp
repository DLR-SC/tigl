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

#include "CCPACSConfiguration.h"

#include "CCPACSFuselageSegment.h"
#include "CCPACSWingSegment.h"

#include "TopoDS_Shape.hxx"
#include "Standard_CString.hxx"
#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "BRepAlgo_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "TopoDS_Compound.hxx"
#include "BRepFeat_Gluer.hxx"
#include "BRep_Builder.hxx"
#include "BRepMesh.hxx"
#include "IGESControl_Controller.hxx"
#include "IGESControl_Writer.hxx"
#include "StlAPI_Writer.hxx"
#include "Interface_Static.hxx"
#include "StlAPI.hxx"
#include "BRepTools.hxx"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include "CTiglFusePlane.h"
#include "CNamedShape.h"
#include "generated/TixiHelper.h"

#include <cfloat>

namespace tigl
{

// Constructor
CCPACSConfiguration::CCPACSConfiguration(TixiDocumentHandle tixiHandle)
    : tixiDocumentHandle(tixiHandle) , acSystems(this)
{
}

// Destructor
CCPACSConfiguration::~CCPACSConfiguration()
{
}

// Invalidates the internal state of the configuration and forces
// recalculation of wires, lofts etc.
void CCPACSConfiguration::Invalidate()
{
    if(aircraftModel) {
        aircraftModel->Invalidate();
    }
    if (rotorcraftModel) {
        rotorcraftModel->Invalidate();
    }
    if (profiles) {
        profiles->Invalidate();
    }
    aircraftFuser.reset();
    shapeCache.Clear();
}

namespace {
    const std::string headerXPath   = "/cpacs/header";
    const std::string profilesXPath = "/cpacs/vehicles/profiles";
    const std::string farFieldXPath = "/cpacs/toolspecific/cFD/farField";
}

// Build up memory structure for whole CPACS file
void CCPACSConfiguration::ReadCPACS(const std::string& configurationUID)
{
    char* path;
    if (tixiUIDGetXPath(tixiDocumentHandle, configurationUID.c_str(), &path) != SUCCESS) {
        throw CTiglError("XML error while reading in CCPACSConfiguration::ReadCPACS", TIGL_XML_ERROR);
    }

    if (tixi::TixiCheckElement(tixiDocumentHandle, headerXPath)) {
        header.ReadCPACS(tixiDocumentHandle, headerXPath);
    }
    if (tixi::TixiCheckElement(tixiDocumentHandle, profilesXPath)) {
        profiles = boost::in_place(&uidManager);
        // read wing airfoils, fuselage profiles, rotor airfoils and guide curve profiles
        profiles->ReadCPACS(tixiDocumentHandle, profilesXPath);
    }
    if (tixi::TixiCheckElement(tixiDocumentHandle, farFieldXPath)) {
        farField.ReadCPACS(tixiDocumentHandle, farFieldXPath);
    }

    // create new root component for CTiglUIDManager
    const bool isRotorcraft = tixi::TixiCheckElement(tixiDocumentHandle, "/cpacs/vehicles/rotorcraft/model[@uID='" + std::string(configurationUID) + "']");
    if (isRotorcraft) {
        aircraftModel = boost::none;
        rotorcraftModel = boost::in_place(this);
        // TODO(bgruber): why can't we just write "/cpacs/vehicles/rotorcraft/model[" + configurationUID + "]" ?
        rotorcraftModel->ReadCPACS(tixiDocumentHandle, path); // reads everything underneath /cpacs/vehicles/rotorcraft/model
    }
    else {
        rotorcraftModel = boost::none;
        aircraftModel = boost::in_place(this);
        // TODO(bgruber): why can't we just write "/cpacs/vehicles/aircraft/model[" + configurationUID + "]" ?
        aircraftModel->ReadCPACS(tixiDocumentHandle, path); // reads everything underneath /cpacs/vehicles/aircraft/model
    }

    acSystems.ReadCPACS(tixiDocumentHandle, configurationUID);

    // Now do parent <-> child transformations. Child should use the parent coordinate system as root.
    uidManager.SetParentComponents();
}

// Write CPACS structure to tixiHandle
void CCPACSConfiguration::WriteCPACS(const std::string& configurationUID)
{
    header.WriteCPACS(tixiDocumentHandle, headerXPath);
    if (aircraftModel) {
        tixi::TixiCreateElementsIfNotExists(tixiDocumentHandle, "/cpacs/vehicles/aircraft/model");
        tixi::TixiSaveAttribute(tixiDocumentHandle, "/cpacs/vehicles/aircraft/model", "uID", configurationUID); // patch uid in tixi, so xpath below is valid
        aircraftModel->SetUID(configurationUID);
        aircraftModel->WriteCPACS(tixiDocumentHandle, "/cpacs/vehicles/aircraft/model[@uID=\"" + configurationUID + "\"]");
    }
    if (rotorcraftModel) {
        tixi::TixiCreateElementsIfNotExists(tixiDocumentHandle, "/cpacs/vehicles/rotorcraft/model");
        tixi::TixiSaveAttribute(tixiDocumentHandle, "/cpacs/vehicles/rotorcraft/model", "uID", configurationUID); // patch uid in tixi, so xpath below is valid
        rotorcraftModel->SetUID(configurationUID);
        rotorcraftModel->WriteCPACS(tixiDocumentHandle, "/cpacs/vehicles/rotorcraft/model[@uID=\"" + configurationUID + "\"]");
    }
    if (profiles) {
        tixi::TixiCreateElementsIfNotExists(tixiDocumentHandle, profilesXPath);
        profiles->WriteCPACS(tixiDocumentHandle, profilesXPath);
    }
}


// Returns the boolean fused airplane as TopoDS_Shape
PTiglFusePlane CCPACSConfiguration::AircraftFusingAlgo()
{
    if (! aircraftFuser) {
        aircraftFuser = PTiglFusePlane(new CTiglFusePlane(*this));
    }
    return aircraftFuser;
}


// Returns the underlying tixi document handle used by a CPACS configuration
TixiDocumentHandle CCPACSConfiguration::GetTixiDocumentHandle() const
{
    return tixiDocumentHandle;
}

bool CCPACSConfiguration::HasWingProfile(std::string uid) const
{
    if (profiles) {
        if (profiles->GetWingAirfoils() && profiles->GetWingAirfoils()->HasProfile(uid)) {
            return true;
        }
        if (profiles->GetRotorAirfoils() && profiles->GetRotorAirfoils()->HasProfile(uid)) {
            return true;
        }
    }

    return false;
}

// Returns whether this configuration is a rotorcraft
bool CCPACSConfiguration::IsRotorcraft() const
{
    if (aircraftModel) {
        return false;
    }
    if (rotorcraftModel) {
        return true;
    }
    throw CTiglError("No configuration loaded");
}

// Returns the class which holds all wing profiles
boost::optional<CCPACSWingProfiles&> CCPACSConfiguration::GetWingProfiles()
{
    if (profiles && profiles->GetWingAirfoils())
        return *profiles->GetWingAirfoils();
    else
        return boost::none;
}

boost::optional<const CCPACSWingProfiles&> CCPACSConfiguration::GetWingProfiles() const
{
    if (profiles && profiles->GetWingAirfoils())
        return *profiles->GetWingAirfoils();
    else
        return boost::none;
}

// Returns the class which holds all wing profiles
boost::optional<CCPACSRotorProfiles&> CCPACSConfiguration::GetRotorProfiles()
{
    if (profiles && profiles->GetRotorAirfoils())
        return *profiles->GetRotorAirfoils();
    else
        return boost::none;
}

// Returns the class which holds all fuselage profiles
boost::optional<CCPACSFuselageProfiles&> CCPACSConfiguration::GetFuselageProfiles()
{
    if (profiles && profiles->GetFuselageProfiles())
        return *profiles->GetFuselageProfiles();
    else
        return boost::none;
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSConfiguration::GetWingProfile(std::string uid) const
{
    if (profiles->GetWingAirfoils() && profiles->GetWingAirfoils()->HasProfile(uid)) {
        return profiles->GetWingAirfoils()->GetProfile(uid);
    }
    else if (profiles->GetRotorAirfoils() && profiles->GetRotorAirfoils()->HasProfile(uid)) {
        return profiles->GetRotorAirfoils()->GetProfile(uid);
    }
    else {
        throw CTiglError("Profile " + uid + " does not exists");
    }
}

// Returns the aircraft systems object.
CCPACSACSystems& CCPACSConfiguration::GetACSystems()
{
    return acSystems;
}

// Returns the total count of wings (including rotor blades) in a configuration
int CCPACSConfiguration::GetWingCount() const
{
    if (aircraftModel && aircraftModel->GetWings()) {
        return aircraftModel->GetWings()->GetWingCount();
    }
    else if (rotorcraftModel) {
        return
            (rotorcraftModel->GetWings() ? rotorcraftModel->GetWings()->GetWingCount() : 0) +
            (rotorcraftModel->GetRotorBlades() ? rotorcraftModel->GetRotorBlades()->GetRotorBladeCount() : 0);
    }
    else {
        return 0;
    }
}

// Returns the count of rotor blade wings in a configuration
int CCPACSConfiguration::GetRotorBladeCount() const
{
    if (rotorcraftModel && rotorcraftModel->GetRotorBlades()) {
        return rotorcraftModel->GetRotorBlades()->GetRotorBladeCount();
    }
    else {
        return 0;
    }
}

// Returns the wing (or rotor blade) for a given index.
CCPACSWing& CCPACSConfiguration::GetWing(int index) const
{
    if (aircraftModel) {
        return aircraftModel->GetWings()->GetWing(index);
    }
    else if (rotorcraftModel) {
        const int wingCount = rotorcraftModel->GetWings() ? rotorcraftModel->GetWings()->GetWingCount() : 0;
        if (index <= wingCount) {
            return rotorcraftModel->GetWings()->GetWing(index);
        }
        else {
            return rotorcraftModel->GetRotorBlades()->GetRotorBlade(index - wingCount);
        }

    }
    else {
        throw CTiglError("No configuration loaded");
    }
}
// Returns the wing (or rotor blade) for a given UID.
CCPACSWing& CCPACSConfiguration::GetWing(const std::string& UID) const
{
    if (aircraftModel) {
        return aircraftModel->GetWings()->GetWing(UID);
    }
    else if (rotorcraftModel) {
        if(rotorcraftModel->GetWings() && rotorcraftModel->GetWings()->HasWing(UID)) {
            return rotorcraftModel->GetWings()->GetWing(UID);
        }
        else {
            return rotorcraftModel->GetRotorBlades()->GetRotorBlade(UID);
        }
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

// Returns the wing (or rotor blade) index for a given UID.
int CCPACSConfiguration::GetWingIndex(const std::string& UID) const
{
    if (aircraftModel) {
        return aircraftModel->GetWings()->GetWingIndex(UID);
    }
    else if (rotorcraftModel) {
        if (rotorcraftModel->GetWings() && rotorcraftModel->GetWings()->HasWing(UID)) {
            return rotorcraftModel->GetWings()->GetWingIndex(UID);
        }
        else {
            // offset rotor blade indices by wing count
            return (rotorcraftModel->GetWings() ? rotorcraftModel->GetWings()->GetWingCount() : 0) +
                    rotorcraftModel->GetRotorBlades()->GetRotorBladeIndex(UID);
        }
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

// Returns the total count of generic systems in a configuration
int CCPACSConfiguration::GetGenericSystemCount()
{
    return acSystems.GetGenericSystems().GetGenericSystemCount();
}

// Returns the generic system for a given index.
CCPACSGenericSystem& CCPACSConfiguration::GetGenericSystem(int index)
{
    return acSystems.GetGenericSystems().GetGenericSystem(index);
}
// Returns the generic system for a given UID.
CCPACSGenericSystem& CCPACSConfiguration::GetGenericSystem(const std::string& UID)
{
    return acSystems.GetGenericSystems().GetGenericSystem(UID);
}

// Returns the total count of rotors in a configuration
int CCPACSConfiguration::GetRotorCount() const
{
    if (rotorcraftModel && rotorcraftModel->GetRotors()) {
        return rotorcraftModel->GetRotors()->GetRotorCount();
    }
    else {
        return 0;
    }
}

// Returns the rotor for a given index.
CCPACSRotor& CCPACSConfiguration::GetRotor(int index) const
{
    if (rotorcraftModel) {
        return rotorcraftModel->GetRotors()->GetRotor(index);
    }
    else {
        throw CTiglError("no rotorcraft loaded");
    }
}

// Returns the rotor for a given UID.
CCPACSRotor& CCPACSConfiguration::GetRotor(const std::string& UID) const
{
    if (rotorcraftModel) {
        return rotorcraftModel->GetRotors()->GetRotor(UID);
    }
    else {
        throw CTiglError("no rotorcraft loaded");
    }
}

// Returns the rotor index for a given UID.
int CCPACSConfiguration::GetRotorIndex(const std::string& UID) const
{
    if (rotorcraftModel) {
        return rotorcraftModel->GetRotors()->GetRotorIndex(UID);
    }
    else {
        throw CTiglError("no rotorcraft loaded");
    }
}

TopoDS_Shape CCPACSConfiguration::GetParentLoft(const std::string& UID)
{
    return uidManager.GetParentGeometricComponent(UID)->GetLoft()->Shape();
}

bool CCPACSConfiguration::HasFuselageProfile(std::string uid) const
{
    if (profiles && profiles->GetFuselageProfiles()) {
        return profiles->GetFuselageProfiles()->HasProfile(uid);
    }
    else {
        return false;
    }
}

// Returns the total count of fuselage profiles in this configuration
int CCPACSConfiguration::GetFuselageProfileCount() const
{
    if (profiles && profiles->GetFuselageProfiles()) {
        return profiles->GetFuselageProfiles()->GetProfileCount();
    }
    else {
        return 0;
    }
}

// Returns the fuselage profile for a given index.
CCPACSFuselageProfile& CCPACSConfiguration::GetFuselageProfile(int index) const
{
    return profiles->GetFuselageProfiles()->GetProfile(index);
}

// Returns the fuselage profile for a given uid.
CCPACSFuselageProfile& CCPACSConfiguration::GetFuselageProfile(std::string uid) const
{
    return profiles->GetFuselageProfiles()->GetProfile(uid);
}

// Returns the total count of fuselages in a configuration
int CCPACSConfiguration::GetFuselageCount() const
{
    if (aircraftModel) {
        if (aircraftModel->GetFuselages()) {
            return aircraftModel->GetFuselages()->GetFuselageCount();
        }
        else {
            return 0;
        }
    }
    else if (rotorcraftModel) {
        if (rotorcraftModel->GetFuselages()) {
            return rotorcraftModel->GetFuselages()->GetFuselageCount();
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

// Returns the fuselage for a given index.
CCPACSFuselage& CCPACSConfiguration::GetFuselage(int index) const
{
    if (aircraftModel) {
        return aircraftModel->GetFuselages()->GetFuselage(index);
    }
    else if(rotorcraftModel) {
        return rotorcraftModel->GetFuselages()->GetFuselage(index);
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}


CCPACSFuselages& CCPACSConfiguration::GetFuselages()
{
    if (aircraftModel) {
        return *aircraftModel->GetFuselages();
    }
    else if (rotorcraftModel) {
        return *rotorcraftModel->GetFuselages();
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

const CCPACSFuselages& CCPACSConfiguration::GetFuselages() const
{
    if (aircraftModel) {
        return *aircraftModel->GetFuselages();
    }
    else if (rotorcraftModel) {
        return *rotorcraftModel->GetFuselages();
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

boost::optional<CCPACSEnginePylons>& CCPACSConfiguration::GetEnginePylons()
{
    if (aircraftModel) {
        return aircraftModel->GetEnginePylons();
    }
    else {
        throw CTiglError("No aircraft loaded");
    }
}

const boost::optional<CCPACSEnginePylons>& CCPACSConfiguration::GetEnginePylons() const
{
    if (aircraftModel) {
        return aircraftModel->GetEnginePylons();
    }
    else {
        throw CTiglError("No aircraft loaded");
    }
}

CCPACSFarField& CCPACSConfiguration::GetFarField()
{
    return farField;
}

// Returns the fuselage index for a given UID.
int CCPACSConfiguration::GetFuselageIndex(const std::string& UID) const
{
    return GetFuselages().GetFuselageIndex(UID);
}

int CCPACSConfiguration::GetExternalObjectCount() const
{
    if (aircraftModel && aircraftModel->GetGenericGeometryComponents()) {
        return aircraftModel->GetGenericGeometryComponents()->GetObjectCount();
    }
    else {
        return 0;
    }
}

CCPACSExternalObject&CCPACSConfiguration::GetExternalObject(int index) const
{
    return aircraftModel->GetGenericGeometryComponents()->GetObject(index);
}

// Returns the fuselage for a given UID.
CCPACSFuselage& CCPACSConfiguration::GetFuselage(const std::string& UID) const
{
    if (aircraftModel) {
        return aircraftModel->GetFuselages()->GetFuselage(UID);
    }
    else if (rotorcraftModel) {
        return rotorcraftModel->GetFuselages()->GetFuselage(UID);
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

// Returns the guide curve profile for a given UID.
CCPACSGuideCurveProfile& CCPACSConfiguration::GetGuideCurveProfile(std::string UID)
{
    return profiles->GetGuideCurves()->GetGuideCurveProfile(UID);
}

const CCPACSGuideCurveProfile& CCPACSConfiguration::GetGuideCurveProfile(std::string UID) const
{
    return profiles->GetGuideCurves()->GetGuideCurveProfile(UID);
}

// Returns the uid manager
CTiglUIDManager& CCPACSConfiguration::GetUIDManager()
{
    return uidManager;
}

const CTiglUIDManager& CCPACSConfiguration::GetUIDManager() const
{
    return uidManager;
}

double CCPACSConfiguration::GetAirplaneLenth()
{
    Bnd_Box boundingBox;

    // Draw all wings
    for (int w = 1; w <= GetWingCount(); w++) {
        tigl::CCPACSWing& wing = GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(i);
            BRepBndLib::Add(segment.GetLoft()->Shape(), boundingBox);

        }

        if (wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
            continue;
        }

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(i);
            BRepBndLib::Add(segment.GetLoft()->Shape(), boundingBox);
        }
    }

    for (int f = 1; f <= GetFuselageCount(); f++) {
        tigl::CCPACSFuselage& fuselage = GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(i);
            BRepBndLib::Add(segment.GetLoft()->Shape(), boundingBox);
        }
    }
    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    return xmax-xmin;
}

// Returns the uid manager
const std::string& CCPACSConfiguration::GetUID() const
{
    if (aircraftModel) {
        return aircraftModel->GetUID();
    }
    else if (rotorcraftModel) {
        return rotorcraftModel->GetUID();
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

CTiglShapeCache& CCPACSConfiguration::GetShapeCache()
{
    return shapeCache;
}

CTiglMemoryPool& CCPACSConfiguration::GetMemoryPool()
{
    return memoryPool;
}

std::string CCPACSConfiguration::GetName() const
{
    if(aircraftModel) {
        return aircraftModel->GetName();
    }
    else if (rotorcraftModel) {
        return rotorcraftModel->GetName();
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

std::string CCPACSConfiguration::GetDescription() const
{
    if (aircraftModel) {
        return aircraftModel->GetDescription().get_value_or("");
    }
    else if (rotorcraftModel) {
        return rotorcraftModel->GetDescription().get_value_or("");
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

CCPACSHeader* CCPACSConfiguration::GetHeader()
{
    return &header;
}

CCPACSWings& CCPACSConfiguration::GetWings()
{
    if (aircraftModel) {
        return *aircraftModel->GetWings();
    }
    else if (rotorcraftModel) {
        return *rotorcraftModel->GetWings();
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}

const CCPACSWings& CCPACSConfiguration::GetWings() const
{
    if (aircraftModel) {
        return *aircraftModel->GetWings();
    }
    else if (rotorcraftModel) {
        return *rotorcraftModel->GetWings();
    }
    else {
        throw CTiglError("No configuration loaded");
    }
}
} // end namespace tigl

