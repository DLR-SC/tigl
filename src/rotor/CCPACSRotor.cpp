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
#include <algorithm>

#include "CCPACSRotor.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include "CNamedShape.h"

#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRep_Builder.hxx"

namespace tigl
{

// Constructor
CCPACSRotor::CCPACSRotor(CCPACSRotors* parent)
    : generated::CPACSRotor(parent)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation, &m_symmetry)
    , rebuildGeometry(true) {}

// Invalidates internal state
void CCPACSRotor::Invalidate()
{
    invalidated = true;
}

// Cleanup routine
void CCPACSRotor::Cleanup()
{
    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Update internal rotor data
void CCPACSRotor::Update()
{
    if (!invalidated) {
        return;
    }

    invalidated = false;
    rebuildGeometry = true;
}

const std::string& CCPACSRotor::GetUID() const {
    return generated::CPACSRotor::GetUID();
}

// Read CPACS rotor element
void CCPACSRotor::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& rotorXPath)
{
    Cleanup();
    generated::CPACSRotor::ReadCPACS(tixiHandle, rotorXPath);
    m_parent->GetConfiguration().GetUIDManager().AddUID(m_uID, this);
    Update();
}

// Get the Transformation object
CTiglTransformation CCPACSRotor::GetTransformation() const
{
    const_cast<CCPACSRotor*>(this)->Update();   // create new transformation matrix if scaling, rotation or translation was changed, TODO: hack
    return m_transformation.getTransformationMatrix();
}

// Sets the Transformation object
void CCPACSRotor::Translate(CTiglPoint trans)
{
    CTiglRelativelyPositionedComponent::Translate(trans);
    Invalidate();
    Update();
}

// Get Translation
CTiglPoint CCPACSRotor::GetTranslation()
{
    Update();
    return m_transformation.getTranslationVector();
}

// Returns the type of the rotor
TiglRotorType CCPACSRotor::GetType() const
{
    if (!m_type)
        return ENUM_VALUE(TiglRotorType, TIGLROTOR_UNDEFINED);

    switch (*m_type) {
        case ENUM_VALUE(ECPACSRotor_type, fenestron): return ENUM_VALUE(TiglRotorType, TIGLROTOR_FENESTRON);
        case ENUM_VALUE(ECPACSRotor_type, mainRotor): return ENUM_VALUE(TiglRotorType, TIGLROTOR_MAIN_ROTOR);
        case ENUM_VALUE(ECPACSRotor_type, propeller): return ENUM_VALUE(TiglRotorType, TIGLROTOR_PROPELLER);
        case ENUM_VALUE(ECPACSRotor_type, tailRotor): return ENUM_VALUE(TiglRotorType, TIGLROTOR_TAIL_ROTOR);
        default: throw CTiglError("unknown rotor type");
    }
}

// Returns the rotor blade attachment count
int CCPACSRotor::GetRotorBladeAttachmentCount() const
{
    return m_rotorHub.GetRotorBladeAttachmentCount();
}

// Returns the rotor blade attachment for a given index
CCPACSRotorBladeAttachment& CCPACSRotor::GetRotorBladeAttachment(int index) const
{
    return m_rotorHub.GetRotorBladeAttachment(index);
}

// Returns the rotor blade count
int CCPACSRotor::GetRotorBladeCount() const
{
    return m_rotorHub.GetRotorBladeCount();
}

// Returns the rotor blade for a given index
CTiglAttachedRotorBlade& CCPACSRotor::GetRotorBlade(int index) const
{
    return m_rotorHub.GetRotorBlade(index);
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSRotor::GetConfiguration() const
{
    return m_parent->GetConfiguration();
}

// Returns the rotor disk geometry (of the first attached blade)
PNamedShape CCPACSRotor::GetRotorDisk()
{
    // Create the rotor disk
    TopoDS_Shape rotorDisk;
    if (GetRotorBladeAttachmentCount() > 0) {
        if (GetRotorBladeAttachment(1).GetNumberOfBlades() > 0) {
            rotorDisk = GetRotorBladeAttachment(1).GetAttachedRotorBlade(1).GetRotorDisk();
        }
    }
    
    PNamedShape diskShape(new CNamedShape(rotorDisk, std::string(GetUID() + "_Disk").c_str()));
    return diskShape;
}

// Returns the geometry of the whole rotor (assembly of all rotor blades)
PNamedShape CCPACSRotor::BuildLoft()
{
    // Create rotor assembly
    TopoDS_Compound rotorGeometry;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound(rotorGeometry);
    for (int iAttach=1; iAttach <= m_rotorHub.GetRotorBladeAttachmentCount(); ++iAttach) {
        CCPACSRotorBladeAttachment& attach = m_rotorHub.GetRotorBladeAttachment(iAttach);
        for (int iBlade=1; iBlade <= attach.GetNumberOfBlades(); ++iBlade) {
            // Add the transformed rotor blade to the rotor assembly
            TopoDS_Shape bladeShape = attach.GetAttachedRotorBlade(iBlade).GetLoft()->Shape();
            aBuilder.Add(rotorGeometry, bladeShape);
        }
    }
    // Return the generated geometry
    PNamedShape rotorShape(new CNamedShape(rotorGeometry, GetUID().c_str()));
    
    return rotorShape;
}

// Returns the volume of this rotor
double CCPACSRotor::GetVolume()
{
    TopoDS_Shape fusedRotorBlades = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedRotorBlades, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Returns the surface area of this rotor
double CCPACSRotor::GetSurfaceArea()
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
double CCPACSRotor::GetReferenceArea()
{
    double r = GetRadius();
    return M_PI * r*r;
}

// Returns the radius of this rotor
double CCPACSRotor::GetRadius()
{
    double rotorRadius = 0.0;
    // Get the blade maximum rotor radius
    for (int i=1; i<=GetRotorBladeAttachmentCount(); ++i) {
        if (GetRotorBladeAttachment(i).GetNumberOfBlades() > 0) {
            rotorRadius = std::max(GetRotorBladeAttachment(i).GetAttachedRotorBlade(1).GetRadius(), rotorRadius);
        }
    }
    return rotorRadius;
}

// Returns the tip speed this rotor
double CCPACSRotor::GetTipSpeed()
{
    // return GetNominalRotationsPerMinute()/60. * 2.*M_PI*GetRadius();
    return *GetNominalRotationsPerMinute() / 30. * M_PI * GetRadius();
}

// Returns the sum of all blade planform areas of a rotor
double CCPACSRotor::GetTotalBladePlanformArea()
{
    double totalRotorBladeArea = 0.0;
    // Add rotor blade planform areas
    for (int i=1; i<=GetRotorBladeAttachmentCount(); ++i) {
        if (GetRotorBladeAttachment(i).GetNumberOfBlades() > 0) {
            totalRotorBladeArea += GetRotorBladeAttachment(i).GetNumberOfBlades() * GetRotorBladeAttachment(i).GetAttachedRotorBlade(1).GetPlanformArea();
        }
    }
    return totalRotorBladeArea;
}

// Returns the rotor solidity
double CCPACSRotor::GetSolidity()
{
    return GetTotalBladePlanformArea()/GetReferenceArea();
}

} // end namespace tigl
