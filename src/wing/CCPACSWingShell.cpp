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
#include "TixiSaveExt.h"
// [[CAS_AES]] added includes
// [[CAS_AES]] BEGIN
#include "CCPACSWingCell.h"
#include "CCPACSWing.h"

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
// [[CAS_AES]] END

namespace tigl 
{

// [[CAS_AES]] added reference to parent element
// [[CAS_AES]] added initialization of stringer and cell
CCPACSWingShell::CCPACSWingShell()
: cells(this)
{
    Reset();
}

// [[CAS_AES]] added destructor
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

    // [[CAS_AES]] Get UID
    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, shellXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        uid = ptrUID;
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
    
    // [[CAS_AES]] commented out validation flag since geometry is not generated yet
    //isvalid = true;
}

// Write CPACS segment elements
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
    cells.Invalidate();
}

bool CCPACSWingShell::IsValid() const
{
    return isvalid;
}

// [[CAS_AES]] added method
void CCPACSWingShell::Update()
{
    if (isvalid) {
        return;
    }

    // TODO: build stringer geometry

    isvalid = true;
}

} // namespace tigl
