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
* @brief  Implementation of CPACS wing segments handling routines.
*/

#include "CCPACSWingSegments.h"

#include "CTiglError.h"
#include "CCPACSWing.h"
#include "CCPACSEnginePylon.h"
#include "CCPACSWingSegment.h"

namespace tigl
{

// Constructor
CCPACSWingSegments::CCPACSWingSegments(CCPACSWing* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSegments(parent, uidMgr)
    , m_parentVariant(parent)
{
}

CCPACSWingSegments::CCPACSWingSegments(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSegments(parent, uidMgr)
    , m_parentVariant(parent)
{
}

// Invalidates internal state
void CCPACSWingSegments::Invalidate(const boost::optional<std::string>& source) const
{
    for (const auto& segment : m_segments) {
        segment->Invalidate(source);
    }
}

CCPACSWingSegment& CCPACSWingSegments::AddSegment()
{
    CCPACSWingSegment& result = generated::CPACSWingSegments::AddSegment();
    InvalidateParent();
    return result;
}

void CCPACSWingSegments::RemoveSegment(CCPACSWingSegment& ref)
{
    generated::CPACSWingSegments::RemoveSegment(ref);
    InvalidateParent();
}

void CCPACSWingSegments::InvalidateParent() const
{
    // Invalidate wing or EnginePylon
    if (const auto* parent = GetNextUIDParent()) {
        parent->Invalidate();
    }
}

} // end namespace tigl
