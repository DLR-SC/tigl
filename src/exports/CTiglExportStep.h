/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-28 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  STEP-Export routines for CPACS configurations.
*/

#ifndef CTIGLEXPORTSTEP_H
#define CTIGLEXPORTSTEP_H

#include "tigl_internal.h"
#include "CCPACSImportExport.h"
#include "ListPNamedShape.h"

class STEPControl_Writer;

namespace tigl 
{

class CCPACSConfiguration;

class CTiglExportStep
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportStep(CCPACSConfiguration& _config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglExportStep(void);

    // Exports the whole configuration as IGES file
    TIGL_EXPORT void ExportStep(const std::string& filename) const;

    // Exports the whole configuration, boolean fused, as IGES file
    TIGL_EXPORT void ExportFusedStep(const std::string& filename);

    // Save a sequence of shapes in IGES Format
    TIGL_EXPORT void ExportShapes(const ListPNamedShape& shapes, const std::string& filename) const;

    // Sets the type of storing shapes to iges
    TIGL_EXPORT void SetGroupMode(ShapeGroupMode mode);

protected:

private:
    // Assignment operator
    void operator=(const CTiglExportStep& ) { /* Do nothing */ }

    CCPACSConfiguration&          _config;       /**< TIGL configuration object */
    ShapeGroupMode                _groupMode;    /**< Type specifying how to group faces in the step file */
    void AddToStep(PNamedShape shape, STEPControl_Writer &writer) const;
};

} // end namespace tigl

#endif // CTIGLEXPORTSTEP_H
