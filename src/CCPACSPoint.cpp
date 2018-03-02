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


#include "CCPACSPoint.h"

namespace tigl
{
CCPACSPoint::CCPACSPoint(CTiglUIDManager* uidMgr)
    : generated::CPACSPoint(uidMgr)
{
}

CTiglPoint CCPACSPoint::AsPoint() const
{
    CTiglPoint point;
    point.x = m_x ? *m_x : 0;
    point.y = m_y ? *m_y : 0;
    point.z = m_z ? *m_z : 0;
    return point;
}

void CCPACSPoint::SetAsPoint(const CTiglPoint& point)
{
    m_x = point.x;
    m_y = point.y;
    m_z = point.z;
}

} // namespace tigl
