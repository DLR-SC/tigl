/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-15 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglTypeRegistry.h"

#define REGISTER_TYPE(type) \
    AUTORUN(type); \
    static const bool registered_ ## type = type ## _RegisterType();

namespace tigl
{

// register all dynamic types to prevent linker optimization
REGISTER_TYPE(CTiglStepReader)
REGISTER_TYPE(CTiglExportStep)
REGISTER_TYPE(CTiglExportIges)
REGISTER_TYPE(CTiglExportVtk)
REGISTER_TYPE(CTiglExportCollada)
REGISTER_TYPE(CTiglExportStl)
REGISTER_TYPE(CTiglExportBrep)


void CTiglTypeRegistry::Init()
{
// dummy implementation
}
 
} // namespace tigl
