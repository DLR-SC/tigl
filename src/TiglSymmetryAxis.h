/*
* Copyright (c) 2018 RISC Software GmbH
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

#pragma once

#include "tigl.h"

#include <string>
#include "to_string.h"
#include "CTiglError.h"

namespace tigl
{
using ::TiglSymmetryAxis;

inline std::string TiglSymmetryAxisToString(const TiglSymmetryAxis& value)
{
    switch (value) {
    case TIGL_NO_SYMMETRY:
        return "";
    case TIGL_X_Y_PLANE:
        return "x-y-plane";
    case TIGL_X_Z_PLANE:
        return "x-z-plane";
    case TIGL_Y_Z_PLANE:
        return "y-z-plane";
    default:
        throw CTiglError("Invalid enum value \"" + std_to_string(static_cast<int>(value)) +
                         "\" for enum type TiglSymmetryAxis");
    }
}

inline TiglSymmetryAxis stringToTiglSymmetryAxis(const std::string& value)
{
    if (value == "x-y-plane") {
        return TIGL_X_Y_PLANE;
    }
    if (value == "x-z-plane") {
        return TIGL_X_Z_PLANE;
    }
    if (value == "y-z-plane") {
        return TIGL_Y_Z_PLANE;
    }
    return TIGL_NO_SYMMETRY;
} // namespace tigl
}

