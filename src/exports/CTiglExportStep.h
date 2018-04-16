/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-28 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  STEP-Export routines for CPACS configurations.
*/

#ifndef CTIGLEXPORTSTEP_H
#define CTIGLEXPORTSTEP_H

#include "tigl_internal.h"
#include "CCPACSImportExport.h"
#include "ListPNamedShape.h"
#include "CTiglCADExporter.h"

class STEPControl_Writer;

namespace tigl 
{

class CCPACSConfiguration;

class StepOptions : public ExporterOptions
{
public:
    StepOptions()
    {
        Set("ApplySymmetries", false);
        Set("IncludeFarfield", true);
        Set("ShapeGroupMode", NAMED_COMPOUNDS);
    }
};

class StepShapeOptions : public ShapeExportOptions
{
public:
    StepShapeOptions(){}
};

class CTiglExportStep : public CTiglCADExporter
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportStep(const ExporterOptions& opt = DefaultExporterOption());

    TIGL_EXPORT ExporterOptions GetDefaultOptions() const OVERRIDE;
    TIGL_EXPORT ShapeExportOptions GetDefaultShapeOptions() const OVERRIDE;

private:
    // Writes the step file
    TIGL_EXPORT bool WriteImpl(const std::string& filename) const OVERRIDE;

    std::string SupportedFileTypeImpl() const OVERRIDE
    {
        return "step;stp";
    }

    // Assignment operator
    void operator=(const CTiglExportStep& ) { /* Do nothing */ }

    void AddToStep(PNamedShape shape, STEPControl_Writer &writer) const;
};

} // end namespace tigl

#endif // CTIGLEXPORTSTEP_H
