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

namespace tigl
{

// [[CAS_AES]] added pointer to parent to constructor
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

void CCPACSWingCells::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& cellsXPath)
{
    std::string elementPath;
    std::string xpath;
    ReturnCode  tixiRet;
    int         cellCount, test;

    elementPath = cellsXPath;
//     TixiSaveExt::TixiSaveElement(tixiHandle, cellsXPath.c_str(), "cells");
    
    tixiRet = tixiGetNamedChildrenCount(tixiHandle, elementPath.c_str(), "cell", &test);
    cellCount = GetCellCount();

    for (int i = 1; i <= cellCount; i++) {
        std::stringstream ss;
        ss << elementPath << "/cell[" << i << "]";
        xpath = ss.str();
        CCPACSWingCell& cell = GetCell(i);
        if (tixiCheckElement(tixiHandle, xpath.c_str()) == ELEMENT_NOT_FOUND) {
            if (tixiCreateElement(tixiHandle, elementPath.c_str(), "cell") != SUCCESS) {
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSWingCells::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        cell.WriteCPACS(tixiHandle, xpath);
    }

    for (int i = cellCount + 1; i <= test; i++) {
        std::stringstream ss;
        ss << elementPath << "/cell[" << cellCount + 1 << "]";
        xpath = ss.str();
        tixiRemoveElement(tixiHandle, xpath.c_str());
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

// [[CAS_AES]] get parent wing shell element
CCPACSWingShell* CCPACSWingCells::GetParentElement()
{
    return parentShell;
}


} // namespace tigl
