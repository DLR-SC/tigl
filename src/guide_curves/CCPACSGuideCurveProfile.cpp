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
* @brief  Implementation of a CPACS guide curve profile
*/

#include "tigl.h"
#include "CCPACSGuideCurveProfile.h"
#include "CTiglError.h"
#include "CTiglPoint.h"
#include "CTiglLogging.h"

namespace tigl
{
CCPACSGuideCurveProfile::CCPACSGuideCurveProfile() {}

// Constructor
CCPACSGuideCurveProfile::CCPACSGuideCurveProfile()
{
    Cleanup();
}

// Destructor
void CCPACSGuideCurveProfile::ReadCPACS(const TixiDocumentHandle& tixiHandle) {
    ReadCPACS(tixiHandle, xpath);
}

// Read guide curve file
void CCPACSGuideCurveProfile::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    std::string elementPath     = GuideCurveProfileXPath + "/pointList";
    generated::CPACSGuideCurveProfileGeometry::ReadCPACS(tixiHandle, xpath);

    // convert to point vector
    coordinates = m_pointList.AsVector();
}

void CCPACSGuideCurveProfile::WriteCPACS(const TixiDocumentHandle& tixiHandle) const {
    this->xpath = xpath;
    generated::CPACSGuideCurveProfileGeometry::WriteCPACS(tixiHandle, xpath);
}

void CCPACSGuideCurveProfile::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const {
    generated::CPACSGuideCurveProfileGeometry::WriteCPACS(tixiHandle, xpath);
}

const std::vector<CTiglPoint>& CCPACSGuideCurveProfile::GetGuideCurveProfilePoints() {
    return coordinates;
}

DEPRECATED TIGL_EXPORT const std::string& CCPACSGuideCurveProfile::GetFileName() const {
    return xpath;
}
} // end namespace tigl



