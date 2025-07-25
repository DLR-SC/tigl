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

#ifndef CCPACSROTOR_H
#define CCPACSROTOR_H

#include <string>

#include "generated/CPACSRotor.h"
#include "tigl_config.h"
#include "CTiglTransformation.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSRotorHub.h"

#include "TopoDS_Shape.hxx"


namespace tigl
{

enum TiglRotorType
{
    TIGLROTOR_UNDEFINED,
    TIGLROTOR_MAIN_ROTOR,
    TIGLROTOR_TAIL_ROTOR,
    TIGLROTOR_FENESTRON,
    TIGLROTOR_PROPELLER
};

class CCPACSConfiguration;

class CCPACSRotor : public generated::CPACSRotor, public CTiglRelativelyPositionedComponent
{

public:
    // Constructor
    TIGL_EXPORT CCPACSRotor(CCPACSRotors* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    TIGL_EXPORT CTiglTransformation GetTransformationMatrix() const override;

    // Read CPACS rotor elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& rotorXPath) override;

    // Returns the Translation
    TIGL_EXPORT CTiglPoint GetTranslation() const override;

    // Returns the type of the rotor
    TIGL_EXPORT TiglRotorType GetDefaultedType() const;

    // Returns the rotor blade attachment count
    TIGL_EXPORT size_t GetRotorBladeAttachmentCount() const;

    // Returns the rotor blade attachment for a given index
    TIGL_EXPORT const CCPACSRotorBladeAttachment& GetRotorBladeAttachment(size_t index) const;
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(size_t index);

    // Returns the rotor blade count
    TIGL_EXPORT size_t GetRotorBladeCount() const;

    // Returns the rotor blade for a given index
    TIGL_EXPORT const CTiglAttachedRotorBlade& GetRotorBlade(size_t index) const;
    TIGL_EXPORT CTiglAttachedRotorBlade& GetRotorBlade(size_t index);

    // Returns the parent configuration
    TIGL_EXPORT const CCPACSConfiguration& GetConfiguration() const;
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration();

    // Returns the rotor disk geometry
    TIGL_EXPORT PNamedShape GetRotorDisk();

    // Returns the volume of this rotor
    TIGL_EXPORT double GetVolume();

    // Returns the surface area of this rotor
    TIGL_EXPORT double GetSurfaceArea();

    // Returns the reference area of this rotor.
    // Here, we always take the reference rotor disk area projected to a plane normal to the rotor hub z direction
    TIGL_EXPORT double GetReferenceArea();

    // Returns the radius of the rotor
    TIGL_EXPORT double GetRadius();

    // Wrapper function to return the nominal rotations per minute or 0 if not defined
    TIGL_EXPORT double GetNominalRotationsPerMinute();

    // Returns the tip speed this rotor
    TIGL_EXPORT double GetTipSpeed();

    // Returns the sum of all blade planform areas of a rotor
    TIGL_EXPORT double GetTotalBladePlanformArea();

    // Returns the rotor solidity
    TIGL_EXPORT double GetSolidity();

    // Returns the Component Type TIGL_COMPONENT_ROTOR.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override
    {
        return TIGL_COMPONENT_ROTOR;
    }

    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override
    {
        return TIGL_INTENT_PHYSICAL;
    }

protected:
    // Cleanup routine
    void Cleanup();

    // Update internal rotor data
    void Update();

    // Returns the geometry of the whole rotor (assembly of all rotor blades).
    // Implementation for abstract base class CTiglAbstractGeometricComponent
    PNamedShape BuildLoft() const override;

private:
    // Invalidates internal state
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

};

} // end namespace tigl

#endif // CCPACSROTOR_H
