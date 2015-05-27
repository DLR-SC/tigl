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

namespace tigl 
{

class CCPACSConfiguration;

class CTiglExportBrep
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportBrep();

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglExportBrep(void);

    // Adds the whole configuration
    TIGL_EXPORT void AddConfiguration(CCPACSConfiguration& config);

    // Adds the whole configuration, boolean fused
    TIGL_EXPORT void AddFusedConfiguration(CCPACSConfiguration& config);

    // Adds a shape
    TIGL_EXPORT void AddShape(PNamedShape shape);
    
    // Writes the shapes to BREP. In multiple shapes were added
    // a compound is created.
    TIGL_EXPORT bool Write(const std::string& filename) const;

private:
    ListPNamedShape _shapes;
};

} // namespace tigl

#endif // CTIGLEXPORTBREP_H
