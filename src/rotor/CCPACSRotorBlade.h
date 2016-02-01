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
* @brief  Implementation of rotor blade handling routines.
*/

#ifndef CCPACSROTORBLADE_H
#define CCPACSROTORBLADE_H

#include "CTiglAbstractPhysicalComponent.h"

namespace tigl
{

class CCPACSWing;
class CCPACSConfiguration;
class CCPACSRotor;
class CCPACSRotorBladeAttachment;

class CCPACSRotorBlade : public CTiglAbstractPhysicalComponent
{

public:
    // Constructor
    TIGL_EXPORT CCPACSRotorBlade(CCPACSRotorBladeAttachment* rotorBladeAttachment, CCPACSWing* wing, int rotorBladeIndex);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSRotorBlade(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Update internal data
    TIGL_EXPORT void Update(void);

    // Returns the Component Type TIGL_COMPONENT_ROTORBLADE
    TIGL_EXPORT TiglGeometricComponentType GetComponentType(void)
    {
        return TIGL_COMPONENT_ROTORBLADE;
    }

    // Returns the original unattached rotor blade
    TIGL_EXPORT CCPACSWing& GetUnattachedRotorBlade(void) const;

    // Returns the parent rotor blade attachment this rotor blade belongs to
    TIGL_EXPORT CCPACSRotorBladeAttachment& GetRotorBladeAttachment(void) const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor(void) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;

    // Returns the azimuth angle of this rotor blade
    TIGL_EXPORT double GetAzimuthAngle(void);

    // Returns the volume of this rotor blade
    TIGL_EXPORT double GetVolume(void);

    // Returns the surface area of this rotor blade
    TIGL_EXPORT double GetSurfaceArea(void);

    // Returns the planform area of this rotor blade
    TIGL_EXPORT double GetPlanformArea(void);

    // Returns the radius of this rotor blade
    TIGL_EXPORT double GetRadius(void);

    // Returns the tip speed of this rotor blade
    TIGL_EXPORT double GetTipSpeed(void);

    // Returns the radius of a point on the rotor blade quarter chord line for a given segment index and eta
    TIGL_EXPORT double GetLocalRadius(const int& segmentIndex, const double& eta);

    // Returns the rotor blade chord length for a given segment index and eta
    TIGL_EXPORT double GetLocalChord(const int& segmentIndex, const double& eta);

    // Returns the local rotor blade twist angle (in degrees) for a given segment index and eta
    TIGL_EXPORT double GetLocalTwistAngle(const int& segmentIndex, const double& eta);

    // Returns the rotor disk geometry
    TIGL_EXPORT TopoDS_Shape GetRotorDisk(void);

protected:
    // Cleanup routine
    void Cleanup(void);

    // Builds transformation matrix for the rotor blade including rotor transformation
    void BuildMatrix(void);

    // Create the rotor blade geometry by copying and transforming the original unattached rotor blade geometry
    TopoDS_Shape BuildLoft(void);

private:
    // Copy constructor
    CCPACSRotorBlade(const CCPACSRotorBlade&);

    // Assignment operator
    void operator=(const CCPACSRotorBlade&);

    // Creates a rotor disk (only using information of the current blade)
    TopoDS_Shape BuildRotorDisk(void);

    CCPACSRotorBladeAttachment*  rotorBladeAttachment; /**< Parent rotor blade attachment           */
    CCPACSWing*                  rotorBlade;           /**< Original unattached rotor blade         */
    int                          rotorBladeIndex;      /**< Index of the rotor blade in the rotor blade attachment */
    bool                         invalidated;          /**< Internal state flag */
    bool                         rebuildRotorBladeLoft;/**< State flag for the rotor blade loft geometry */
    bool                         rebuildRotorDisk;     /**< State flag for the rotor disk geometry */
    TopoDS_Shape                 rotorBladeLoft;       /**< Transformed lofted geometry of the rotor blade */
    TopoDS_Shape                 rotorDisk;            /**< Rotor disk geometry of the rotor blade */
    double                       myVolume;             /**< Volume of this rotor blade              */
    double                       mySurfaceArea;        /**< Surface area of this rotor blade        */
};

} // end namespace tigl

#endif // CCPACSROTORBLADE_H
