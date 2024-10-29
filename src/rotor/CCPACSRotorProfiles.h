/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Implementation of CPACS wing profiles handling routines.
*/

#ifndef CCPACSROTORPROFILES_H
#define CCPACSROTORPROFILES_H

#include <string>
#include <boost/optional.hpp>

#include "generated/CPACSRotorAirfoils.h"
#include "tigl_internal.h"

namespace tigl
{
class CCPACSWingProfile;

class CCPACSRotorProfiles : public generated::CPACSRotorAirfoils
{
public:
    TIGL_EXPORT CCPACSRotorProfiles(CCPACSProfiles* parent, CTiglUIDManager* uidMgr);

    // Read CPACS wing profiles
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    // importing profiles from CPACS
    // profiles with same UID are overwritten
    TIGL_EXPORT void ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

    // add a CPACS wing profile to list
    TIGL_EXPORT void AddProfile(CCPACSWingProfile* profile);
        
    // removes a CPACS wing profile from the list
    TIGL_EXPORT void DeleteProfile( std::string uid );

    TIGL_EXPORT bool HasProfile(std::string uid) const;

    // Returns the wing profile for a given index or uid.
    TIGL_EXPORT CCPACSWingProfile& GetProfile(std::string uid) const;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(const boost::optional<std::string>& source = boost::none) const;
};

} // end namespace tigl

#endif // CCPACSROTORPROFILES_H
