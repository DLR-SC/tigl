/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-10-31 Martin Siggel <martin.siggel@dlr.de>
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


#include "CCPACSEnginePylon.h"

#include "CTiglEnginePylonBuilder.h"

namespace tigl
{

CCPACSEnginePylon::CCPACSEnginePylon(CCPACSEnginePylons* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSEnginePylon(parent, uidMgr)
    , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation)
{
}

std::string CCPACSEnginePylon::GetDefaultedUID() const
{
    return generated::CPACSEnginePylon::GetUID();
}

void CCPACSEnginePylon::Invalidate()
{
}

PNamedShape CCPACSEnginePylon::BuildLoft() const
{
    CTiglEnginePylonBuilder builder(*this);

    return builder.BuildShape();
}

// Get the positioning transformation for a given section-uid
CTiglTransformation CCPACSEnginePylon::GetPositioningTransformation(std::string sectionUID)
{
    if (m_positionings)
        return m_positionings->GetPositioningTransformation(sectionUID);
    else
        return CTiglTransformation(); // return identity if no positioning transformation is given
}

bool CCPACSEnginePylon::HasLoft() const
{
    return m_segments.is_initialized() && m_segments->GetSegmentCount() > 0;
}

} // namespace tigl
