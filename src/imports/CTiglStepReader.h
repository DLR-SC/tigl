/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-05 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLSTEPREADER_H
#define CTIGLSTEPREADER_H

#include "tigl_internal.h"
#include "tigl_config.h"
#include "ITiglCADImporter.h"

#include <string>

namespace tigl
{

class CTiglStepReader : public ITiglCADImporter
{
public:
    TIGL_EXPORT CTiglStepReader();
    
    /// reads in a step file
    TIGL_EXPORT ListPNamedShape Read(const std::string stepFileName) OVERRIDE;

    TIGL_EXPORT std::string SupportedFileType() const OVERRIDE;
    
    TIGL_EXPORT ~CTiglStepReader();
};

}

#endif // CTIGLSTEPREADER_H
