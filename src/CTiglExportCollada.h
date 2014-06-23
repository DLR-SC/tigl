/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-03-19 Volker Poddey <Volker.Poddey@dlr.de>
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

#ifndef CTIGLEXPORTCOLLADA_H
#define CTIGLEXPORTCOLLADA_H

#include "tigl.h"
#include "tigl_internal.h"

#include <string>

class TopoDS_Shape;

namespace tigl 
{

class CTiglExportCollada
{
public:
    TIGL_EXPORT CTiglExportCollada(class CCPACSConfiguration& config);
    
    /// some convenience helper functions
    TIGL_EXPORT TiglReturnCode exportFuselage(const std::string& fuselageUID, const std::string& filename, const double deflection = 0.1);
    TIGL_EXPORT TiglReturnCode exportWing    (const std::string& wingUID    , const std::string& filename, const double deflection = 0.1);
    
    TIGL_EXPORT TiglReturnCode exportShape(TopoDS_Shape& shape, const std::string& shapeID, const std::string& filename, const double deflection = 0.1);

    /// Exports a polygon object to a collada file, the true export code
    TIGL_EXPORT static TiglReturnCode writeToDisc(class CTiglPolyData &polyData, const char * id, const char * filename);
    
private:
    class CCPACSConfiguration& myconfig;
};


} // end namespace tigl

#endif // CTIGLEXPORTCOLLADA_H
