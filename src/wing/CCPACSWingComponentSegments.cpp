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
void CCPACSWingComponentSegments::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 1; i <= GetComponentSegmentCount(); i++) {
        GetComponentSegment(i).Invalidate(source);
    }
}

} // end namespace tigl
