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

#ifndef CTIGLEXPORTSTL_H
#define CTIGLEXPORTSTL_H

#include "tigl_internal.h"
#include "CTiglUIDManager.h"
#include "CCPACSHeader.h"
#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"


namespace tigl 
{

class CTiglExportStl
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportStl(CCPACSConfiguration& config);

    // Empty destructor
    TIGL_EXPORT ~CTiglExportStl() { /* empty */}

    // Exports a selected wing, boolean fused and meshed, as STL file
    TIGL_EXPORT void ExportMeshedWingSTL(int wingIndex, const std::string& filename, double deflection = 0.1);

    // Exports a selected fuselage, boolean fused and meshed, as STL file
    TIGL_EXPORT void ExportMeshedFuselageSTL(int fuselageIndex, const std::string& filename, double deflection = 0.1);

    // Exports a whole geometry, boolean fused and meshed, as STL file
    TIGL_EXPORT void ExportMeshedGeometrySTL(const std::string& filename, double deflection = 0.1);

protected:

private:
    // Assignment operator
    void operator=(const CTiglExportStl& ) { /* Do nothing */ }

    CCPACSConfiguration & myConfig;       /**< TIGL configuration object */
};

} // end namespace tigl

#endif // CTIGLEXPORTSTL_H
