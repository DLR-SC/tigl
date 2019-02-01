/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#include <generated/CPACSEngineNacelle.h>
#include "tigl_internal.h"

#pragma once

namespace tigl
{
    class CTiglEngineNacelleBuilder
    {
    public:
        explicit CTiglEngineNacelleBuilder(const CCPACSEngineNacelle& nacelle);
        explicit CTiglEngineNacelleBuilder(const CCPACSEngineNacelle& nacelle, const CTiglTransformation& transformation);

        TIGL_EXPORT operator PNamedShape();

        TIGL_EXPORT PNamedShape BuildShape();

    private:
        const CCPACSEngineNacelle& m_nacelle;
        CTiglTransformation const* m_transformation = nullptr;

    };

} //namespace tigl
