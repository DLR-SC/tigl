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

#include "CCPACSPointXY.h"
#include "CTiglUIDManager.h"

namespace tigl
{

CCPACSPointXY::CCPACSPointXY(CCPACSGlobalBeamProperties* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPointXY(parent, uidMgr)
{
}

CCPACSPointXY::CCPACSPointXY(CCPACSPointListXY* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPointXY(parent, uidMgr)
{
}

CCPACSPointXY::CCPACSPointXY(CCPACSSheet* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPointXY(parent, uidMgr)
{
}

CCPACSPointXY::CCPACSPointXY(CCPACSTransformation2D* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPointXY(parent, uidMgr)
{
}

gp_Pnt2d CCPACSPointXY::As_gp_Pnt() const
{
    return gp_Pnt2d(m_x, m_y);
}

void CCPACSPointXY::SetAs_gp_Pnt(const gp_Pnt2d& point)
{
    m_x = point.X();
    m_y = point.Y();
    Invalidate();
}

void CCPACSPointXY::SetX(const double& value)
{
    generated::CPACSPointXY::SetX(value);
    Invalidate();
}

void CCPACSPointXY::SetY(const double& value)
{
    generated::CPACSPointXY::SetY(value);
    Invalidate();
}

void CCPACSPointXY::InvalidateImpl(const boost::optional<std::string>& source) const
{
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate(GetUID());
    }
}

} // namespace tigl
