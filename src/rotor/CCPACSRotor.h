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

#include "tigl_config.h"
#include "tixi.h"
#include "CTiglTransformation.h"
#include "CTiglAbstractPhysicalComponent.h"
#include "CCPACSRotorHub.h"

#include "TopoDS_Shape.hxx"


namespace tigl
{

enum TiglRotorType {
    TIGLROTOR_UNDEFINED,
    TIGLROTOR_MAIN_ROTOR,
    TIGLROTOR_TAIL_ROTOR,
    TIGLROTOR_FENESTRON,
    TIGLROTOR_PROPELLER
};
typedef enum TiglRotorType TiglRotorType;

class CCPACSConfiguration;

class CCPACSRotor : public CTiglAbstractPhysicalComponent
{

public:
    // Constructor
    TIGL_EXPORT CCPACSRotor(CCPACSConfiguration* config);

    // Virtual destructor
    TIGL_EXPORT virtual ~CCPACSRotor(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS rotor elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorXPath);

    // Returns the name of the rotor
    TIGL_EXPORT const std::string& GetName(void) const;

    // Returns the description of the rotor
    TIGL_EXPORT const std::string& GetDescription(void) const;

    // Returns the Transformation object
    TIGL_EXPORT CTiglTransformation GetTransformation(void);

    // Sets the Translation object
    TIGL_EXPORT void Translate(CTiglPoint trans);

    // Returns the Translation
    TIGL_EXPORT CTiglPoint GetTranslation(void);

    // Returns the type of the rotor
    TIGL_EXPORT const TiglRotorType& GetType(void) const;

    // Returns the nominal rotations per minute (rpm) of the rotor
    TIGL_EXPORT const double& GetNominalRotationsPerMinute(void) const;

    // Returns the rotor blade attachment count
    TIGL_EXPORT int GetRotorBladeAttachmentCount(void) const;

    // Returns the rotor blade attachment for a given index
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(int index) const;

    // Returns the rotor blade count
    TIGL_EXPORT int GetRotorBladeCount(void) const;

    // Returns the rotor blade for a given index
    TIGL_EXPORT CCPACSRotorBlade& GetRotorBlade(int index) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;

    // Returns the rotor hub object
    TIGL_EXPORT const CCPACSRotorHub& GetRotorHub(void) const;

    // Returns the rotor disk geometry
    TIGL_EXPORT TopoDS_Shape GetRotorDisk(void);

    // Returns the volume of this rotor
    TIGL_EXPORT double GetVolume(void);

    // Returns the surface area of this rotor
    TIGL_EXPORT double GetSurfaceArea(void);

    // Returns the reference area of this rotor.
    // Here, we always take the reference rotor disk area projected to a plane normal to the rotor hub z direction
    TIGL_EXPORT double GetReferenceArea(void);

    // Returns the radius of the rotor
    TIGL_EXPORT double GetRadius(void);

    // Returns the diameter of this rotor
    TIGL_EXPORT double GetDiameter(void);

    // Returns the sum of all blade planform areas of a rotor
    TIGL_EXPORT double GetTotalBladePlanformArea(void);

    // Returns the rotor solidity
    TIGL_EXPORT double GetSolidity(void);

    // Returns the Component Type TIGL_COMPONENT_ROTOR.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType(void)
    {
        return TIGL_COMPONENT_ROTOR | TIGL_COMPONENT_PHYSICAL;
    }

protected:
    // Cleanup routine
    void Cleanup(void);

    // Build transformation matrix for the rotor
    void BuildMatrix(void);

    // Update internal rotor data
    void Update(void);

    // Returns the geometry of the whole rotor (assembly of all rotor blades). (implementation for abstract base class CTiglAbstractGeometricComponent)
    virtual TopoDS_Shape BuildLoft(void);

private:
    // Copy constructor
    CCPACSRotor(const CCPACSRotor&);

    // Assignment operator
    void operator=(const CCPACSRotor&);

private:
    std::string            name;                        /**< Rotor name           */
    std::string            description;                 /**< Rotor description    */
    TiglRotorType          type;                        /**< Rotor type           */
    double                 nominalRotationsPerMinute;   /**< Rotor type           */
    CCPACSRotorHub         rotorHub;                    /**< Rotor hub            */
    CCPACSConfiguration*   configuration;               /**< Parent configuration */
    bool                   invalidated;                 /**< Internal state flag  */
    bool                   rebuildGeometry;             /**< Indicates if geometry needs to be rebuilt */
};

} // end namespace tigl

#endif // CCPACSROTOR_H
