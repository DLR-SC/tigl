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

#include "CCPACSGuideCurveProfile.h"
#include "CTiglPoint.h"

namespace tigl
{
CCPACSGuideCurveProfile::CCPACSGuideCurveProfile(CCPACSGuideCurveProfiles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGuideCurveProfileGeometry(parent, uidMgr) {}

const std::vector<CTiglPoint>& CCPACSGuideCurveProfile::GetGuideCurveProfilePoints() const {
    return m_pointList.AsVector();
}
} // end namespace tigl
