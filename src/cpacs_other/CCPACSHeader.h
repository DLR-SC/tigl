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
* @brief  Implementation of CPACS configuration header handling routines.
*/

#ifndef CCPACSHEADER_H
#define CCPACSHEADER_H

#include <string>
#include "tigl_internal.h"
#include "tixi.h"

namespace tigl
{

class CCPACSHeader
{

public:
    // Constructor
    TIGL_EXPORT CCPACSHeader(const std::string& aName = "", const std::string& aCreator = "", const std::string& aTimestamp = "");

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSHeader(void);

    // Getter/Setter for member name
    TIGL_EXPORT std::string GetName(void) const;

    // Getter/Setter for member creator
    TIGL_EXPORT std::string GetCreator(void) const;

    // Getter/Setter for member timestamp
    TIGL_EXPORT std::string GetTimestamp(void) const;

    // Getter/Setter for member description
    TIGL_EXPORT void SetDescription(const std::string& aDescription);
    TIGL_EXPORT const std::string& GetDescription(void) const;

    // Read CPACS header elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Write (and Save) header element, or create it if doesn't exist yet
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle);

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSHeader(const CCPACSHeader& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CCPACSHeader& ) { /* Do nothing */ }

private:
    std::string name;       /**< Configuration name          */
    std::string description;/**< Configuration description   */
    std::string creator;    /**< Configuration author        */
    std::string timestamp;  /**< Configuration creation time */

};

} // end namespace tigl

#endif // CCPACSHEADER_H
