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
* @brief  Implementation of CPACS rotor hub handling routines.
*/

#ifndef CCPACSROTORHUB_H
#define CCPACSROTORHUB_H

#include <string>

#include "tixi.h"
#include "CCPACSRotorBladeAttachments.h"
//TODO:#include "CCPACSRotorBlades.h"


namespace tigl
{

enum TiglRotorHubType {
    TIGLROTORHUB_UNDEFINED,
    TIGLROTORHUB_SEMI_RIGID,
    TIGLROTORHUB_RIGID,
    TIGLROTORHUB_ARTICULATED,
    TIGLROTORHUB_HINGELESS
};
typedef enum TiglRotorHubType TiglRotorHubType;

class CCPACSConfiguration;

class CCPACSRotorHub
{

public:
    // Constructor
    CCPACSRotorHub(CCPACSConfiguration* config);

    // Virtual destructor
    virtual ~CCPACSRotorHub(void);

    // Invalidates internal state
    void Invalidate(void);

    // Read CPACS rotorHub elements
    void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorHubXPath);

    // Returns the UID of the rotor hub
    std::string GetUID(void) const;

    // Returns the name of the rotor hub
    const std::string& GetName(void) const;

    // Returns the description of the rotor hub
    const std::string& GetDescription(void) const;

    // Returns the type of the rotor hub
    const TiglRotorHubType& GetType(void) const;

    // Get rotor blade attachment count
    int GetRotorBladeAttachmentCount(void) const;

    // Returns the rotor blade attachment for a given index
    CCPACSRotorBladeAttachment& GetRotorBladeAttachment(int index) const;

    // Returns the parent configuration
    CCPACSConfiguration& GetConfiguration(void) const;

//TODO:        // Get rotor blade count
//TODO:        int GetRotorBladeCount(void) const;

//TODO:        // Returns the rotor blade for a given index
//TODO:        CCPACSRotorBlade& GetRotorBlade(int index) const;

protected:
    // Cleanup routine
    void Cleanup(void);

    // Update internal rotor hub data
    void Update(void);

//TODO:        // Adds all Segments of this rotor to one shape
//TODO:        TopoDS_Shape BuildRotorGeometry(void);


private:
    // Copy constructor
    CCPACSRotorHub(const CCPACSRotorHub&);

    // Assignment operator
    void operator=(const CCPACSRotorHub&);

private:
    std::string                    uID;                     /**< Rotor hub uID        */
    std::string                    name;                    /**< Rotor hub name       */
    std::string                    description;             /**< Rotor hub description*/
    TiglRotorHubType               type;                    /**< Rotor hub type       */
    CCPACSRotorBladeAttachments    rotorBladeAttachments;   /**< Rotor blade attachments of the rotor hub */
//TODO:        CCPACSRotorBlades              rotorBlades;             /**< Rotor blades         */
    CCPACSConfiguration*           configuration;           /**< Parent configuration */
    bool                           invalidated;             /**< Internal state flag  */
};

} // end namespace tigl

#endif // CCPACSROTORHUB_H
