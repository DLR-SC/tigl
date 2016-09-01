/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Implementation of CPACS wing section element handling routines.
*/

#ifndef CCPACSWINGSECTIONELEMENT_H
#define CCPACSWINGSECTIONELEMENT_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSTransformation.h"
#include "CTiglPoint.h"
#include <string>

namespace tigl
{

class CCPACSWingSectionElement
{

public:
    // Constructor
    TIGL_EXPORT CCPACSWingSectionElement();

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingSectionElement(void);

    // Read CPACS wing section element
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath);

    // Write CPACS wing section element
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& elementXPath);

    // Returns the UID of the referenced wing profile
    TIGL_EXPORT std::string GetProfileIndex(void) const;

    // Returns the UID of the WingSectionElement
    TIGL_EXPORT std::string GetUID(void) const;

    // Getter for the member name
    TIGL_EXPORT std::string GetName(void) const;

    // Getter for the member description
    TIGL_EXPORT std::string GetDescription(void) const;

    // Returns the UID of the profile of this element
    TIGL_EXPORT std::string GetProfileUID(void) const;

    // Gets the section element transformation
    TIGL_EXPORT CTiglTransformation GetSectionElementTransformation(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSWingSectionElement(const CCPACSWingSectionElement& );

    // Assignment operator
    void operator=(const CCPACSWingSectionElement& );

private:
    std::string           name;           /**< Section name                          */
    std::string           description;    /**< Section name                          */
    std::string           profileUID;     /**< Profile index in wing profile library */
    std::string           uID;            /**< UID of the WingSectionElement         */
    CCPACSTransformation  transformation; /**< Section element transformation        */

};

} // end namespace tigl

#endif // CCPACSWINGSECTIONELEMENT_H
