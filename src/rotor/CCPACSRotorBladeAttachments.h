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
* @brief  Implementation of CPACS blade attachments handling routines.
*/

#ifndef CCPACSROTORBLADEATTACHMENTS_H
#define CCPACSROTORBLADEATTACHMENTS_H

#include "tixi.h"
#include "CCPACSRotorBladeAttachment.h"
#include <string>
#include <vector>

namespace tigl
{

class CCPACSConfiguration;

class CCPACSRotorBladeAttachments
{
private:
    // Typedef for a CCPACSRotorBladeAttachment container to store the rotor blade attachments of a rotor hub.
    typedef std::vector<CCPACSRotorBladeAttachment*> CCPACSRotorBladeAttachmentContainer;

public:
    // Constructor
    CCPACSRotorBladeAttachments(CCPACSConfiguration* config);

    // Virtual Destructor
    virtual ~CCPACSRotorBladeAttachments(void);

    // Invalidates internal state
    void Invalidate(void);

    // Read CPACS rotorBladeAttachments elements
    void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string rotorBladeAttachmentsXPath, const std::string rotorBladeAttachmentElementName="rotorBladeAttachment");

    // Returns the total count of rotor blade attachments in a rotor hub
    int GetRotorBladeAttachmentCount(void) const;

    // Returns the rotor blade attachment for a given index.
    CCPACSRotorBladeAttachment& GetRotorBladeAttachment(int index) const;

    // Returns the parent configuration
    CCPACSConfiguration& GetConfiguration(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSRotorBladeAttachments(const CCPACSRotorBladeAttachments&);

    // Assignment operator
    void operator=(const CCPACSRotorBladeAttachments&);

private:
    CCPACSRotorBladeAttachmentContainer rotorBladeAttachments;  /**< RotorBladeAttachment elements   */
    CCPACSConfiguration* configuration;                         /**< Pointer to parent configuration */

};

} // end namespace tigl

#endif // CCPACSROTORBLADEATTACHMENTS_H
