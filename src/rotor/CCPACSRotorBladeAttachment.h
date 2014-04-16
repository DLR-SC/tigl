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
* @brief  Implementation of CPACS rotor blade attachment handling routines.
*/

#ifndef CCPACSROTORBLADEATTACHMENT_H
#define CCPACSROTORBLADEATTACHMENT_H

#include <string>
#include <vector>

#include "tixi.h"
#include "CCPACSRotorBlades.h"
#include "CCPACSRotorHinges.h"


namespace tigl
{

class CCPACSConfiguration;
class CCPACSRotor;

class CCPACSRotorBladeAttachment
{

public:
    // Constructor
    TIGL_EXPORT CCPACSRotorBladeAttachment(CCPACSRotor* rotor);

    // Virtual destructor
    TIGL_EXPORT virtual ~CCPACSRotorBladeAttachment(void);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Read CPACS rotor elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorBladeAttachmentXPath);

    // Builds and returns the transformation matrix for an attached rotor blade
    TIGL_EXPORT CTiglTransformation GetRotorBladeTransformationMatrix(double thetaDeg=0., double bladeDeltaThetaDeg=0., bool doHingeTransformation=true, bool doRotorTransformation=false);

    // Returns the UID of the rotor blade attachment
    TIGL_EXPORT const std::string& GetUID(void) const;

    // Returns the number of attached rotor blades
    TIGL_EXPORT int GetNumberOfBlades(void) const;

    // Returns the azimuth angle of the attached rotor blade with the given index
    TIGL_EXPORT const double& GetAzimuthAngle(int index) const;

    // Returns the UID of the referenced rotor blade
    TIGL_EXPORT const std::string& GetRotorBladeUID(void) const;

    // Get hinge count
    TIGL_EXPORT int GetHingeCount(void) const;

    // Returns the hinge for a given index
    TIGL_EXPORT CCPACSRotorHinge& GetHinge(int index) const;

    // Get rotor blade count
    TIGL_EXPORT int GetRotorBladeCount(void) const;

    // Returns the rotor blade for a given index
    TIGL_EXPORT CCPACSRotorBlade& GetRotorBlade(int index) const;

    // Returns the parent rotor
    TIGL_EXPORT CCPACSRotor& GetRotor(void) const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration& GetConfiguration(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

    // Update internal rotor blade attachment data
    void Update(void);

private:
    // Copy constructor
    CCPACSRotorBladeAttachment(const CCPACSRotorBladeAttachment&);

    // Assignment operator
    void operator=(const CCPACSRotorBladeAttachment&);

private:
    std::string                    uID;                      /**< Rotor hub uID        */
    std::vector<double>            azimuthAngles;            /**< Rotor hub type       */
    std::string                    rotorBladeUID;            /**< Rotor blade uID      */
    CCPACSRotorHinges              hinges;                   /**< Hinges               */
    CCPACSRotorBlades              rotorBlades;              /**< Rotor blades         */
    CCPACSRotor*                   rotor;                    /**< Parent rotor         */
    bool                           invalidated;              /**< Internal state flag  */
};

} // end namespace tigl

#endif // CCPACSROTORBLADEATTACHMENT_H
