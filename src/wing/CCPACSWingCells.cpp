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

namespace tigl
{

CCPACSWingCells::CCPACSWingCells()
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

void CCPACSWingCells::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &cellsXPath)
{
    Cleanup();
    
    // check path
    if (tixiCheckElement(tixiHandle, cellsXPath.c_str()) != SUCCESS) {
        LOG(ERROR) << "Wing Cells definition" << cellsXPath << " not found in CPACS file!" << std::endl;
        return;
    }
    
    int ncells = 0;
    if (tixiGetNamedChildrenCount(tixiHandle, cellsXPath.c_str(), "cell", &ncells) != SUCCESS) {
        // no cells found
        return;
    }
    
    for (int icell = 1; icell <= ncells; ++icell) {
        std::stringstream stream;
        stream << cellsXPath << "/" << "cell[" << icell << "]";
        
        // check path
        if ( tixiCheckElement(tixiHandle, stream.str().c_str()) == SUCCESS) {
            CCPACSWingCell * cell = new CCPACSWingCell();
            cell->ReadCPACS(tixiHandle, stream.str().c_str());
            cells.push_back(cell);
        }
    }
}

int CCPACSWingCells::GetCellCount() const
{
    return cells.size();
}

CCPACSWingCell& CCPACSWingCells::GetCell(int index) const
{
    if (index < 1 || index > GetCellCount()) {
        throw CTiglError("Illegal index in CCPACSWingCells::GetCell", TIGL_INDEX_ERROR);
    }
    
    return *cells.at(index-1);
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
