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

#include "CTiglError.h"
#include "CTiglLogging.h"


// [[CAS_AES]] include helper routines for save method
#include "TixiSaveExt.h"

namespace tigl 
{

CCPACSWingShell::CCPACSWingShell() 
{
    Reset();
}

void CCPACSWingShell::Reset()
{
    cells.Reset();
    Invalidate();
}

int CCPACSWingShell::GetCellCount() const
{
    return cells.GetCellCount();
}

CCPACSWingCell& CCPACSWingShell::GetCell(int index)
{
    return cells.GetCell(index);
}

CCPACSMaterial& CCPACSWingShell::GetMaterial()
{
    return material;
}

void CCPACSWingShell::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &shellXPath)
{
    Reset();
    
    // check path
    if ( tixiCheckElement(tixiHandle, shellXPath.c_str()) != SUCCESS) {
        LOG(ERROR) << "Wing Shell " << shellXPath << " not found in CPACS file!" << std::endl;
        return;
    }
    
    // read cell data
    std::string cellpath = shellXPath + "/cells";
    if (tixiCheckElement(tixiHandle, cellpath.c_str()) == SUCCESS) {
        cells.ReadCPACS(tixiHandle, cellpath.c_str());
    }
    
    // read material
    std::string materialString;
    materialString = shellXPath + "/skin/material";
    if ( tixiCheckElement(tixiHandle, materialString.c_str()) == SUCCESS) {
        material.ReadCPACS(tixiHandle, materialString.c_str());
    }
    else {
        // @todo: should that be an error?
        LOG(WARNING) << "No material definition found for shell " << shellXPath;
    }
    
    isvalid = true;
}

// [[CAS_AES]] Write CPACS segment elements
void CCPACSWingShell::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& shellDefinitionXPath)
{
    std::string elementPath;
    
    TixiSaveExt::TixiSaveElement(tixiHandle, shellDefinitionXPath.c_str(), "skin");

    if (material.GetUID() != "UID_NOTSET") {
        elementPath = shellDefinitionXPath + "/skin";
        TixiSaveExt::TixiSaveElement(tixiHandle, elementPath.c_str(), "material");
        elementPath = shellDefinitionXPath + "/skin/material";
        material.WriteCPACS(tixiHandle, elementPath);
    }
    
    if (cells.GetCellCount() > 0) {
        TixiSaveExt::TixiSaveElement(tixiHandle, shellDefinitionXPath.c_str(), "cells");
        elementPath = shellDefinitionXPath + "/cells";
        cells.WriteCPACS(tixiHandle, elementPath);
    }
}

void CCPACSWingShell::Invalidate()
{
    isvalid = false;
}

bool CCPACSWingShell::IsValid() const
{
    return isvalid;
}

} // namespace tigl
