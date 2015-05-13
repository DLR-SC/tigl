/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file
* @brief  Export routines for CPACS configurations.
*/

#include "CTiglExportStl.h"
#include "CCPACSConfiguration.h"

#include "TopoDS_Shape.hxx"
#include "Standard_CString.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRep_Builder.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "StlAPI_Writer.hxx"
#include "Interface_Static.hxx"
#include "StlAPI.hxx"
#include "CTiglFusePlane.h"

#include <cassert>

namespace tigl 
{

// Constructor
CTiglExportStl::CTiglExportStl(CCPACSConfiguration& config)
:myConfig(config)
{
}


// Exports a selected wing, boolean fused and meshed, as STL file
void CTiglExportStl::ExportMeshedWingSTL(int wingIndex, const std::string& filename, double deflection)
{
    CCPACSWing& wing = myConfig.GetWing(wingIndex);
    PNamedShape loft = wing.GetLoft();

    BRepMesh_IncrementalMesh(loft->Shape(), deflection);
    StlAPI_Writer *StlWriter = new StlAPI_Writer();
    StlWriter->Write(loft->Shape(), const_cast<char*>(filename.c_str()));
}


// Exports a selected fuselage, boolean fused and meshed, as STL file
void CTiglExportStl::ExportMeshedFuselageSTL(int fuselageIndex, const std::string& filename, double deflection)
{
    CCPACSFuselage& fuselage = myConfig.GetFuselage(fuselageIndex);
    PNamedShape loft = fuselage.GetLoft();

    BRepMesh_IncrementalMesh(loft->Shape(), deflection);
    StlAPI_Writer *StlWriter = new StlAPI_Writer();
    StlWriter->Write(loft->Shape(), const_cast<char*>(filename.c_str()));
}

// Exports a whole geometry, boolean fused and meshed, as STL file
void CTiglExportStl::ExportMeshedGeometrySTL(const std::string& filename, double deflection)
{
    PTiglFusePlane fuser = myConfig.AircraftFusingAlgo();
    assert(fuser);
    fuser->SetResultMode(FULL_PLANE);

    // get/compute shape
    PNamedShape ac = fuser->FusedPlane();
    if (!ac) {
        throw CTiglError("Error computing fused geometry in CTiglExportStl::ExportMeshedGeometrySTL", TIGL_ERROR);
    }

    TopoDS_Shape loft = ac->Shape();

    BRepMesh_IncrementalMesh(loft, deflection);
    StlAPI_Writer *StlWriter = new StlAPI_Writer();
    StlWriter->Write(loft, const_cast<char*>(filename.c_str()));
}

} // end namespace tigl
