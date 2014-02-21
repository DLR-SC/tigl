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

    std::string guideCurvesXPath = segmentXPath + "/guideCurves";
    if (tixiCheckElement(tixiHandle, guideCurvesXPath.c_str()) == SUCCESS) {
        // Get element count
        int elementCount;
        if (tixiGetNamedChildrenCount(tixiHandle, guideCurvesXPath.c_str(), "guideCurve", &elementCount) != SUCCESS) {
            throw CTiglError("Error: tixiGetNamedChildrenCount failed in CCPACSGuideCurves::ReadCPACS", TIGL_XML_ERROR);
        }

        // Loop over all <guideCurve> elements
        for (int i = 1; i <= elementCount; i++) {
            std::ostringstream xpath;
            xpath << guideCurvesXPath << "/guideCurve[" << i << "]";

            PCCPACSGuideCurve guideCurve(new CCPACSGuideCurve(xpath.str()));
            guideCurve->ReadCPACS(tixiHandle);
            guideCurves[guideCurve->GetUID()] = guideCurve;
        }
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
    int i = 0;
    CCPACSGuideCurveContainer::const_iterator p;
    if (index-1>=0 && index-1<guideCurves.size()) {
        for (p = guideCurves.begin(); p!=guideCurves.end(); ++p) {
            if (i == index-1) {
                break;
            }
            i++;
        }
        return (*p->second);
    }
    else {
        char index_ch[10];
        sprintf(index_ch, "%i", index);
        std::string index_str=index_ch;
        throw CTiglError("Error in CCPACSGuideCurve::GetGuideCurve: Guide curve with index \"" + index_str + "\" not found in CPACS file!", TIGL_INDEX_ERROR);
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
        throw CTiglError("Error in CCPACSGuideCurve::GetGuideCurve: Guide curve \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
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


