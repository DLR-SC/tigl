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

#ifndef CTIGLEXPORTVTK_H
#define CTIGLEXPORTVTK_H

#include "tigl_internal.h"

#include <string>

namespace tigl 
{

enum VTK_EXPORT_MODE
    {
    TIGL_VTK_SIMPLE = 0,
    TIGL_VTK_COMPLEX  = 1
    };

class CTiglExportVtk
{
public:
    // Constructor
    TIGL_EXPORT CTiglExportVtk(class CCPACSConfiguration & config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglExportVtk();

    // Exports a by index selected wing, boolean fused and meshed, as VTK file
    TIGL_EXPORT void ExportMeshedWingVTKByIndex(const int wingIndex, const std::string& filename, const double deflection = 0.1);

    // Exports a by UID selected wing, boolean fused and meshed, as VTK file
    TIGL_EXPORT void ExportMeshedWingVTKByUID(const std::string& wingUID, const std::string& filename, const double deflection = 0.1);

    // Exports a by index selected fuselage, boolean fused and meshed, as VTK file
    TIGL_EXPORT void ExportMeshedFuselageVTKByIndex(const int fuselageIndex, const std::string& filename, const double deflection = 0.1);

    // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file
    TIGL_EXPORT void ExportMeshedFuselageVTKByUID(const std::string& fuselageUID, const std::string& filename, const double deflection = 0.1);

    // Exports a whole geometry, boolean fused and meshed, as VTK file
    TIGL_EXPORT void ExportMeshedGeometryVTK(const std::string& filename, const double deflection = 0.1);


    // Simple exports without cpacs information
    // Exports a by UID selected wing, meshed, as VTK file
    // No additional information are computed.
    TIGL_EXPORT void ExportMeshedWingVTKSimpleByUID(const std::string& wingUID, const std::string& filename, const double deflection = 0.1);
    
    TIGL_EXPORT void ExportMeshedWingVTKSimpleByIndex(const int wingIndex, const std::string& filename, const double deflection = 0.1);

    // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file.
    // No additional information are computed.
    TIGL_EXPORT void ExportMeshedFuselageVTKSimpleByUID(const std::string& fuselageUID, const std::string& filename, const double deflection = 0.1);
    
    TIGL_EXPORT void ExportMeshedFuselageVTKSimpleByIndex(const int fuselageIndex, const std::string& filename, const double deflection = 0.1);

    // Exports a whole geometry, boolean fused and meshed, as VTK file
    // No additional information are computed.
    TIGL_EXPORT void ExportMeshedGeometryVTKSimple(const std::string& filename, const double deflection = 0.1);

    TIGL_EXPORT void ExportMeshedGeometryVTKNoFuse(const std::string& filename, const double deflection = 0.1);

    TIGL_EXPORT static void SetOptions(const std::string& key, const std::string& value);


    // Options
    TIGL_EXPORT static bool normalsEnabled;

private:
    class CCPACSConfiguration & myConfig;       /**< TIGL configuration object */

};

} // end namespace tigl

#endif // CTIGLEXPORTVTK_H
