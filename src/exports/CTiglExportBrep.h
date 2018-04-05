/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-04-22 Martin Siggel <Martin.Siggel@dlr.de>
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
* @brief  BRep-Export routines for CPACS configurations.
*/

#ifndef CTIGLEXPORTBREP_H
#define CTIGLEXPORTBREP_H

#include <string>

#include "tigl_internal.h"
#include "ListPNamedShape.h"
#include "CTiglCADExporter.h"

namespace tigl 
{

class CCPACSConfiguration;

class BRepOptions : public ExporterOptions
{
public:
    BRepOptions()
    {
        Set("ApplySymmetries", false);
        Set("IncludeFarfield", false);
        Set("ShapeGroupMode", WHOLE_SHAPE);
    }
};

class CTiglExportBrep : public CTiglCADExporter
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportBrep(const ExporterOptions& opt = DefaultExporterOption())
        : CTiglCADExporter(opt)
    {
    }

    TIGL_EXPORT ExporterOptions GetDefaultOptions() const OVERRIDE;
    TIGL_EXPORT ShapeExportOptions GetDefaultShapeOptions() const OVERRIDE;

private:
    // Writes the shapes to BREP. In multiple shapes were added
    // a compound is created.
    TIGL_EXPORT bool WriteImpl(const std::string& filename) const OVERRIDE;

    std::string SupportedFileTypeImpl() const OVERRIDE
    {
        return "brep";
    }
};

} // namespace tigl

#endif // CTIGLEXPORTBREP_H
