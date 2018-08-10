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

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT CTiglTransformation GetTransformationMatrix() const OVERRIDE;

    // Read CPACS rotor elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& rotorXPath) OVERRIDE;

    // Returns the Translation
    TIGL_EXPORT CTiglPoint GetTranslation() const OVERRIDE;

    // Returns the type of the rotor
    TIGL_EXPORT TiglRotorType GetDefaultedType() const;

    // Returns the rotor blade attachment count
    TIGL_EXPORT int GetRotorBladeAttachmentCount() const;

    // Returns the rotor blade attachment for a given index
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(int index) const;

    // Returns the rotor blade count
    TIGL_EXPORT int GetRotorBladeCount() const;

    // Returns the rotor blade for a given index
    TIGL_EXPORT CTiglAttachedRotorBlade& GetRotorBlade(int index) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

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

    // Returns the tip speed this rotor
    TIGL_EXPORT double GetTipSpeed();

    // Returns the sum of all blade planform areas of a rotor
    TIGL_EXPORT double GetTotalBladePlanformArea();

    // Returns the rotor solidity
    TIGL_EXPORT double GetSolidity();

    // Returns the Component Type TIGL_COMPONENT_ROTOR.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE
    {
        return TIGL_COMPONENT_ROTOR | TIGL_COMPONENT_PHYSICAL;
    }

protected:
    // Cleanup routine
    void Cleanup();

    // Update internal rotor data
    void Update();

    // Returns the geometry of the whole rotor (assembly of all rotor blades).
    // Implementation for abstract base class CTiglAbstractGeometricComponent
    PNamedShape BuildLoft() const OVERRIDE;

private:
    bool                   invalidated;                 /**< Internal state flag  */
    bool                   rebuildGeometry;             /**< Indicates if geometry needs to be rebuilt */
};

} // end namespace tigl

#endif // CCPACSROTOR_H
