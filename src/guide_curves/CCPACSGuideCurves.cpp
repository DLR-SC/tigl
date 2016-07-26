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
#include <algorithm>

namespace tigl
{
void CCPACSGuideCurves::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) {
    generated::CPACSGuideCurves::ReadCPACS(tixiHandle, xpath);

    // sort by uid as some unit tests rely on this (TODO: should we fix the tests?)
    // WARN: this destroys the order of the guide curves as stored in the CPACS file
    std::sort(std::begin(m_guideCurve), std::end(m_guideCurve), [](const CCPACSGuideCurve* a, const CCPACSGuideCurve* b) {
        return a->GetUID() < b->GetUID();
    });
}

// Returns the total count of guide curves in this configuration
int CCPACSGuideCurves::GetGuideCurveCount() const
{
    return static_cast<int>(m_guideCurve.size());
}

// Returns the guide curve for a given index
const CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(int index) const
{
    index--;
    if (index < 0 || index >= GetGuideCurveCount()) {
        throw CTiglError("Error: Invalid index in CCPACSGuideCurves::GetGuideCurve", TIGL_INDEX_ERROR);
    }
    return *m_guideCurve[index];
}

// Returns the guide curve for a given uid.
const CCPACSGuideCurve& CCPACSGuideCurves::GetGuideCurve(std::string uid) const
{
    for (std::size_t i = 0; i < m_guideCurve.size(); i++) {
        if (m_guideCurve[i]->GetUID() == uid) {
            return *m_guideCurve[i];
        }
    }
    throw CTiglError("CCPACSGuideCurve::GetGuideCurve: Guide curve \"" + uid + "\" not found in CPACS file!", TIGL_UID_ERROR);
}

// Returns the guide curve for a given uid.
bool CCPACSGuideCurves::GuideCurveExists(std::string uid) const
{
    for (std::size_t i = 0; i < m_guideCurve.size(); i++) {
        if (m_guideCurve[i]->GetUID() == uid) {
            return true;
        }
    }
    return false;
}

} // end namespace tigl


