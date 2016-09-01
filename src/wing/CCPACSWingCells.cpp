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

#include "CCPACSWingCells.h"

#include <sstream>

#include "CCPACSWingCell.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"
#include "IOHelper.h"

namespace tigl
{

CCPACSWingCells::CCPACSWingCells(CCPACSWingShell* parent)
: parentShell(parent)
{
    Reset();
}

CCPACSWingCells::~CCPACSWingCells()
{
    Cleanup();
}

void CCPACSWingCells::Reset()
{
    Cleanup();
}

void CCPACSWingCells::Cleanup()
{
    CCPACSWingCellContainer::iterator cellit;
    for (cellit = cells.begin(); cellit != cells.end(); ++cellit) {
        delete (*cellit);
        *cellit = NULL;
    }
    cells.clear();
}

void CCPACSWingCells::Invalidate()
{

}

void CCPACSWingCells::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &xpath)
{
    Cleanup();
    ReadContainerElement(tixiHandle, xpath, "cell", 1, cells, this);
}

void CCPACSWingCells::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    WriteContainerElement(tixiHandle, xpath, "cell", cells);
}

int CCPACSWingCells::GetCellCount() const
{
    return static_cast<int>(cells.size());
}

CCPACSWingCell& CCPACSWingCells::GetCell(int index) const
{
    if (index < 1 || index > GetCellCount()) {
        throw CTiglError("Illegal index in CCPACSWingCells::GetCell", TIGL_INDEX_ERROR);
    }
    
    return *cells.at(index-1);
}

// Get parent wing shell element
CCPACSWingShell* CCPACSWingCells::GetParentElement() const
{
    return parentShell;
}

CCPACSWingCell &CCPACSWingCells::GetCell(const std::string &UID) const
{
    for (int i=0; i < GetCellCount(); i++) {
        const std::string tmpUID(cells[i]->GetUID());
        if (tmpUID == UID) {
            return (*cells[i]);
        }
    }

    // UID not there
    throw CTiglError("Error: Invalid UID in CCPACSWingCells::GetCell", TIGL_UID_ERROR);
}


} // namespace tigl
