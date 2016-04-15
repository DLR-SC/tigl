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
#include "TixiSaveExt.h"

#include <cfloat>

namespace tigl
{

// Constructor
// [[CAS_AES]] added initialization of structuralProfiles and structuralElements
CCPACSConfiguration::CCPACSConfiguration(TixiDocumentHandle tixiHandle)
    : tixiDocumentHandle(tixiHandle)
    , header()
    , wings(this)
    , fuselages(this)
    , structuralProfiles(this)
    , structuralElements(this)
    , materials(this)
    , uidManager()
    // [[CAS_AES]] added CPACSModel
    , cpacsModel(NULL)
{
}

// Destructor
CCPACSConfiguration::~CCPACSConfiguration(void)
{
    // [[CAS_AES]] added CPACSModel
    if (cpacsModel)
    {
        delete cpacsModel;
    }
    tixiCloseDocument(tixiDocumentHandle);
}

// Invalidates the internal state of the configuration and forces
// recalculation of wires, lofts etc.
void CCPACSConfiguration::Invalidate(void)
{
    wings.Invalidate();
    fuselages.Invalidate();
    aircraftFuser.reset();
    shapeCache.Clear();
    configUID = "";
    // [[CAS_AES]] added invalidation of freeform surfaces and materials
    freeFormSurfaces.Invalidate();
    materials.Invalidate();

}

// Build up memory structure for whole CPACS file
void CCPACSConfiguration::ReadCPACS(const char* configurationUID)
{
    if (!configurationUID) {
        return;
    }

    // reading name and description
    std::string tempString;
    std::string xpath;
    char* path;

    // memory for path freed by tixi when document is closed
    tempString = configurationUID;
    if (tixiUIDGetXPath(tixiDocumentHandle, tempString.c_str(), &path) != SUCCESS) {
        throw CTiglError("Error: XML error while reading in CCPACSConfiguration::ReadCPACS", TIGL_XML_ERROR);
    }
    xpath = path;

    char* ptrName = NULL;
    tempString    = xpath + "/name";
    if (tixiGetTextElement(tixiDocumentHandle, tempString.c_str(), &ptrName) == SUCCESS) {
        name = ptrName;
    }

    char* ptrDescription = "";
    tempString    = xpath + "/description";
    if (tixiGetTextElement(tixiDocumentHandle, tempString.c_str(), &ptrDescription) == SUCCESS) {
        description = ptrDescription;
    }

    // [[CAS_AES]] added CCPACSModel as root component for CTiglUIDManager
    // [[CAS_AES]] BEGIN
    if (cpacsModel) {
        delete cpacsModel;
    }
    cpacsModel = new CCPACSModel();
    cpacsModel->SetUID(configurationUID);
    uidManager.SetRootComponent(cpacsModel);
    // [[CAS_AES]] END

    header.ReadCPACS(tixiDocumentHandle);
    guideCurveProfiles.ReadCPACS(tixiDocumentHandle);
    wings.ReadCPACS(tixiDocumentHandle, configurationUID);
    fuselages.ReadCPACS(tixiDocumentHandle, configurationUID);
    farField.ReadCPACS(tixiDocumentHandle);

    // [[CAS_AES]] reading structural profiles
    std::string structurePath = "/cpacs/vehicles/profiles";
    structuralProfiles.ReadCPACS(tixiDocumentHandle, structurePath);
    
    // [[CAS_AES]] reading structural elements
    std::string structureElementsPath = "/cpacs/vehicles";
    structuralElements.ReadCPACS(tixiDocumentHandle, structureElementsPath);

    // [[CAS_AES]] reading materials
    materials.ReadCPACS(tixiDocumentHandle, structureElementsPath);
    
    // [[CAS_AES]] reading free form surfaces
    std::string sFFFSPath("/cpacs/ToolSpecific/CAS");
    freeFormSurfaces.ReadCPACS(tixiDocumentHandle, sFFFSPath, this);


    configUID = configurationUID;
    // Now do parent <-> child transformations. Child should use the
    // parent coordinate system as root.
    try {
        transformAllComponents(uidManager.GetRootComponent());
    }
    catch (tigl::CTiglError& ex) {
        LOG(ERROR) << ex.getError() << std::endl;
    }
}

// Write CPACS structure to tixiHandle
void CCPACSConfiguration::WriteCPACS(const std::string& configurationUID)
{
    header.WriteCPACS(tixiDocumentHandle);
    
    TixiSaveExt::TixiSaveTextAttribute(tixiDocumentHandle, "/cpacs/vehicles/aircraft/model", "uID", configurationUID.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiDocumentHandle, "/cpacs/vehicles/aircraft/model", "name", name.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiDocumentHandle, "/cpacs/vehicles/aircraft/model", "description", description.c_str());

    fuselages.WriteCPACS(tixiDocumentHandle, configurationUID);
    wings.WriteCPACS(tixiDocumentHandle, configurationUID);

    std::string structurePath = "/cpacs/vehicles/profiles";
    structuralProfiles.WriteCPACS(tixiDocumentHandle, structurePath);
        
    std::string structureElementsPath = "/cpacs/vehicles";
    structuralElements.WriteCPACS(tixiDocumentHandle, structureElementsPath);
    materials.WriteCPACS(tixiDocumentHandle, structureElementsPath);
}

// transform all components relative to their parents
void CCPACSConfiguration::transformAllComponents(CTiglAbstractPhysicalComponent* parent)
{
    // [[CAS_AES]] handle case when no parent object exists
    if (!parent) {
        return;
    }

    CTiglAbstractPhysicalComponent::ChildContainerType children = parent->GetChildren(false);
    CTiglAbstractPhysicalComponent::ChildContainerType::iterator pIter;
    CTiglPoint parentTranslation = parent->GetTranslation();
    for (pIter = children.begin(); pIter != children.end(); ++pIter) {
        CTiglAbstractPhysicalComponent* child = *pIter;
        if (child->GetTranslationType() == ABS_LOCAL) {
            child->Translate(parentTranslation);
        }
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

bool CCPACSConfiguration::HasWingProfile(std::string uid) const
{
    return wings.HasProfile(uid);
}

// Returns the total count of wing profiles in this configuration
int CCPACSConfiguration::GetWingProfileCount(void) const
{
    return wings.GetProfileCount();
}

// Returns the class which holds all wing profiles
CCPACSWingProfiles& CCPACSConfiguration::GetWingProfiles(void)
{
    return wings.GetProfiles();
}

//Victor
CCPACSFuselageProfiles& CCPACSConfiguration::GetFuselageProfiles(void)
{
    return fuselages.GetProfiles();
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

TopoDS_Shape CCPACSConfiguration::GetParentLoft(const std::string& UID)
{
    return uidManager.GetParentComponent(UID)->GetLoft()->Shape();
}

bool CCPACSConfiguration::HasFuselageProfile(std::string uid) const
{
    return fuselages.HasProfile(uid);
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


CCPACSFuselages& CCPACSConfiguration::GetFuselages()
{
    return fuselages;
}


CCPACSFarField& CCPACSConfiguration::GetFarField()
{
    return farField;
}

// Returns the fuselage for a given UID.
CCPACSFuselage& CCPACSConfiguration::GetFuselage(const std::string UID) const
{
    return fuselages.GetFuselage(UID);
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
const std::string& CCPACSConfiguration::GetUID(void) const
{
    return configUID;
}

CTiglShapeCache& CCPACSConfiguration::GetShapeCache()
{
    return shapeCache;
}

CTiglMemoryPool& CCPACSConfiguration::GetMemoryPool()
{
    return memoryPool;
}

// [[CAS_AES]] Returns the total count of structural element in a configuration
int CCPACSConfiguration::GetProfileElementCount(void) const
{
    return structuralElements.GetProfiles()->GetProfileBasedStructuralElementCount();
}
// [[CAS_AES]] Returns the structural element for a given index.
CCPACSProfileBasedStructuralElement& CCPACSConfiguration::GetProfileElement(int index) const
{
    return structuralElements.GetProfiles()->GetProfileBasedStructuralElement(index);
}
// [[CAS_AES]] Returns the structural element for a given UID.
CCPACSProfileBasedStructuralElement& CCPACSConfiguration::GetProfileElement(const std::string UID) const
{
    return structuralElements.GetProfiles()->GetProfileBasedStructuralElement(UID);
}

// [[CAS_AES]] Returns the total count of structural profile in a configuration
int CCPACSConfiguration::GetStructuralProfileCount(void) const
{
    return structuralProfiles.GetStructuralProfile2DCount();
}
// [[CAS_AES]] Returns the structural profile for a given index.
CCPACSStructuralProfile2D& CCPACSConfiguration::GetStructuralProfile(int index) const
{
    return structuralProfiles.GetStructuralProfile2D(index);
}
// [[CAS_AES]] Returns the structural profile for a given UID.
CCPACSStructuralProfile2D& CCPACSConfiguration::GetStructuralProfile(const std::string UID) const
{
    return structuralProfiles.GetStructuralProfile2D(UID);
}

// [[CAS_AES]] added getter for free form surfaces
CCPACSFreeFormSurfaces& CCPACSConfiguration::GetFFFS()
{
    return freeFormSurfaces;
}

// [[CAS_AES]] added getter for number of free form surfaces
int CCPACSConfiguration::GetFFFSCount()
{
    return freeFormSurfaces.GetFreeFormSurfaceCount();
}

// [[CAS_AES]] added getter for free form surface by index
CCPACSFreeFormSurface& CCPACSConfiguration::GetFFFSbyIndex(int index)
{
    return freeFormSurfaces.GetFreeFormSurface(index);
}

// [[CAS_AES]]
CCPACSMaterialType* CCPACSConfiguration::GetMaterial(int index) const
{
    return materials.GetMaterial(index);
}

// [[CAS_AES]]
CCPACSMaterialType* CCPACSConfiguration::GetMaterial(std::string nUId) const
{
    return materials.GetMaterial(nUId);
}

// [[CAS_AES]]
CCPACSComposite* CCPACSConfiguration::GetComposite(std::string nUId) const
{
    return materials.GetComposite(nUId);
}

// [[CAS_AES]]
CCPACSComposite* CCPACSConfiguration::GetComposite(int index) const
{
    return materials.GetComposite(index);
}

// [[CAS_AES]] Returns the structural element for a given index.
CCPACSSheetBasedStructuralElement& CCPACSConfiguration::GetSheetElement(int index) const
{
    return structuralElements.GetSheets()->GetSheetBasedStructuralElement(index);
}
// [[CAS_AES]] Returns the structural element for a given UID.
CCPACSSheetBasedStructuralElement& CCPACSConfiguration::GetSheetElement(const std::string UID) const
{
    return structuralElements.GetSheets()->GetSheetBasedStructuralElement(UID);
}

CCPACSPressureBulkheadElement& CCPACSConfiguration::getPressureBulheadElement(std::string nUId) const
{
    return structuralElements.getPressureBulkheadElements()->GetPressureBulkheadElements(nUId);
}

CCPACSPressureBulkheadElement& CCPACSConfiguration::getPressureBulheadElement(int index) const
{
    return structuralElements.getPressureBulkheadElements()->GetPressureBulkheadElements(index);
}

CCPACSDoor& CCPACSConfiguration::getDoorElement(std::string nUId) const
{
    return structuralElements.getDoors()->GetDoor(nUId);
}

CCPACSDoor& CCPACSConfiguration::getDoorElement(int index) const
{
    return structuralElements.getDoors()->GetDoor(index);
}


CCPACSWingFuelTank& CCPACSConfiguration::getWingFuelTank(std::string nUId, bool& sym) const
{
    return wings.GetWingFuelTank(nUId, sym);
}

CCPACSWingFuelTank& CCPACSConfiguration::getWingFuelTank(std::string nUId, std::string& wingUId) const
{
    return wings.GetWingFuelTank(nUId, wingUId);
}

CTiglStructuralMountParent& CCPACSConfiguration::getStructuralMount(std::string nUId, std::string& parentUID, TiglSymmetryAxis& symmetryAxis) const
{
    for (int w = 1; w <= GetWingCount(); w++) {
        tigl::CCPACSWing& wing = GetWing(w);
        
        for ( int l = 1; l <= wing.GetComponentSegmentCount(); l++) {
            tigl::CCPACSWingComponentSegment& CpacsWCSegment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(l);
            
            CCPACSWingStructuralMounts& wStrMounts = CpacsWCSegment.GetWingStructuralMounts();
            
            for (int wsm = 1; wsm <= wStrMounts.GetWingStructuralMountCount(); wsm++) {
                CCPACSWingStructuralMount& mount = wStrMounts.GetWingStructuralMount(wsm);
                
                if (mount.GetUID() == nUId) {
                    mount.Update();
                    parentUID = wing.GetUID();
                    symmetryAxis = wing.GetSymmetryAxis();
                    return mount;
                }
            }
        }
    }
    
    for (int f = 1; f <= GetFuselageCount(); f++) {
        tigl::CCPACSFuselageStructuralMounts& fStrMounts = GetFuselage(f).GetFuselageStructure().getFuselageStructuralMounts();
        
        for (int fsm = 1; fsm <= fStrMounts.GetFuselageStructuralMountCount(); fsm++) {
            CCPACSFuselageStructuralMount& mount = fStrMounts.GetFuselageStructuralMount(fsm);
            
            if (mount.GetUID() == nUId) {
                mount.Update();
                parentUID = GetFuselage(f).GetUID();
                symmetryAxis = GetFuselage(f).GetSymmetryAxis();
                return mount;
            }
        }
    }
    
    throw CTiglError("Error: Structural Mount with uID " + nUId + " not found!\nIn CCPACSConfiguration::getStructuralMount");
    
}

std::string CCPACSConfiguration::GetName(void) const   // EU
{
    return name;
}

std::string CCPACSConfiguration::GetDescription(void) const   // EU
{
    return description;
}

CCPACSHeader* CCPACSConfiguration::GetHeader()    // EU
{
    return &header;
}

CCPACSWings* CCPACSConfiguration::GetWings()  // EU
{
    return &wings;
}

CCPACSMaterials* CCPACSConfiguration::GetMaterials()  // EU
{
    return &materials;
}

CCPACSStructuralProfiles* CCPACSConfiguration::GetStructuralProfiles()  // EU
{
    return &structuralProfiles;
}

CCPACSStructuralElements* CCPACSConfiguration::GetStructuralElements()  // EU
{
    return &structuralElements;
}

} // end namespace tigl

