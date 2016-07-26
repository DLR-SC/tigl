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
* @brief  Implementation of CPACS fuselages handling routines.
*/

#ifndef CCPACSFUSELAGES_H
#define CCPACSFUSELAGES_H

#include "generated/CPACSFuselages.h"
#include "tigl_internal.h"

#include "CCPACSFuselage.h"
#include "CCPACSFuselageProfile.h"
#include "CCPACSFuselageProfiles.h"


namespace tigl
{
class CCPACSFuselages : public generated::CPACSFuselages
{
public:
    //TIGL_EXPORT CCPACSFuselages();
    TIGL_EXPORT CCPACSFuselages(CCPACSModel* parent);
    TIGL_EXPORT CCPACSFuselages(generated::CPACSRotorcraftModel* parent);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS fuselage elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath);

    // Write CPACS fuselage elements
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const;

    DEPRECATED TIGL_EXPORT bool HasProfile(std::string uid) const;

    // Returns the total count of fuselage profiles in this configuration
	DEPRECATED TIGL_EXPORT int GetProfileCount() const;
        
    // Returns the wing profiles in this configuration
	DEPRECATED TIGL_EXPORT CCPACSFuselageProfiles& GetProfiles();
	DEPRECATED TIGL_EXPORT const CCPACSFuselageProfiles& GetProfiles() const;

    // Returns the fuselage profile for a given index.
	DEPRECATED TIGL_EXPORT CCPACSFuselageProfile& GetProfile(int index) const;
	DEPRECATED TIGL_EXPORT CCPACSFuselageProfile& GetProfile(std::string uid) const;

    // Returns the total count of fuselages in a configuration
    TIGL_EXPORT int GetFuselageCount() const;

    // Returns the fuselage for a given index.
    TIGL_EXPORT CCPACSFuselage& GetFuselage(int index) const;

    // Returns the fuselage for a given UID.
    TIGL_EXPORT CCPACSFuselage& GetFuselage(const std::string& UID) const;

    // Adds a new fuselage to the list of fuselages
    TIGL_EXPORT void AddFuselage(CCPACSFuselage* fuselage);
};

} // end namespace tigl

#endif // CCPACSFUSELAGES_H
