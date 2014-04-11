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
#include "CTiglTransformation.h"
#include "CCPACSRotorHinges.h"
//TODO:#include "CCPACSRotorBlades.h"


namespace tigl
{

class CCPACSConfiguration;

class CCPACSRotorBladeAttachment
{

public:
    // Constructor
    CCPACSRotorBladeAttachment(CCPACSConfiguration* config);

    // Virtual destructor
    virtual ~CCPACSRotorBladeAttachment(void);

    // Invalidates internal state
    void Invalidate(void);

    // Read CPACS rotor elements
    void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorBladeAttachmentXPath);

    // Returns the UID of the rotor blade attachment
    const std::string& GetUID(void) const;

    // Returns the number of attached rotor blades
    int GetNumberOfBlades(void) const;

    // Returns the azimuth angle of the attached rotor blade with the given index
    const double& GetAzimuthAngle(int index) const;

    // Returns the UID of the referenced rotor blade
    const std::string& GetRotorBladeUID(void) const;

    // Returns the parent configuration
    CCPACSConfiguration& GetConfiguration(void) const;

    // Get hinge count
    int GetHingeCount(void) const;

    // Returns the hinge for a given index
    CCPACSRotorHinge& GetHinge(int index) const;

//TODO:        // Get rotor blade count
//TODO:        int GetRotorBladeCount(void) const;

//TODO:        // Returns the rotor blade for a given index
//TODO:        CCPACSRotorBlade& GetRotorBlade(int index) const;

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
//TODO:        CCPACSRotorBlades              rotorBlades;              /**< Rotor blades         */
    CCPACSConfiguration*           configuration;            /**< Parent configuration */
    bool                           invalidated;              /**< Internal state flag  */
};

} // end namespace tigl

#endif // CCPACSROTORBLADEATTACHMENT_H
