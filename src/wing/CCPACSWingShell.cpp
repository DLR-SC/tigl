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
#include "CTiglLogging.h"
#include "TixiSaveExt.h"
#include "CCPACSWingCell.h"


namespace tigl 
{

CCPACSWingShell::CCPACSWingShell(CCPACSWingCSStructure* parent)
    : generated::CPACSWingShell(parent)
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

const CCPACSMaterial& CCPACSWingShell::GetMaterial() const
{
    return m_skin.GetMaterial();
}

CCPACSMaterial& CCPACSWingShell::GetMaterial()
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

void CCPACSWingShell::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &shellXPath)
{
    generated::CPACSWingShell::ReadCPACS(tixiHandle, shellXPath);
}

void CCPACSWingShell::Invalidate()
{
    geometryCache.valid = false;
}

bool CCPACSWingShell::IsValid() const
{
    return geometryCache.valid;
}

void CCPACSWingShell::Update() const
{
    if ( geometryCache.valid) {
        return;
    }

    // TODO: build stringer geometry

    geometryCache.valid = true;
}

TiglLoftSide CCPACSWingShell::GetLoftSide() const
{
    if (&GetParent()->GetLowerShell() == this)
        return TiglLoftSide::LOWER_SIDE;
    if (&GetParent()->GetUpperShell() == this)
        return TiglLoftSide::UPPER_SIDE;
    throw std::logic_error("Cannot determine loft side, this shell is neither lower nor upper shell of parent");
}

} // namespace tigl
