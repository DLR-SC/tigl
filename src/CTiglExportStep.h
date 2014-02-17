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
#include "CTiglUIDManager.h"
#include "CCPACSHeader.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"

#ifdef TIGL_USE_XCAF
#include "STEPCAFControl_Writer.hxx"
#endif
#include "TopTools_HSequenceOfShape.hxx"

class CCPACSConfiguration;
class STEPControl_Writer;

enum CAFStoreType;

namespace tigl 
{

enum TiglStepExportMode 
{
    AS_FACES,
    AS_SOLIDS
};

class CTiglExportStep
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportStep(CCPACSConfiguration& config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglExportStep(void);

    // Exports the whole configuration as IGES file
    TIGL_EXPORT void ExportStep(const std::string& filename) const;

    // Exports the whole configuration, boolean fused, as IGES file
    TIGL_EXPORT void ExportFusedStep(const std::string& filename);

    // Save a sequence of shapes in IGES Format
    TIGL_EXPORT void ExportShapes(const Handle(TopTools_HSequenceOfShape)& aHSequenceOfShape, const std::string& filename);

    // Sets the type of storing shapes to iges
    TIGL_EXPORT void SetOCAFStoreType(CAFStoreType type);

#ifdef TIGL_USE_XCAF // this feature requires xcaf
    // Saves as IGES, with cpacs metadata information in it
    TIGL_EXPORT void ExportStepWithCPACSMetadata(const std::string& filename);
#endif

protected:

private:
    // Assignment operator
    void operator=(const CTiglExportStep& ) { /* Do nothing */ }

    CCPACSConfiguration&          myConfig;       /**< TIGL configuration object */
    CAFStoreType                  myStoreType;    /**< Type specifying how to translate shapes into an OCAF document */
    void AddFacesOfShape(const TopoDS_Shape &shape, STEPControl_Writer &writer) const;
};

} // end namespace tigl

#endif // CTIGLEXPORTSTEP_H
