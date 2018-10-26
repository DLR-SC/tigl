/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSWingShell.h"

#include "CCPACSWingCSStructure.h"
#include "CTiglError.h"
#include "CCPACSWingCell.h"
#include "tiglcommonfunctions.h"


namespace tigl
{

CCPACSWingShell::CCPACSWingShell(CCPACSWingCSStructure* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingShell(parent, uidMgr)
    , m_geometryCache(*this, &CCPACSWingShell::BuildGeometry)
{
}

int CCPACSWingShell::GetCellCount() const
{
    if (m_cells)
        return m_cells->GetCellCount();
    else
        return 0;
}

const CCPACSWingCell& CCPACSWingShell::GetCell(int index) const
{
    return m_cells->GetCell(index);
}

CCPACSWingCell& CCPACSWingShell::GetCell(int index)
{
    // forward call to const method
    return const_cast<CCPACSWingCell&>(static_cast<const CCPACSWingShell&>(*this).GetCell(index));
}

const CCPACSMaterialDefinition& CCPACSWingShell::GetMaterial() const
{
    return m_skin.GetMaterial();
}

CCPACSMaterialDefinition& CCPACSWingShell::GetMaterial()
{
    return m_skin.GetMaterial();
}

const CCPACSWingCSStructure& CCPACSWingShell::GetStructure() const
{
    return *m_parent;
}

CCPACSWingCSStructure& CCPACSWingShell::GetStructure()
{
    return *m_parent;
}

void CCPACSWingShell::Invalidate()
{
    m_geometryCache.clear();
    if (m_cells)
        m_cells->Invalidate();
}

void CCPACSWingShell::BuildGeometry(GeometryCache& cache) const
{

}

TiglLoftSide CCPACSWingShell::GetLoftSide() const
{
    if (&GetParent()->GetLowerShell() == this)
        return LOWER_SIDE;
    if (&GetParent()->GetUpperShell() == this)
        return UPPER_SIDE;
    throw CTiglError("Cannot determine loft side, this shell is neither lower nor upper shell of parent");
}

} // namespace tigl
