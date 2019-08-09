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

#ifndef CCPACSWINGPROFILES_H
#define CCPACSWINGPROFILES_H

#include "generated/CPACSWingAirfoils.h"
#include "tigl_internal.h"
#include "CCPACSWingProfile.h"
#include <string>
#include <map>

namespace tigl
{

class CCPACSWingProfiles : public generated::CPACSWingAirfoils
{
public:
    TIGL_EXPORT CCPACSWingProfiles(CTiglUIDManager* uidMgr);

    // Read CPACS wing profiles
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) OVERRIDE;

    // importing profiles from CPACS
    // profiles with same UID are overwritten
    TIGL_EXPORT void ImportCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

    TIGL_EXPORT CCPACSWingProfile& AddWingAirfoil() OVERRIDE;

    TIGL_EXPORT bool HasProfile(std::string uid) const;

    // Return the number of profile
    TIGL_EXPORT int GetProfileCount() const;

    // Returns the wing profile for a given uid.
    TIGL_EXPORT CCPACSWingProfile& GetProfile(std::string uid) const;

    // Returns the wing profile for a given index.
    TIGL_EXPORT CCPACSWingProfile& GetProfile(int index) const;


    // Invalidates internal state
    TIGL_EXPORT void Invalidate();
};

} // end namespace tigl

#endif // CCPACSWINGPROFILES_H
