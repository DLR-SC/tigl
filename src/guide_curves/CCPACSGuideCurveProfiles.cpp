/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS guide curve profiles handling routines
*/

#include "CCPACSGuideCurveProfiles.h"
#include "CTiglError.h"
#include "CTiglPoint.h"
#include "CTiglLogging.h"
#include "IOHelper.h"
#include <sstream>
#include <iostream>
#include <fstream>

namespace tigl
{

// Constructor
CCPACSGuideCurveProfiles::CCPACSGuideCurveProfiles(void)
{
    Cleanup();
}

// Destructor
CCPACSGuideCurveProfiles::~CCPACSGuideCurveProfiles(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSGuideCurveProfiles::Cleanup(void)
{
    guideCurves.clear();
}

// Read CPACS guide curves
void CCPACSGuideCurveProfiles::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    Cleanup();
    const std::string xpath = "/cpacs/vehicles/profiles/guideCurveProfiles";
    std::vector<CCPACSGuideCurveProfile*> children;
    if (tixiCheckElement(tixiHandle, xpath.c_str()) == SUCCESS) {
        ReadContainerElement(tixiHandle, xpath, "guideCurveProfile", 1, children);
        for (std::size_t i = 0; i < children.size(); i++) {
            guideCurves[children[i]->GetUID()].reset(children[i]);
        }
    }
}

// Returns the total count of guide curves in this configuration
int CCPACSGuideCurveProfiles::GetGuideCurveProfileCount(void) const
{
    return (static_cast<int>(guideCurves.size()));
}

// Returns the guide curve for a given uid.
CCPACSGuideCurveProfile& CCPACSGuideCurveProfiles::GetGuideCurveProfile(std::string uid) const
{
    CCPACSGuideCurveProfileContainer::const_iterator it = guideCurves.find(uid);
    if (it != guideCurves.end() && it->second) {
        return *(it->second);
    }
    else {
        LOG(ERROR) << "Guide curve \"" + uid + "\" not found in CPACS file!" << endl;
        throw CTiglError("Guide curve \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
    }
}

} // end namespace tigl


