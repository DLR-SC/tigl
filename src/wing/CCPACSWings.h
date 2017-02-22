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
* @brief  Implementation of CPACS wings handling routines.
*/

#ifndef CCPACSWINGS_H
#define CCPACSWINGS_H

#include "generated/CPACSWings.h"
#include "generated/CPACSWingAirfoils.h"
#include "tigl_internal.h"
#include "CCPACSWing.h"
#include "CCPACSWingProfile.h"
#include "CCPACSWingProfiles.h"

namespace tigl
{
class CCPACSWings : public generated::CPACSWings
{
public:
    TIGL_EXPORT CCPACSWings(CCPACSRotorcraftModel* parent);
    TIGL_EXPORT CCPACSWings(CCPACSAircraftModel* parent);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    DEPRECATED TIGL_EXPORT bool HasProfile(std::string uid) const;

    // Returns the wing profiles in this configuration
    DEPRECATED TIGL_EXPORT CCPACSWingProfiles& GetProfiles();
    DEPRECATED TIGL_EXPORT const CCPACSWingProfiles& GetProfiles() const;

    // Returns the total count of wing profiles in this configuration
    DEPRECATED TIGL_EXPORT int GetProfileCount() const;

    // Returns the wing profile for a given uid.
    DEPRECATED TIGL_EXPORT CCPACSWingProfile& GetProfile(std::string uid) const;

    // Returns the wing profile for a given index - TODO: depricated function!
    DEPRECATED TIGL_EXPORT CCPACSWingProfile& GetProfile(int index) const;

    // Returns the total count of wings in a configuration
    TIGL_EXPORT int GetWingCount() const;

    // Returns the count of wings in a configuration with the property isRotorBlade set to true
    TIGL_EXPORT int GetRotorBladeCount(void) const;

    // Returns the wing for a given index.
    TIGL_EXPORT CCPACSWing& GetWing(int index) const;

    // Returns the wing for a given UID.
    TIGL_EXPORT CCPACSWing& GetWing(const std::string& UID) const;
    
    // Adds a wing to the wing list
    TIGL_EXPORT void AddWing(CCPACSWing* wing);

    // Returns the wing index for a given UID.
    TIGL_EXPORT int GetWingIndex(const std::string& UID) const;

};

} // end namespace tigl

#endif // CCPACSWINGS_H
