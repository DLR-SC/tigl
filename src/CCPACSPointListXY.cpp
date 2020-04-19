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

#include "CCPACSPointListXY.h"

#include "CTiglLogging.h"
#include "CCPACSPointXY.h"

namespace tigl
{

CCPACSPointListXY::CCPACSPointListXY(CCPACSStructuralProfile* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPointListXY(parent, uidMgr)
{
}

const CCPACSPointXY& CCPACSPointListXY::GetPoint(const std::string& uid) const
{
    for (int i = 0; i < m_points.size(); i++) {
        if (m_points[i]->GetUID() == uid) {
            return *m_points[i];
        }
    }

    LOG(ERROR) << "Invalid structural profile point uid: '" << uid << "'";
    throw CTiglError("Invalid uid in CCPACSStructuralProfilePoints::GetPoint");
}

CCPACSPointXY& CCPACSPointListXY::AddPoint()
{
    CCPACSPointXY& p = generated::CPACSPointListXY::AddPoint();
    InvalidateParent();
    return p;
}

void CCPACSPointListXY::RemovePoint(CCPACSPointXY& ref)
{
    generated::CPACSPointListXY::RemovePoint(ref);
    InvalidateParent();
}

void CCPACSPointListXY::InvalidateParent() const
{
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate();
    }
}


} // namespace tigl
