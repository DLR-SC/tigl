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
#include "BRepTools.hxx"
#include "BRepMesh.hxx"
#include "StlAPI_Writer.hxx"
#include "Interface_Static.hxx"
#include "StlAPI.hxx"
#include "CTiglFusePlane.h"

#include <cassert>

namespace tigl 
{

// Constructor
CTiglExportStl::CTiglExportStl()
{
}

void CTiglExportStl::AddShape(PNamedShape shape, double deflection)
{
    if (!shape) {
        return;
    }
    
    BRepMesh::Mesh(shape->Shape(), deflection);
    _shapes.push_back(shape);
}

void CTiglExportStl::AddConfiguration(CCPACSConfiguration &config, double deflection)
{
    PTiglFusePlane fuser = config.AircraftFusingAlgo();
    assert(fuser);
    fuser->SetResultMode(FULL_PLANE);

    // get/compute shape
    PNamedShape ac = fuser->FusedPlane();
    if (!ac) {
        throw CTiglError("Error computing fused geometry in CTiglExportStl::addConfiguration", TIGL_ERROR);
    }

    AddShape(ac, deflection);
}


TiglReturnCode CTiglExportStl::Write(const std::string& filename)
{
    if (_shapes.size() > 1) {
        TopoDS_Compound c;
        BRep_Builder b;
        b.MakeCompound(c);
        
        for (ListPNamedShape::const_iterator it = _shapes.begin(); it != _shapes.end(); ++it) {
            PNamedShape shape = *it;
            if (shape) {
                b.Add(c, shape->Shape());
            }
        }
        
        // write the file
        StlAPI_Writer StlWriter;
        StlWriter.Write(c, const_cast<char*>(filename.c_str()));
        
        return TIGL_SUCCESS;
    }
    else if ( _shapes.size() == 1) {
        PNamedShape shape = _shapes[0];
        StlAPI_Writer StlWriter;
        StlWriter.Write(shape->Shape(), const_cast<char*>(filename.c_str()));
        
        return TIGL_SUCCESS;
    }
    else {
        return TIGL_ERROR;
    }
}

} // end namespace tigl
