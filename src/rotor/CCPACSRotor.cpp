/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-11 Philipp Kunze <Philipp.Kunze@dlr.de>
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
* @brief  Implementation of CPACS rotor handling routines.
*/

#include <iostream>

#include "CCPACSRotor.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"

#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRep_Builder.hxx"

namespace
{
inline double max(double a, double b)
{
    return a > b? a : b;
}
}

namespace tigl
{

// Constructor
CCPACSRotor::CCPACSRotor(CCPACSConfiguration* config)
    : configuration(config)
    , rotorHub(this)
    , rebuildGeometry(true)
{
    Cleanup();
}

// Destructor
CCPACSRotor::~CCPACSRotor(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSRotor::Invalidate(void)
{
    invalidated = true;
    rotorHub.Invalidate();
}

// Cleanup routine
void CCPACSRotor::Cleanup(void)
{
    name = "";
    description = "";
    transformation.reset();
    type = TIGLROTOR_MAIN_ROTOR;
    nominalRotationsPerMinute = 0.;

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Update internal rotor data
void CCPACSRotor::Update(void)
{
    if (!invalidated) {
        return;
    }

    transformation.updateMatrix();
    invalidated = false;
    rebuildGeometry = true;

    // Update all rotor blade transformations
    for (int i=1; i<=GetRotorBladeAttachmentCount(); ++i) {
        for (int j=1; j<=GetRotorBladeAttachment(i).GetRotorBladeCount(); ++j) {
            GetRotorBladeAttachment(i).GetRotorBlade(j).Invalidate();
            GetRotorBladeAttachment(i).GetRotorBlade(j).Update();
        }
    }
}

// Read CPACS rotor element
void CCPACSRotor::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get attribute "uid"
    char* ptrUID = NULL;
    tempString   = "uID";
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(rotorXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = rotorXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name      = ptrName;
    }

    // Get subelement "description"
    char* ptrDescription = NULL;
    tempString           = rotorXPath + "/description";
    elementPath          = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrDescription) == SUCCESS) {
        description      = ptrDescription;
    }

    // Get subelement "parent_uid"
    char* ptrParentUID = NULL;
    tempString         = rotorXPath + "/parentUID";
    elementPath        = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS &&
        tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID) == SUCCESS) {

        SetParentUID(ptrParentUID);
    }
    
    // Get Transformation
    transformation.ReadCPACS(tixiHandle, rotorXPath);

    // Get subelement "type"
    char* ptrType = NULL;
    tempString    = rotorXPath + "/type";
    if (tixiGetTextElement(tixiHandle, tempString.c_str(), &ptrType) == SUCCESS) {
        std::string strType(ptrType);
        if (strType == "mainRotor") {
            type = TIGLROTOR_MAIN_ROTOR;
        }
        else if (strType == "tailRotor") {
            type = TIGLROTOR_TAIL_ROTOR;
        }
        else if (strType == "fenestron") {
            type = TIGLROTOR_FENESTRON;
        }
        else if (strType == "propeller") {
            type = TIGLROTOR_PROPELLER;
        }
        else {
            throw CTiglError("Error: XML error while reading <type> in CCPACSRotor::ReadCPACS: illegal value", TIGL_XML_ERROR);
        }
    }
    else {   // default value: MAIN_ROTOR
        type = TIGLROTOR_MAIN_ROTOR;
    }

    // Get subelement "nominalRotationsPerMinute"
    double tmpDouble = 0.;
    tempString       = rotorXPath + "/nominalRotationsPerMinute";
    elementPath      = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle,elementPath)== SUCCESS) {
        if (tixiGetDoubleElement(tixiHandle, elementPath, &tmpDouble) == SUCCESS) {
            nominalRotationsPerMinute = tmpDouble;
        }
    }

    // Get subelement "rotorHub"
    rotorHub.ReadCPACS(tixiHandle, rotorXPath + "/rotorHub");

    // Register ourself at the unique id manager
    configuration->GetUIDManager().AddUID(ptrUID, this);

    // Get symmetry axis attribute, has to be done, when rotor blades are build
    char* ptrSym = NULL;
    tempString   = "symmetry";
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(rotorXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(ptrSym);
    }

    Update();
}

// Returns the name of the rotor
const std::string& CCPACSRotor::GetName(void) const
{
    return name;
}

// Returns the description of the rotor
const std::string& CCPACSRotor::GetDescription(void) const
{
    return description;
}

// Get the Transformation object
CTiglTransformation CCPACSRotor::GetTransformation(void)
{
    Update();   // create new transformation matrix if scaling, rotation or translation was changed
    return transformation.getTransformationMatrix();
}

// Sets the Transformation object
void CCPACSRotor::Translate(CTiglPoint trans)
{
    CTiglAbstractGeometricComponent::Translate(trans);
    Invalidate();
    Update();
}

// Get Translation
CTiglPoint CCPACSRotor::GetTranslation(void)
{
    Update();
    return transformation.getTranslationVector();
}

// Returns the type of the rotor
const TiglRotorType& CCPACSRotor::GetType(void) const
{
    return type;
}

// Returns the nominal rotations per minute (rpm) of the rotor
const double& CCPACSRotor::GetNominalRotationsPerMinute(void) const
{
    return nominalRotationsPerMinute;
}

// Returns the rotor blade attachment count
int CCPACSRotor::GetRotorBladeAttachmentCount(void) const
{
    return rotorHub.GetRotorBladeAttachmentCount();
}

// Returns the rotor blade attachment for a given index
CCPACSRotorBladeAttachment& CCPACSRotor::GetRotorBladeAttachment(int index) const
{
    return rotorHub.GetRotorBladeAttachment(index);
}

// Returns the rotor blade count
int CCPACSRotor::GetRotorBladeCount(void) const
{
    return rotorHub.GetRotorBladeCount();
}

// Returns the rotor blade for a given index
CCPACSRotorBlade& CCPACSRotor::GetRotorBlade(int index) const
{
    return rotorHub.GetRotorBlade(index);
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotor::GetConfiguration(void) const
{
    return *configuration;
}

// Returns the rotor hub object
const CCPACSRotorHub& CCPACSRotor::GetRotorHub(void) const
{
    return rotorHub;
}

// Returns the rotor disk geometry (of the first attached blade)
PNamedShape CCPACSRotor::GetRotorDisk(void)
{
    // Create the rotor disk
    TopoDS_Shape rotorDisk;
    if (GetRotorBladeAttachmentCount() > 0) {
        if (GetRotorBladeAttachment(1).GetRotorBladeCount() > 0) {
            rotorDisk = GetRotorBladeAttachment(1).GetRotorBlade(1).GetRotorDisk();
        }
    }
    
    PNamedShape diskShape(new CNamedShape(rotorDisk, std::string(GetUID() + "_Disk").c_str()));
    return diskShape;
}

// Returns the geometry of the whole rotor (assembly of all rotor blades)
PNamedShape CCPACSRotor::BuildLoft(void)
{
    // Create rotor assembly
    TopoDS_Compound rotorGeometry;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound(rotorGeometry);
    for (int iAttach=1; iAttach <= rotorHub.GetRotorBladeAttachmentCount(); ++iAttach) {
        CCPACSRotorBladeAttachment& attach = rotorHub.GetRotorBladeAttachment(iAttach);
        for (int iBlade=1; iBlade <= attach.GetRotorBladeCount(); ++iBlade) {
            // Add the transformed rotor blade to the rotor assembly
            TopoDS_Shape bladeShape = attach.GetRotorBlade(iBlade).GetLoft()->Shape();
            aBuilder.Add(rotorGeometry, bladeShape);
        }
    }
    // Return the generated geometry
    PNamedShape rotorShape(new CNamedShape(rotorGeometry, GetUID().c_str()));
    
    return rotorShape;
}

// Returns the volume of this rotor
double CCPACSRotor::GetVolume(void)
{
    TopoDS_Shape fusedRotorBlades = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedRotorBlades, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Returns the surface area of this rotor
double CCPACSRotor::GetSurfaceArea(void)
{
    TopoDS_Shape fusedRotorBlades = GetLoft()->Shape();

    // Calculate surface area
    GProp_GProps System;
    BRepGProp::SurfaceProperties(fusedRotorBlades, System);
    double myArea = System.Mass();
    return myArea;
}

// Returns the reference area of this rotor
// Here, we always take the reference rotor disk area projected to a plane normal to the rotor hub direction
double CCPACSRotor::GetReferenceArea(void)
{
    double r = GetRadius();
    return M_PI * r*r;
}

// Returns the radius of this rotor
double CCPACSRotor::GetRadius(void)
{
    double rotorRadius = 0.0;
    // Get the blade maximum rotor radius
    for (int i=1; i<=GetRotorBladeAttachmentCount(); ++i) {
        if (GetRotorBladeAttachment(i).GetRotorBladeCount() > 0) {
            rotorRadius = max(GetRotorBladeAttachment(i).GetRotorBlade(1).GetRadius(), rotorRadius);
        }
    }
    return rotorRadius;
}

// Returns the tip speed this rotor
double CCPACSRotor::GetTipSpeed(void)
{
    // return GetNominalRotationsPerMinute()/60. * 2.*M_PI*GetRadius();
    return GetNominalRotationsPerMinute()/30. * M_PI*GetRadius();
}

// Returns the sum of all blade planform areas of a rotor
double CCPACSRotor::GetTotalBladePlanformArea(void)
{
    double totalRotorBladeArea = 0.0;
    // Add rotor blade planform areas
    for (int i=1; i<=GetRotorBladeAttachmentCount(); ++i) {
        if (GetRotorBladeAttachment(i).GetRotorBladeCount() > 0) {
            totalRotorBladeArea += GetRotorBladeAttachment(i).GetRotorBladeCount() * GetRotorBladeAttachment(i).GetRotorBlade(1).GetPlanformArea();
        }
    }
    return totalRotorBladeArea;
}

// Returns the rotor solidity
double CCPACSRotor::GetSolidity(void)
{
    return GetTotalBladePlanformArea()/GetReferenceArea();
}

} // end namespace tigl
