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
#include "CTiglLogger.h"

namespace tigl {

CCPACSWingCells::CCPACSWingCells(){
    Reset();
}

CCPACSWingCells::~CCPACSWingCells(){
    Cleanup();
}

void CCPACSWingCells::Reset(){
    Cleanup();
}

void CCPACSWingCells::Cleanup(){
    CCPACSWingCellContainer::iterator cellit;
    for(cellit = cells.begin(); cellit != cells.end(); ++cellit){
        delete (*cellit);
        *cellit = NULL;
    }
    cells.clear();
}

void CCPACSWingCells::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &cellsXPath){
    Cleanup();
    
    // check path
    if( tixiCheckElement(tixiHandle, cellsXPath.c_str()) != SUCCESS){
        LOG(ERROR) << "Wing Cells definition" << cellsXPath << " not found in CPACS file!" << std::endl;
        return;
    }
    
    int ncells = 0;
    if (tixiGetNamedChildrenCount(tixiHandle, cellsXPath.c_str(), "cell", &ncells) != SUCCESS){
        // no cells found
        return;
    }
    
    for(int icell = 1; icell <= ncells; ++icell){
        std::stringstream stream;
        stream << cellsXPath << "/" << "cell[" << icell << "]";
        
        // check path
        if( tixiCheckElement(tixiHandle, stream.str().c_str()) == SUCCESS){
            CCPACSWingCell * cell = new CCPACSWingCell();
            cell->ReadCPACS(tixiHandle, stream.str().c_str());
            cells.push_back(cell);
        }
    }
}



} // namespace tigl
