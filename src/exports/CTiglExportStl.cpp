/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
#include "CTiglExporterFactory.h"
#include "CTiglTypeRegistry.h"

#include "TopoDS_Shape.hxx"
#include "Standard_CString.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRep_Builder.hxx"
#include "BRepTools.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "StlAPI_Writer.hxx"
#include "Interface_Static.hxx"
#include "StlAPI.hxx"
#include "CTiglFusePlane.h"

#include <cassert>

namespace tigl 
{

AUTORUN(CTiglExportStl)
{
    static CCADExporterBuilder<CTiglExportStl> stlExporterBuilder;
    CTiglExporterFactory::Instance().RegisterExporter(&stlExporterBuilder, StlOptions());
    return true;
}

// Constructor
CTiglExportStl::CTiglExportStl(const ExporterOptions& opt)
    : CTiglCADExporter(opt)
{
}

ExporterOptions CTiglExportStl::GetDefaultOptions() const
{
    return StlOptions();
}

ShapeExportOptions CTiglExportStl::GetDefaultShapeOptions() const
{
    return TriangulatedExportOptions(0.001);
}

bool CTiglExportStl::WriteImpl(const std::string& filename) const
{
    for (size_t ishape = 0; ishape < NShapes(); ++ishape) {
        PNamedShape shape = GetShape(ishape);
        if (shape) {
            BRepMesh_IncrementalMesh(shape->Shape(), GetOptions(ishape).Get<double>("Deflection"));
        }
    }

    if (NShapes() > 1) {
        TopoDS_Compound c;
        BRep_Builder b;
        b.MakeCompound(c);
        
        for (size_t ishape = 0; ishape < NShapes(); ++ishape) {
            PNamedShape shape = GetShape(ishape);
            if (shape) {
                b.Add(c, shape->Shape());
            }
        }
        
        // write the file
        StlAPI_Writer StlWriter;
        StlWriter.Write(c, const_cast<char*>(filename.c_str()));
        
        return true;
    }
    else if (NShapes() == 1) {
        PNamedShape shape = GetShape(0);
        StlAPI_Writer StlWriter;
        StlWriter.Write(shape->Shape(), const_cast<char*>(filename.c_str()));
        
        return true;
    }
    else {
        return false;
    }
}

} // end namespace tigl
