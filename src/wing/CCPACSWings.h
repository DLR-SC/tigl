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
* @brief  Implementation of CPACS wings handling routines.
*/

#ifndef CCPACSWINGS_H
#define CCPACSWINGS_H

#include <string>
#include <boost/optional.hpp>

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
    TIGL_EXPORT CCPACSWings(CCPACSRotorcraftModel* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSWings(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr);

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(const boost::optional<std::string>& source = boost::none) const;

    TIGL_EXPORT bool HasWing(const std::string& uid) const;

    /**
     * Create a new wing with sections and segments.
     *
     * @param fuselageUID
     * @param numberOfSection
     * @param profileUID
     * @return
     */
    TIGL_EXPORT CCPACSWing& CreateWing(const std::string& wingUID, int numberOfSection, const std::string& airfoilUID);
};

} // end namespace tigl

#endif // CCPACSWINGS_H
