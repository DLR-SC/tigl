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

#ifndef CTIGLEXPORTSTL_H
#define CTIGLEXPORTSTL_H

#include "tigl_internal.h"
#include "CTiglCADExporter.h"




namespace tigl 
{

class StlOptions : public ExporterOptions
{
public:
    StlOptions()
    {
        Set("ApplySymmetries", true);
        Set("IncludeFarfield", false);
    }
};

class CTiglExportStl : public CTiglCADExporter
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportStl(const ExporterOptions& opt = DefaultExporterOption());

    TIGL_EXPORT ExporterOptions GetDefaultOptions() const OVERRIDE;
    TIGL_EXPORT ShapeExportOptions GetDefaultShapeOptions() const OVERRIDE;

private:

    bool WriteImpl(const std::string& filename) const OVERRIDE;

    std::string SupportedFileTypeImpl() const OVERRIDE
    {
        return "stl";
    }

    // Assignment operator
    void operator=(const CTiglExportStl& ) { /* Do nothing */ }
};

} // end namespace tigl

#endif // CTIGLEXPORTSTL_H
