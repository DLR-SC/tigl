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
* @brief  Implementation of CPACS guide curve container handling routines
*/

#include "CCPACSGuideCurves.h"
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
CCPACSGuideCurves::CCPACSGuideCurves(void)
{
    Cleanup();
}

// Destructor
CCPACSGuideCurves::~CCPACSGuideCurves(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSGuideCurves::Cleanup(void)
{
    guideCurves.clear();
}

// Read CPACS guide curves
void CCPACSGuideCurves::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath)
{
    Cleanup();

    std::vector<CCPACSGuideCurve*> curves;
    ReadContainerElement(tixiHandle, segmentXPath + "/guideCurves", "guideCurve", 1, curves);
    for (size_t i = 0; i < curves.size(); i++) {
        guideCurves[curves[i]->GetUID()] = PCCPACSGuideCurve(curves[i]);
    }
}

// Returns the total count of guide curves in this configuration
int CCPACSGuideCurves::GetGuideCurveCount(void) const
{
    return (static_cast<int>(guideCurves.size()));
}

// Returns the guide curve for a given index
CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(int index) const
{
    unsigned int arrayIndex = index - 1;
    if (arrayIndex < guideCurves.size()) {
        CCPACSGuideCurveContainer::const_iterator p = guideCurves.begin();
        std::advance(p, arrayIndex);
        return (*p->second);
    }
    else {
        std::stringstream msg;
        msg << "Invalid guide curve \"" << index << "\" in CCPACSGuideCurve::GetGuideCurve!";
        throw CTiglError(msg.str(), TIGL_INDEX_ERROR);
    }
}

// Returns the guide curve for a given uid.
CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(std::string uid) const
{
    CCPACSGuideCurveContainer::const_iterator it = guideCurves.find(uid);
    if (it != guideCurves.end() && it->second) {
        return *(it->second);
    }
    else {
        throw CTiglError("CCPACSGuideCurve::GetGuideCurve: Guide curve \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
    }
}

// Returns the guide curve for a given uid.
bool CCPACSGuideCurves::GuideCurveExists(std::string uid) const
{
    CCPACSGuideCurveContainer::const_iterator it = guideCurves.find(uid);
    if (it != guideCurves.end() && it->second) {
        return true;
    }
    else {
        return false;
    }
}

} // end namespace tigl


