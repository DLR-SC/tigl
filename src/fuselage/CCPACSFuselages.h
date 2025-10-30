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
    TIGL_EXPORT CCPACSFuselages(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSFuselages(CCPACSRotorcraftModel* parent, CTiglUIDManager* uidMgr);

    // Read CPACS fuselage elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

    // Write CPACS fuselage elements
    TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

    // Create a new fuselage with the given parameters.
    // Remark, all children UIDs and transformations are set.
    TIGL_EXPORT CCPACSFuselage& CreateFuselage(const std::string& fuselageUID, int numberOfSection,
                                               const std::string& profileUID);
};

} // end namespace tigl

#endif // CCPACSFUSELAGES_H
