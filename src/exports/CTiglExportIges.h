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

#ifndef CTIGLEXPORTIGES_H
#define CTIGLEXPORTIGES_H

#include "tigl_config.h"
#include "tigl_internal.h"
#include "PNamedShape.h"
#include "ListPNamedShape.h"
#include "CCPACSImportExport.h"

class CCPACSConfiguration;
class IGESControl_Writer;

namespace tigl 
{

class CTiglExportIges
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportIges(CCPACSConfiguration& config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglExportIges(void);

    // Exports the whole configuration as IGES file
    TIGL_EXPORT void ExportIGES(const std::string& filename) const;

    // Exports the whole configuration, boolean fused, as IGES file
    TIGL_EXPORT void ExportFusedIGES(const std::string& filename);

    // Save a sequence of shapes in IGES Format
    TIGL_EXPORT void ExportShapes(const ListPNamedShape& shapes, const std::string& filename) const;

    // Sets the type of storing shapes to iges
    TIGL_EXPORT void SetOCAFStoreType(ShapeStoreType type);

protected:

private:
    // Assignment operator
    void operator=(const CTiglExportIges& ) { /* Do nothing */ }
    void AddToIges(PNamedShape shape, IGESControl_Writer& writer) const;

    CCPACSConfiguration&          myConfig;       /**< TIGL configuration object */
    ShapeStoreType                  myStoreType;    /**< Type specifying how to translate shapes into an OCAF document */
    void SetTranslationParameters() const;
};

} // end namespace tigl

#endif // CTIGLEXPORTIGES_H
