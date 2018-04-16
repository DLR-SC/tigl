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

#ifndef CTIGLEXPORTVTK_H
#define CTIGLEXPORTVTK_H

#include "tigl_internal.h"
#include "CTiglCADExporter.h"
#include "CTiglTriangularizer.h"

#include <string>
#include <map>

namespace tigl 
{

class CTiglPolyData;
class CTiglPolyObject;

class VtkOptions : public ExporterOptions
{
public:
    VtkOptions()
    {
        AddOption("WriteNormals", true);
        AddOption("MultiplePieces", false);
        AddOption("WriteMetaData", true);

        Set("ApplySymmetries", true);
        Set("IncludeFarfield", false);
    }
};

class CTiglExportVtk : public CTiglCADExporter
{
public:
    // Constructor
    TIGL_EXPORT CTiglExportVtk(const ExporterOptions& opt = DefaultExporterOption());

    TIGL_EXPORT ExporterOptions GetDefaultOptions() const OVERRIDE;
    TIGL_EXPORT ShapeExportOptions GetDefaultShapeOptions() const OVERRIDE;


    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglExportVtk();

    /// Exports a polygonal data representation directly
    TIGL_EXPORT static void WritePolys(const CTiglPolyData& polys, const char * filename);

private:
    bool WriteImpl(const std::string& filename) const OVERRIDE;

    std::string SupportedFileTypeImpl() const OVERRIDE
    {
        return "vtp;vtk";
    }

    static void writeVTKPiece(const CTiglPolyObject& co, TixiDocumentHandle& handle, unsigned int iObject); 
    static void writeVTKHeader(TixiDocumentHandle& handle);
};

} // end namespace tigl

#endif // CTIGLEXPORTVTK_H
