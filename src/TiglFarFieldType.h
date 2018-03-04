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

#include <string>
#include <cctype>
#include "to_string.h"
#include "tigl_internal.h"
#include "CTiglError.h"

namespace tigl
{
enum TiglFarFieldType
{
    NONE, // TODO(bgruber) not part of CPACS
    HALF_SPHERE,
    FULL_SPHERE,
    HALF_CUBE,
    FULL_CUBE
};

inline std::string TiglFarFieldTypeToString(const TiglFarFieldType& value)
{
    switch (value) {
    case HALF_SPHERE:
        return "halfSphere";
    case FULL_SPHERE:
        return "fullSphere";
    case HALF_CUBE:
        return "halfCube";
    case FULL_CUBE:
        return "fullCube";
    case NONE:
        return "none";
    default:
        throw CTiglError("Invalid enum value \"" + std_to_string(static_cast<int>(value)) +
                         "\" for enum type TiglFarFieldType");
    }
}

inline TiglFarFieldType stringToTiglFarFieldType(const std::string& value)
{
    struct ToLower {
        std::string operator()(std::string str)
        {
            for (std::size_t i = 0; i < str.length(); i++) {
                str[i] = std::tolower(str[i]);
            }
            return str;
        }
    } toLower;
    if (toLower(value) == "halfsphere") {
        return HALF_SPHERE;
    }
    if (toLower(value) == "fullsphere") {
        return FULL_SPHERE;
    }
    if (toLower(value) == "halfcube") {
        return HALF_CUBE;
    }
    if (toLower(value) == "fullcube") {
        return FULL_CUBE;
    }
    if (toLower(value) == "none") {
        return NONE;
    }
    throw CTiglError("Invalid string value \"" + value + "\" for enum type TiglFarFieldType");
}
}
