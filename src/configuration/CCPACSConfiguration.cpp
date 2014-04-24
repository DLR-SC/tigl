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

#include <cfloat>

namespace tigl
{

// Constructor
CCPACSConfiguration::CCPACSConfiguration(TixiDocumentHandle tixiHandle)
    : tixiDocumentHandle(tixiHandle)
    , header()
    , isRotorcraft(false)
    , wings(this)
    , fuselages(this)
    , rotors(this)
    , uidManager()
{
}

// Destructor
CCPACSConfiguration::~CCPACSConfiguration(void)
{
}

// Invalidates the internal state of the configuration and forces
// recalculation of wires, lofts etc.
void CCPACSConfiguration::Invalidate(void)
{
    isRotorcraft = false;
    wings.Invalidate();
    fuselages.Invalidate();
    rotors.Invalidate();
    aircraftFuser.reset();
    shapeCache.Clear();
    configUID = "";
}

// Build up memory structure for whole CPACS file
void CCPACSConfiguration::ReadCPACS(const char* configurationUID)
{
    if (!configurationUID) {
        return;
    }

    // Check if the configuration is a rotorcraft
    std::string rotorcraftModelXPath = "/cpacs/vehicles/rotorcraft/model[@uID='" + std::string(configurationUID) + "']";
    if (tixiCheckElement(tixiDocumentHandle, rotorcraftModelXPath.c_str()) == SUCCESS) {
        isRotorcraft = true;
    }

    header.ReadCPACS(tixiDocumentHandle);
    wings.ReadCPACS(tixiDocumentHandle, configurationUID);
    if (isRotorcraft) {
        wings.ReadCPACS(tixiDocumentHandle, configurationUID, true, true, "rotorBlades", "rotorBlade", "/cpacs/vehicles/profiles/rotorAirfoils", "rotorAirfoil");
        rotors.ReadCPACS(tixiDocumentHandle, configurationUID);
    }
    fuselages.ReadCPACS(tixiDocumentHandle, configurationUID);
    farField.ReadCPACS(tixiDocumentHandle);
    guideCurveProfiles.ReadCPACS(tixiDocumentHandle);

    configUID = configurationUID;

    // Now do parent <-> child transformations.
    // Child should use the parent coordinate system as root.
    try {
        uidManager.Update();
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
    }
    try {
        const UIDStoreContainerType& allRootComponentsWithChildren = uidManager.GetAllRootComponentsWithChildren();
        for (UIDStoreContainerType::const_iterator pIter = allRootComponentsWithChildren.begin(); pIter != allRootComponentsWithChildren.end(); ++pIter) {
            CTiglAbstractPhysicalComponent* rootComponent = pIter->second;
            transformAllComponents(rootComponent);
        }
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
    }
}

// transform all components relative to their parents
void CCPACSConfiguration::transformAllComponents(CTiglAbstractPhysicalComponent* parent)
{
    CTiglAbstractPhysicalComponent::ChildContainerType children = parent->GetChildren(false);
    CTiglAbstractPhysicalComponent::ChildContainerType::iterator pIter;
    CTiglPoint parentTranslation = parent->GetTranslation();
    for (pIter = children.begin(); pIter != children.end(); ++pIter) {
        CTiglAbstractPhysicalComponent* child = *pIter;
        child->Translate(parentTranslation);
        transformAllComponents(child);
    }
}


// Returns the boolean fused airplane as TopoDS_Shape
PTiglFusePlane CCPACSConfiguration::AircraftFusingAlgo(void)
{
    if (! aircraftFuser) {
        aircraftFuser = PTiglFusePlane(new CTiglFusePlane(*this));
    }
    return aircraftFuser;
}


// Returns the underlying tixi document handle used by a CPACS configuration
TixiDocumentHandle CCPACSConfiguration::GetTixiDocumentHandle(void) const
{
    return tixiDocumentHandle;
}

// Returns whether this configuration is a rotorcraft
bool CCPACSConfiguration::IsRotorcraft(void) const
{
    return isRotorcraft;
}

// Returns the total count of wing profiles in this configuration
int CCPACSConfiguration::GetWingProfileCount(void) const
{
    return wings.GetProfileCount();
}

// Returns the wing profile for a given uid.
CCPACSWingProfile& CCPACSConfiguration::GetWingProfile(std::string uid) const
{
    return wings.GetProfile(uid);
}

// Returns the wing profile for a given index - TODO: depricated function!
CCPACSWingProfile& CCPACSConfiguration::GetWingProfile(int index) const
{
    return wings.GetProfile(index);
}

// Returns the total count of wings in a configuration
int CCPACSConfiguration::GetWingCount(void) const
{
    return wings.GetWingCount();
}

// Returns the count of wings in a configuration with the property isRotorBlade set to true
int CCPACSConfiguration::GetRotorBladeCount(void) const
{
    return wings.GetRotorBladeCount();
}

// Returns the wing for a given index.
CCPACSWing& CCPACSConfiguration::GetWing(int index) const
{
    return wings.GetWing(index);
}

// Returns the wing for a given UID.
CCPACSWing& CCPACSConfiguration::GetWing(const std::string& UID) const
{
    return wings.GetWing(UID);
}

// Returns the wing index for a given UID.
int CCPACSConfiguration::GetWingIndex(const std::string& UID) const
{
    return wings.GetWingIndex(UID);
}

// Returns the total count of rotors in a configuration
int CCPACSConfiguration::GetRotorCount(void) const
{
    return rotors.GetRotorCount();
}

// Returns the rotor for a given index.
CCPACSRotor& CCPACSConfiguration::GetRotor(int index) const
{
    return rotors.GetRotor(index);
}

// Returns the rotor for a given UID.
CCPACSRotor& CCPACSConfiguration::GetRotor(const std::string& UID) const
{
    return rotors.GetRotor(UID);
}

// Returns the rotor index for a given UID.
int CCPACSConfiguration::GetRotorIndex(const std::string& UID) const
{
    return rotors.GetRotorIndex(UID);
}

TopoDS_Shape CCPACSConfiguration::GetParentLoft(const std::string& UID)
{
    return uidManager.GetParentComponent(UID)->GetLoft();
}

// Returns the total count of fuselage profiles in this configuration
int CCPACSConfiguration::GetFuselageProfileCount(void) const
{
    return fuselages.GetProfileCount();
}

// Returns the fuselage profile for a given index.
CCPACSFuselageProfile& CCPACSConfiguration::GetFuselageProfile(int index) const
{
    return fuselages.GetProfile(index);
}

// Returns the fuselage profile for a given uid.
CCPACSFuselageProfile& CCPACSConfiguration::GetFuselageProfile(std::string uid) const
{
    return fuselages.GetProfile(uid);
}

// Returns the total count of fuselages in a configuration
int CCPACSConfiguration::GetFuselageCount(void) const
{
    return fuselages.GetFuselageCount();
}

// Returns the fuselage for a given index.
CCPACSFuselage& CCPACSConfiguration::GetFuselage(int index) const
{
    return fuselages.GetFuselage(index);
}

// Returns the fuselage for a given UID.
CCPACSFuselage& CCPACSConfiguration::GetFuselage(const std::string& UID) const
{
    return fuselages.GetFuselage(UID);
}

// Returns the fuselage index for a given UID.
int CCPACSConfiguration::GetFuselageIndex(const std::string& UID) const
{
    return fuselages.GetFuselageIndex(UID);
}

CCPACSFarField& CCPACSConfiguration::GetFarField()
{
    return farField;
}

// Returns the guide curve profile for a given UID.
CCPACSGuideCurveProfile& CCPACSConfiguration::GetGuideCurveProfile(std::string UID) const
{
    return guideCurveProfiles.GetGuideCurveProfile(UID);
}

// Returns the uid manager
CTiglUIDManager& CCPACSConfiguration::GetUIDManager(void)
{
    return uidManager;
}

double CCPACSConfiguration::GetAirplaneLenth(void)
{
    Bnd_Box boundingBox;

    // Draw all wings
    for (int w = 1; w <= GetWingCount(); w++) {
        tigl::CCPACSWing& wing = GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(i);
            BRepBndLib::Add(segment.GetLoft(), boundingBox);

        }

        if (wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
            continue;
        }

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment&) wing.GetSegment(i);
            BRepBndLib::Add(segment.GetLoft(), boundingBox);
        }
    }

    for (int f = 1; f <= GetFuselageCount(); f++) {
        tigl::CCPACSFuselage& fuselage = GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment&) fuselage.GetSegment(i);
            BRepBndLib::Add(segment.GetLoft(), boundingBox);
        }
    }
    Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
    boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    return xmax-xmin;
}

// Returns the uid manager
const std::string& CCPACSConfiguration::GetUID(void) const
{
    return configUID;
}

CTiglShapeCache& CCPACSConfiguration::GetShapeCache()
{
    return shapeCache;
}

} // end namespace tigl

