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
* @brief  Implementation of CPACS wing ComponentSegments handling routines.
*/

#include <iostream>
#include <sstream>
#include <exception>

#include "CCPACSWingComponentSegments.h"

namespace tigl
{

// Constructor
CCPACSWingComponentSegments::CCPACSWingComponentSegments(CCPACSWing* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSComponentSegments(parent, uidMgr) {}

// Invalidates internal state
void CCPACSWingComponentSegments::Invalidate()
{
    for (int i = 1; i <= GetComponentSegmentCount(); i++) {
        GetComponentSegment(i).Invalidate();
    }
}

// Gets a componentSegment by index.
CCPACSWingComponentSegment& CCPACSWingComponentSegments::GetComponentSegment(int index)
{
	index--;
    if (index < 0 || index >= GetComponentSegmentCount()) {
        throw CTiglError("Error: Invalid index value in CCPACSWingComponentSegments::GetComponentSegment", TIGL_INDEX_ERROR);
    }
    return *m_componentSegments[index];
}

const CCPACSWingComponentSegment& CCPACSWingComponentSegments::GetComponentSegment(int index) const
{
	index--;
    if (index < 0 || index >= GetComponentSegmentCount()) {
        throw CTiglError("Error: Invalid index value in CCPACSWingComponentSegments::GetComponentSegment", TIGL_INDEX_ERROR);
    }
    return *m_componentSegments[index];
}

// Gets a componentSegment by uid.
CCPACSWingComponentSegment & CCPACSWingComponentSegments::GetComponentSegment(const std::string& componentSegmentUID)
{
    for (std::size_t i = 0; i < m_componentSegments.size(); i++) {
        if (m_componentSegments[i]->GetUID() == componentSegmentUID) {
            return *m_componentSegments[i];
        }
    }
    throw CTiglError("Error: Invalid uid in CCPACSWingComponentSegments::GetComponentSegment", TIGL_UID_ERROR);
}

const CCPACSWingComponentSegment & CCPACSWingComponentSegments::GetComponentSegment(const std::string& componentSegmentUID) const
{
    for (std::size_t i = 0; i < m_componentSegments.size(); i++) {
        if (m_componentSegments[i]->GetUID() == componentSegmentUID) {
            return *m_componentSegments[i];
        }
    }
    throw CTiglError("Error: Invalid uid in CCPACSWingComponentSegments::GetComponentSegment", TIGL_UID_ERROR);
}

// Gets total componentSegment count
int CCPACSWingComponentSegments::GetComponentSegmentCount() const
{
    return static_cast<int>(m_componentSegments.size());
}
} // end namespace tigl
