/*
* Copyright (c) 2018 RISC Software GmbH
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

#include "CCPACSSkinSegment.h"

#include "CCPACSFuselageStructure.h"
#include "CTiglUIDManager.h"

namespace tigl
{
CCPACSSkinSegment::CCPACSSkinSegment(CCPACSSkinSegments* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSSkinSegment(parent, uidMgr)
    , CTiglStringerFrameBorderedObject(*uidMgr, *parent->GetParent()->GetParent()->GetParent(), generated::CPACSSkinSegment::m_startFrameUID, generated::CPACSSkinSegment::m_endFrameUID, generated::CPACSSkinSegment::m_startStringerUID, generated::CPACSSkinSegment::m_endStringerUID)
{
}

void CCPACSSkinSegment::SetStartFrameUID(const std::string& value)
{
    generated::CPACSSkinSegment::SetStartFrameUID(value);
    Invalidate();
}

void CCPACSSkinSegment::SetEndFrameUID(const std::string& value)
{
    generated::CPACSSkinSegment::SetEndFrameUID(value);
    Invalidate();
}

void CCPACSSkinSegment::SetStartStringerUID(const std::string& value)
{
    generated::CPACSSkinSegment::SetStartStringerUID(value);
    Invalidate();
}

void CCPACSSkinSegment::SetEndStringerUID(const boost::optional<std::string>& value)
{
    generated::CPACSSkinSegment::SetEndStringerUID(value);
    Invalidate();
}


void CCPACSSkinSegment::InvalidateImpl(const boost::optional<std::string>& source) const
{
    CTiglStringerFrameBorderedObject::InvalidateShapes(source);
}

} // namespace tigl
