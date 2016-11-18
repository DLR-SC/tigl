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

CCPACSWingShell::CCPACSWingShell(CCPACSWingCSStructure& parent, TiglLoftSide side)
    : parent(parent),
    side(side),
    cells(this)
{
    Reset();
}

CCPACSWingShell::~CCPACSWingShell()
{
    Reset();
}

void CCPACSWingShell::Reset()
{
    cells.Reset();
    Invalidate();
}

const std::string& CCPACSWingShell::GetUID() const
{
    return uid;
}

int CCPACSWingShell::GetCellCount() const
{
    return cells.GetCellCount();
}

const CCPACSWingCell& CCPACSWingShell::GetCell(int index) const
{
    return cells.GetCell(index);
}

CCPACSWingCell& CCPACSWingShell::GetCell(int index)
{
    // forward call to const method
    return const_cast<CCPACSWingCell&>(static_cast<const CCPACSWingShell&>(*this).GetCell(index));
}

const CCPACSMaterial& CCPACSWingShell::GetMaterial() const
{
    return material;
}

CCPACSMaterial& CCPACSWingShell::GetMaterial()
{
    return material;
}

const CCPACSWingCSStructure& CCPACSWingShell::GetStructure() const
{
    return parent;
}

CCPACSWingCSStructure& CCPACSWingShell::GetStructure()
{
    return parent;
}

void CCPACSWingShell::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &shellXPath)
{
    Reset();
    
    // check path
    if ( tixiCheckElement(tixiHandle, shellXPath.c_str()) != SUCCESS) {
        LOG(ERROR) << "Wing Shell " << shellXPath << " not found in CPACS file!" << std::endl;
        return;
    }

    // Get UID
    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, shellXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        uid = ptrUID;
    }

    // read cell data
    const std::string cellpath = shellXPath + "/cells";
    if (tixiCheckElement(tixiHandle, cellpath.c_str()) == SUCCESS) {
        cells.ReadCPACS(tixiHandle, cellpath.c_str());
    }
    
    // read material
    const std::string materialString = shellXPath + "/skin/material";
    if ( tixiCheckElement(tixiHandle, materialString.c_str()) == SUCCESS) {
        material.ReadCPACS(tixiHandle, materialString.c_str());
    }
    else {
        // @todo: should that be an error?
        LOG(WARNING) << "No material definition found for shell " << shellXPath;
    }
}

// Write CPACS segment elements
void CCPACSWingShell::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& shellDefinitionXPath) const
{
    TixiSaveExt::TixiSaveElement(tixiHandle, shellDefinitionXPath.c_str(), "skin", 1);

    if (material.GetUID() != "UID_NOTSET") {
        TixiSaveExt::TixiSaveElement(tixiHandle, (shellDefinitionXPath + "/skin").c_str(), "material");
        material.WriteCPACS(tixiHandle, shellDefinitionXPath + "/skin/material");
    }

    if (cells.GetCellCount() > 0) {
        TixiSaveExt::TixiSaveElement(tixiHandle, shellDefinitionXPath.c_str(), "cells");
        cells.WriteCPACS(tixiHandle, shellDefinitionXPath + "/cells");
    }
}

void CCPACSWingShell::Invalidate()
{
    geometryCache.valid = false;
    cells.Invalidate();
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
    return side;
}

} // namespace tigl
